/* Copyright (c) 2018 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include <thrift/lib/cpp2/server/ThriftServer.h>
#include "base/Base.h"
#include "common/base/SignalHandler.h"
#include "hdfs/HdfsCommandHelper.h"
#include "hdfs/HdfsHelper.h"
#include "kvstore/NebulaStore.h"
#include "kvstore/PartManager.h"
#include "meta/ActiveHostsMan.h"
#include "meta/ClusterIdMan.h"
#include "meta/MetaHttpDownloadHandler.h"
#include "meta/MetaHttpIngestHandler.h"
#include "meta/MetaHttpReplaceHostHandler.h"
#include "meta/MetaServiceHandler.h"
#include "meta/RootUserMan.h"
#include "meta/processors/jobMan/JobManager.h"
#include "network/NetworkUtils.h"
#include "process/ProcessUtils.h"
#include "thread/GenericThreadPool.h"
#include "webservice/Router.h"
#include "webservice/WebService.h"


#include <seastar/core/future.hh>
#include "../meta/k2_client.h"
#include "../meta/k2_includes.h"

#include <map>

using nebula::Status;
using nebula::ProcessUtils;
using nebula::network::NetworkUtils;
using nebula::web::PathParams;
using nebula::operator<<;

DEFINE_int32(port,9777, "Meta daemon listening port");
DEFINE_bool(reuse_port, true, "Whether to turn on the SO_REUSEPORT option");
DEFINE_string(data_path, "data/meta", "Root data path");
DEFINE_string(meta_server_addrs,
              "",
              "It is a list of IPs split by comma, used in cluster deployment"
              "the ips number is equal to the replica number."
              "If empty, it means it's a single node");
DEFINE_string(local_ip, "127.0.0.1", "Local ip specified for NetworkUtils::getLocalIP");
DEFINE_int32(num_io_threads, 1, "Number of IO threads");
DEFINE_int32(meta_http_thread_num, 1, "Number of meta daemon's http thread");
DEFINE_int32(num_worker_threads, 1, "Number of workers");

DEFINE_string(pid_file, "pids/nebula-metad.pid", "File to hold the process id");
DEFINE_bool(daemonize, true, "Whether run as a daemon process");
DECLARE_bool(check_leader);


static std::unique_ptr<apache::thrift::ThriftServer> gServer;
static void signalHandler(int sig);
static Status setupSignalHandler();

namespace nebula {
namespace meta {
const std::string kClusterIdKey = "__meta_cluster_id_key__";  // NOLINT
}  // namespace meta
}  // namespace nebula


nebula::ClusterID gClusterId = 0;

std::unique_ptr<nebula::kvstore::KVStore> initKV(std::vector<nebula::HostAddr> peers,
                                                 nebula::HostAddr localhost) {
    auto partMan
        = std::make_unique<nebula::kvstore::MemPartManager>();
    // The meta server has only one space (0), one part (0)
    partMan->addPart(nebula::meta::kDefaultSpaceId,
                     nebula::meta::kDefaultPartId,
                     std::move(peers));
    // folly IOThreadPoolExecutor
    auto ioPool = std::make_shared<folly::IOThreadPoolExecutor>(FLAGS_num_io_threads);
    std::shared_ptr<apache::thrift::concurrency::ThreadManager> threadManager(
        apache::thrift::concurrency::PriorityThreadManager::newPriorityThreadManager(
                                 FLAGS_num_worker_threads, true /*stats*/));
    threadManager->setNamePrefix("executor");
    threadManager->start();
    // On metad, we are allowed to read on follower
    FLAGS_check_leader = false;
    nebula::kvstore::KVOptions options;
    options.dataPaths_ = {FLAGS_data_path};
    options.partMan_ = std::move(partMan);
    auto kvstore = std::make_unique<nebula::kvstore::NebulaStore>(
                                                        std::move(options),
                                                        ioPool,
                                                        localhost,
                                                        threadManager);
    if (!(kvstore->init())) {
        LOG(ERROR) << "Nebula store init failed";
        return nullptr;
    }

    LOG(INFO) << "Waiting for the leader elected...";
    nebula::HostAddr leader;
    while (true) {
        auto ret = kvstore->partLeader(nebula::meta::kDefaultSpaceId,
                                       nebula::meta::kDefaultPartId);
        if (!nebula::ok(ret)) {
            LOG(ERROR) << "Nebula store init failed";
            return nullptr;
        }
        leader = nebula::value(ret);
        if (leader != nebula::HostAddr(0, 0)) {
            break;
        }
        LOG(INFO) << "Leader has not been elected, sleep 1s";
        sleep(1);
    }

    gClusterId = nebula::meta::ClusterIdMan::getClusterIdFromKV(kvstore.get(),
                                                                nebula::meta::kClusterIdKey);
    if (gClusterId == 0) {
        if (leader == localhost) {
            LOG(INFO) << "I am leader, create cluster Id";
            gClusterId = nebula::meta::ClusterIdMan::create(FLAGS_meta_server_addrs);
            if (!nebula::meta::ClusterIdMan::persistInKV(kvstore.get(),
                                                         nebula::meta::kClusterIdKey,
                                                         gClusterId)) {
                LOG(ERROR) << "Persist cluster failed!";
                return nullptr;
            }
        } else {
            LOG(INFO) << "I am follower, wait for the leader's clusterId";
            while (gClusterId == 0) {
                LOG(INFO) << "Waiting for the leader's clusterId";
                sleep(1);
                gClusterId = nebula::meta::ClusterIdMan::getClusterIdFromKV(
                                                kvstore.get(),
                                                nebula::meta::kClusterIdKey);
            }
        }
    }
    LOG(INFO) << "Nebula store init succeeded, clusterId " << gClusterId;
    return kvstore;
}

