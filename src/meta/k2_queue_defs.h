//数据结构
//graph 相关的数据重新包装到K2的结构；
//

#pragma once

#include <atomic>
#include <future>
#include <queue>

#include "k2_includes.h"


namespace k2graph{
struct MyCollectionCreateRequest {
    k2::dto::CollectionCreateRequest req;
    // seastar::promise<k2::Status>* prom;
    std::promise<k2::Status> *prom;
    // std::promise<k2::Status>* prom = new std::promise<k2::Status>();
    // K2_DEF_FMT(MyCollectionCreateRequest, req);
};

struct MySchemaCreateRequest {
    k2::dto::CreateSchemaRequest req;
    std::promise<k2::CreateSchemaResult> *prom; //返回的future 不同
};

struct MySchemaGetRequest {
    k2::String collectionName;
    k2::String schemaName;
    uint64_t schemaVersion;
    std::promise<k2::GetSchemaResult> *prom; //返回的future 不同
};

/*
struct MyWriteRequest {
    k2::dto::K23SI_MTR mtr;
    // k2::K2TxnHandle txn;
    bool erase = false;
    //前提条件默认为None，暂时不做处理
    k2::dto::ExistencePrecondition precondition = k2::dto::ExistencePrecondition::None;
    k2::SKVRecord record;
    std::promise<k2::WriteResult> *prom;
};
*/
struct MyBeginTxnRequest {
    k2::K2TxnOptions opts;
    std::promise<k2::dto::K23SI_MTR> *prom;
    k2::TimePoint startTime;
};

struct MyEndTxnRequest {
    k2::dto::K23SI_MTR mtr;
    bool shouldCommit;
    std::promise<k2::EndResult> *prom;
};



inline std::queue<MyCollectionCreateRequest> collectionCreateQ;
inline std::queue<MySchemaCreateRequest> SchemaCreateQ;
inline std::queue<MySchemaGetRequest> SchemaGetQ;
//inline std::queue<MyWriteRequest> WriteRequestQ;
inline std::queue<MyBeginTxnRequest> BeginTxnQ;
inline std::queue<MyEndTxnRequest> EndTxnQ;

inline volatile bool finish{false};
template <typename Q, typename Request>
void pushQ(Q &queue, Request &&r)
{
    // std::lock_guard lock{requestQMutex};
    if (finish){    //返回错误信息
        r.prom->set_exception(std::make_exception_ptr(std::runtime_error("queue processing has been shutdown")));
    }
    else{
        queue.push(std::forward<Request>(r));
    }
    // queue.push(std::forward<Request>(r));
    // queue.push(r);
}


}//end of k2grapg