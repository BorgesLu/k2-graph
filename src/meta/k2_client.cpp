#include "k2_client.h"

#include "k2_includes.h"
#include "k2_queue_defs.h"
#include "Log.h"

//using namespace k2;
namespace k2pg {
namespace gate {
PGK2Client::PGK2Client() {
    //K2LOG_I(log::k23si, "Ctor");
    _client = new k2::K23SIClient(k2::K23SIClientConfig());
    _txns = new std::unordered_map<k2::dto::K23SI_MTR, k2::K2TxnHandle>();
}

PGK2Client::~PGK2Client() {
    delete _client;
    delete _txns;
}
seastar::future<> PGK2Client::gracefulStop() {
   // K2LOG_I(log::k23si, "Stop");
    _stop = true;

    return std::move(_poller)
        .then([this] { 
            return _client->gracefulStop(); 
        })
        .then([this] {
            // drain all queue items and fail them due to shutdown
            return _pollForWork();
        });
}

seastar::future<> PGK2Client::start() {
   // K2LOG_I(log::k23si, "Start");
    // start polling the request queues only on core 0

    std::cout << "\n\n\n  seastar app start \n\n";
    if (seastar::this_shard_id() == 0) {
      //  K2LOG_I(log::k23si, "Poller starting");
        _poller = _poller.then([this] {
            return seastar::do_until(
                [this] {
                    return _stop;
                },
                [this] {
                    // std::cout << "\n\n\n\nabc\n\n\n\n";
                    return _pollForWork();
                });
        });
    }
    return _client->start();
}

template <typename Q, typename Func>
seastar::future<> pollQ(Q& q, Func&& visitor) {
    // lock the mutex before manipulating the queue
    // std::unique_lock lock(requestQMutex);
    std::vector<seastar::future<>> futs;
    futs.reserve(q.size());
    while (!q.empty()) {
       // K2LOG_I(log::k23si, "Found Req");
        futs.push_back(
            seastar::do_with(std::move(q.front()), std::forward<Func>(visitor),
            [](auto &req, auto &visitor) {
                try {
                    // std::cout << "\n\n\n\nline104\n\n\n\n";
                    return visitor(req)
                        .handle_exception([&req](auto exc) {
                           // K2LOG_W_EXC(log::k23si, exc, "caught exception");
                            req.prom->set_exception(exc);
                            //返回异常
                        });
                }
                catch (const std::exception &exc) {
                    req.prom->set_exception(std::current_exception());
                    //返回异常
                    return seastar::make_ready_future();
                }
                catch (...) {
                    req.prom->set_exception(std::current_exception());
                    //返回异常
                    return seastar::make_ready_future();
                }
            }));
        q.pop();
    }
    return seastar::when_all_succeed(futs.begin(), futs.end());
}



seastar::future<> PGK2Client::_pollBeginQ() {
    return pollQ(k2graph::BeginTxnQ, [this](auto& req) {
        if (_stop) {
            return seastar::make_exception_future(std::runtime_error("seastar app has been shutdown"));
        }
        return _client->beginTxn(req.opts)
            .then([this, &req](auto&& txn) {
                auto mtr = txn.mtr();
                (*_txns)[txn.mtr()] = std::move(txn);
                // req.prom->set_value(mtr);  // send a copy to the promise
                req.prom->set_value(k2pg::gate::K23SITxn(mtr,req.startTime));
            });
    });
}

seastar::future<> PGK2Client::_pollEndQ() {
    return pollQ(k2graph::EndTxnQ, [this](auto& req) {
        if (_stop) {
            return seastar::make_exception_future(std::runtime_error("seastar app has been shutdown"));
        }
        auto fiter = _txns->find(req.mtr);
        if (fiter == _txns->end()) {
            // PG sends Abort after a failed Commit call (in this case we don't fail the abort)
            req.prom->set_value(req.shouldCommit ?
            k2::EndResult(k2::dto::K23SIStatus::OperationNotAllowed("invalid txn id")) :
            k2::EndResult(k2::dto::K23SIStatus::OK("")));
            return seastar::make_ready_future();
        }
        return fiter->second.end(req.shouldCommit)
            .then([this, &req](auto&& endResult) {
                _txns->erase(req.mtr);
                req.prom->set_value(std::move(endResult));
            });
    });
}

seastar::future<> PGK2Client::_pollSchemaGetQ() {
    return pollQ(k2graph::SchemaGetQ, [this](auto &req) {
        if (_stop) {
            return seastar::make_exception_future(std::runtime_error("seastar app has been shutdown"));
        }
        return _client->getSchema(req.collectionName, req.schemaName, req.schemaVersion)
            .then([this, &req](auto&& result){
                req.prom -> set_value(std::move(result));
            });
    });
}

seastar::future<> PGK2Client::_pollSchemaCreateQ() {
    return pollQ(k2graph::SchemaCreateQ, [this](auto &req) {
        std::cout<<"\n\n\n _pollSchemaCreateQ";
        // std::cout << "\n\n\n\n"<<req.req.clusterEndpoints[0]<<"\n\n\n\n"<< req.req.rangeEnds<<"abc\n\n\n";
        if (_stop) {
            return seastar::make_exception_future(std::runtime_error("seastar app has been shutdown"));
        }
        // std::cout<<"\n\n_client->createSchema \n";
        // std::cout<<req.req.collectionName<<std::endl;
        // std::cout<<"line149   "<<req.req.schema.fields[3].type<<"\n\n";s
        return _client->createSchema(std::move(req.req.collectionName), std::move(req.req.schema))
            .then([this, &req](auto &&result) {
                // std::cout<<"\n\nafter _clientcreateSchema\n\n";
                //K2LOG_D(log::k23si, "Schema create received {}", result);
                req.prom->set_value(std::move(result));
            });
    });
}



seastar::future<> PGK2Client::_pollCreateCollectionQ() {
    return pollQ(k2graph::collectionCreateQ, [this](auto &req) {
        // std::cout << "\n\n\n\n"<<req.req.clusterEndpoints[0]<<"\n\n\n\n"<< req.req.rangeEnds<<"abc\n\n\n";
        if (_stop) {
            return seastar::make_exception_future(std::runtime_error("seastar app has been shutdown"));
        }
        return _client->makeCollection(std::move(req.req.metadata), {req.req.clusterEndpoints[0]},
                                        std::move(req.req.rangeEnds))
            .then([this, &req](auto &&result) {
                //K2LOG_D(log::k23si, "Collection create received {}", result);
                // std::cout << "\n\n\n\nline164\n\n\n\n";
                std::cout << "\n\n cliet_makecollection \n\n";
                req.prom->set_value(std::move(result));
            });
    });
}



seastar::future<> PGK2Client::_pollWriteQ() {
    return pollQ(k2graph::WriteRequestQ, [this](auto& req) mutable {
        if (_stop) {
            return seastar::make_exception_future(std::runtime_error("seastar app has been shutdown"));
        }
        auto fiter = _txns->find(req.mtr);
        if (fiter == _txns->end()) {
            req.prom->set_value(k2::WriteResult(k2::dto::K23SIStatus::OperationNotAllowed("invalid txn id"), k2::dto::K23SIWriteResponse{}));
            return seastar::make_ready_future();
        }
        k2::dto::SKVRecord copy = req.record.deepCopy();
        return fiter->second.write(copy, false, k2::dto::ExistencePrecondition::None)
            .then([this, &req](auto&& writeResult) {
                req.prom->set_value(std::move(writeResult));
            });
    });
}



seastar::future<> PGK2Client::_pollForWork() {
    return seastar::when_all_succeed(
        _pollCreateCollectionQ(),
         _pollSchemaCreateQ(),
        _pollSchemaGetQ(), 
        _pollBeginQ(), 
        _pollEndQ(),
        _pollWriteQ())
        .discard_result();
}


}  // namespace gate
}  // namespace k2pg