Status initWebService(nebula::WebService* svc,
                      nebula::kvstore::KVStore* kvstore,
                      nebula::hdfs::HdfsCommandHelper* helper,
                      nebula::thread::GenericThreadPool* pool) {
    LOG(INFO) << "Starting Meta HTTP Service";
    auto& router = svc->router();
    router.get("/download-dispatch").handler([kvstore, helper, pool](PathParams&&) {
        auto handler = new nebula::meta::MetaHttpDownloadHandler();
        handler->init(kvstore, helper, pool);
        return handler;
    });
    router.get("/ingest-dispatch").handler([kvstore, pool](PathParams&&) {
        auto handler = new nebula::meta::MetaHttpIngestHandler();
        handler->init(kvstore, pool);
        return handler;
    });
    router.get("/replace").handler([kvstore](PathParams &&) {
        auto handler = new nebula::meta::MetaHttpReplaceHostHandler();
        handler->init(kvstore);
        return handler;
    });
    return svc->start();
}




 void addArg(int &argc,char** &argv, const std::string& sarg)
     {
        argv[argc] = (char*)malloc(sizeof(char)*(sarg.size() + 1));  
        strncpy(argv[argc], sarg.c_str(), sarg.size());
        argv[argc][sarg.size()] = '\0';
        argc++;
    }
       
using namespace k2;
//独立的线程去运行seastar的app
static void seastarStart() {
    LOG(INFO) << "seastar thread 开始  ";
    k2::App app("main");
    app.addOptions()("tcp_remotes",
                     bpo::value<std::vector<k2::String>>()->multitoken()->default_value(
                         std::vector<k2::String>()),
                     "A list(space-delimited) of endpoints to assign in the test collection")(
        "tso_endpoint",
        bpo::value<k2::String>(),
        "URL of Timestamp Oracle (TSO), e.g. 'tcp+k2rpc://192.168.1.2:12345'")(
        "cpo",
        bpo::value<k2::String>(),
        "URL of Control Plane Oracle (CPO), e.g. 'tcp+k2rpc://192.168.1.2:12345'");
    app.addApplet<k2::TSO_ClientLib>();
    app.addApplet<k2pg::gate::PGK2Client>();   //和k2服务器交互的客户端
   
//直接传参数而不是通过脚本，因为folly：：init() 不识别这些K2需要的参数,传人的参数如下：
 /*  {"/home/kvgroup/lmx/nebula-1.0.1/build/bin/nebula-metad",
   "--cpo","tcp+k2rpc://0.0.0.0:9000",
   "--tcp_remotes","tcp+k2rpc://0.0.0.0:10000","tcp+k2rpc://0.0.0.0:10001","tcp+k2rpc://0.0.0.0:10002",
   "--enable_tx_checksum","ture",
   "--reactor-backend","epoll",
   "--prometheus_port","63100",
   "--tso_endpoint","tcp+k2rpc://0.0.0.0:13000"};
*/
   const int MAX_K2_ARGS = 128;
    int argc = 0;
    char** argv = (char**)malloc(MAX_K2_ARGS*sizeof(char*));
    addArg(argc,argv,"/home/kvgroup/lmx/nebula-1.0.1/build/bin/nebula-metad");
    addArg(argc,argv,"--cpo");
    addArg(argc,argv,"tcp+k2rpc://0.0.0.0:9000");
    addArg(argc,argv,"--tcp_remotes");
    addArg(argc,argv,"tcp+k2rpc://0.0.0.0:10000");
    addArg(argc,argv,"tcp+k2rpc://0.0.0.0:10001");
    addArg(argc,argv,"tcp+k2rpc://0.0.0.0:10002");
    addArg(argc,argv,"--enable_tx_checksum");
    addArg(argc,argv,"true");
    addArg(argc,argv,"--reactor-backend");
    addArg(argc,argv,"epoll");
    addArg(argc,argv,"--prometheus_port");
    addArg(argc,argv,"63100");
    addArg(argc,argv,"--tso_endpoint");
    addArg(argc,argv,"tcp+k2rpc://0.0.0.0:13000"); 
    std::cout<<"argc is: "<<argc<<std::endl;
 /*   for(int i=0;i<argc;i++){
        std::cout<<" i "<<i<<argv[i]<<std::endl;
    }
*/
    LOG(ERROR) << "seastar thread 开始  ";
    std::cout<<"before start .....\n";
    std::cout<<"before start .....\n";
    app.start(argc, argv);
}


void thriftStart() {    
    auto ioPool = std::make_shared<folly::IOThreadPoolExecutor>(FLAGS_num_io_threads);
    std::shared_ptr<apache::thrift::concurrency::ThreadManager> threadManager(
        apache::thrift::concurrency::PriorityThreadManager::newPriorityThreadManager(
                                 FLAGS_num_worker_threads, true /*stats*/));

    threadManager->setNamePrefix("executor");
    threadManager->start();  

}


int main(int argc, char* argv[]) {
    google::SetVersionString(nebula::versionString());
    std::cout<<"start nebula-metad\n\n\n";
    folly::init(&argc, &argv, true);
   
     if (FLAGS_data_path.empty()) {
        LOG(ERROR) << "Meta Data Path should not empty";
        return EXIT_FAILURE;
    }

    if (FLAGS_daemonize) {
        google::SetStderrLogging(google::FATAL);
    } else {
        google::SetStderrLogging(google::INFO);
    }

    // Detect if the server has already been started
    auto pidPath = FLAGS_pid_file;
    nebula::Status status = ProcessUtils::isPidAvailable(pidPath);
    if (!status.ok()) {
        LOG(ERROR) << status;
        return EXIT_FAILURE;
    }

    if (FLAGS_daemonize) {
        status = ProcessUtils::daemonize(pidPath);
        if (!status.ok()) {
            LOG(ERROR) << status;
            return EXIT_FAILURE;
        }
    } else {
        status = ProcessUtils::makePidFile(pidPath);
        if (!status.ok()) {
            LOG(ERROR) << status;
            return EXIT_FAILURE;
        }
    }

    auto result = nebula::network::NetworkUtils::getLocalIP(FLAGS_local_ip);
    if (!result.ok()) {
        LOG(ERROR) << "Get local ip failed! status:" << result.status();
        return EXIT_FAILURE;
    }
    auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(result.value(), FLAGS_port);
    if (!hostAddrRet.ok()) {
        LOG(ERROR) << "Bad local host addr, status:" << hostAddrRet.status();
        return EXIT_FAILURE;
    }
    nebula::HostAddr& localhost = hostAddrRet.value();
    auto peersRet = nebula::network::NetworkUtils::toHosts(FLAGS_meta_server_addrs);
    if (!peersRet.ok()) {
        LOG(ERROR) << "Can't get peers address, status:" << peersRet.status();
        return EXIT_FAILURE;
    }

/*
    auto kvstore = initKV(peersRet.value(), localhost);
    if (kvstore == nullptr) {
        LOG(ERROR) << "Init kv failed!";
        return EXIT_FAILURE;
    }

    LOG(INFO) << "Start http service";
    auto helper = std::make_unique<nebula::hdfs::HdfsCommandHelper>();
    auto pool = std::make_unique<nebula::thread::GenericThreadPool>();
    pool->start(FLAGS_meta_http_thread_num, "http thread pool");

    auto webSvc = std::make_unique<nebula::WebService>();
    status = initWebService(webSvc.get(), kvstore.get(), helper.get(), pool.get());
    if (!status.ok()) {
        LOG(ERROR) << "Init web service failed: " << status;
        return EXIT_FAILURE;
    }

    {
        nebula::meta::JobManager* jobMgr = nebula::meta::JobManager::getInstance();
        if (!jobMgr->init(kvstore.get())) {
            LOG(ERROR) << "Init job manager failed";
            return EXIT_FAILURE;
        }
    }
*/

 //   {
        /**
         *  Only leader part needed.
         */

/*        auto ret = kvstore->partLeader(nebula::meta::kDefaultSpaceId,
                                       nebula::meta::kDefaultPartId);
        if (!nebula::ok(ret)) {
            LOG(ERROR) << "Part leader get failed";
            return EXIT_FAILURE;
        }
        if (nebula::value(ret) == localhost) {
            LOG(INFO) << "Check and init root user";
            if (!nebula::meta::RootUserMan::isUserExists(kvstore.get())) {
                if(!nebula::meta::RootUserMan::initRootUser(kvstore.get())) {
                    LOG(ERROR) << "Init root user failed";
                    return EXIT_FAILURE;
                }
            }
        }
    }
*/

/*
    // Setup the signal handlers
    status = setupSignalHandler();
    if (!status.ok()) {
        LOG(ERROR) << status;
        return EXIT_FAILURE;
    }
*/
    auto handler = std::make_shared<nebula::meta::MetaServiceHandler>();
    //LOG(INFO) << "The meta deamon start on " << localhost;
   // std::thread k2Thread(seastarStart);
    try {
        gServer = std::make_unique<apache::thrift::ThriftServer>();
        gServer->setPort(FLAGS_port);

       //https://github.com/facebook/fbthrift/blob/main/thrift/doc/Cpp2.md
        gServer->setNumIOWorkerThreads(1); //网络IO的线程数
        gServer->setNumCPUWorkerThreads(1); //工作线程
        gServer->setReusePort(FLAGS_reuse_port);
        gServer->setIdleTimeout(std::chrono::seconds(10));  // No idle timeout on client connection
        gServer->setInterface(std::move(handler));
       
        gServer->serve();  // Will wait until the server shuts down
    } catch (const std::exception &e) {
        LOG(ERROR) << "Exception thrown: " << e.what();
        return EXIT_FAILURE;
    }
    
   // if(k2Thread.joinable()){
   //     k2Thread.join();
   // }

    LOG(INFO) << "The meta Daemon stopped";
    return EXIT_SUCCESS;

}




/*

nebula::Status setupSignalHandler() {
    return nebula::SignalHandler::install(
        {SIGINT, SIGTERM},
        [](nebula::SignalHandler::GeneralSignalInfo *info) {
            signalHandler(info->sig());
        });
}


void signalHandler(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            FLOG_INFO("Signal %d(%s) received, stopping this server", sig, ::strsignal(sig));
            if (gServer) {
                gServer->stop();
            }
            {
                auto gJobMgr = nebula::meta::JobManager::getInstance();
                gJobMgr->shutDown();
            }
            break;
        default:
            FLOG_ERROR("Signal %d(%s) received but ignored", sig, ::strsignal(sig));
    }
}

*/

