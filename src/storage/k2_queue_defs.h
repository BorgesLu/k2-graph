//数据结构
// graph 相关的数据重新包装到K2的结构；
//

#pragma once
#include "k2_client.h"

#include <pthread.h>
#include <atomic>
#include <future>
#include <queue>

#include "k2_includes.h"

namespace k2graph
{

    struct MyBeginTxnRequest
    {
        k2::K2TxnOptions opts;
        std::promise<k2pg::gate::K23SITxn> prom;
        
        k2::TimePoint startTime;
    };

    struct MyCollectionCreateRequest
    {
        k2::dto::CollectionCreateRequest req;
        std::promise<k2::Status> prom;
    };

    struct MySchemaCreateRequest
    {
        k2::dto::CreateSchemaRequest req;
        std::promise<k2::CreateSchemaResult> prom; //返回的future 不同
    };

    struct MySchemaGetRequest
    {
        k2::String collectionName;
        k2::String schemaName;
        uint64_t schemaVersion;
        std::promise<k2::GetSchemaResult> prom; //返回的future 不同
    };

    struct MyWriteRequest
    {
        k2::dto::K23SI_MTR mtr;
        bool erase = false;
        k2::dto::SKVRecord record;
        std::promise<k2::WriteResult> prom;
    };

    struct MyEndTxnRequest
    {
        k2::dto::K23SI_MTR mtr;
        bool shouldCommit;
        std::promise<k2::EndResult> prom;
    };

    //单点读
    struct MyReadRequest
    {
        k2::dto::K23SI_MTR mtr;
        k2::dto::SKVRecord record;
        // For key-oriented read without SKVRecord
        k2::dto::Key key = k2::dto::Key();
        std::string collectionName = "";
        std::promise<k2::ReadResult<k2::dto::SKVRecord>> prom;

        //  K2_DEF_FMT(ReadRequest, mtr, key, collectionName);
    };

    struct CreateScanReadResult
    {
        k2::Status status;
        std::shared_ptr<k2::Query> query;
        //  K2_DEF_FMT(CreateScanReadResult, status);
    };

    struct MyScanReadCreateRequest
    {
        k2::String collectionName;
        k2::String schemaName;
        std::promise<CreateScanReadResult> prom;
        // K2_DEF_FMT(ScanReadCreateRequest, collectionName, schemaName);
    };

    struct MyScanReadRequest
    {
        k2::dto::K23SI_MTR mtr;
        std::shared_ptr<k2::Query> query;
        std::promise<k2::QueryResult> prom;
        //  K2_DEF_FMT(ScanReadRequest, mtr);
    };

    struct MyUpdateRequest
    {
        k2::dto::K23SI_MTR mtr;
        k2::SKVRecord record;
        std::vector<uint32_t> fieldsForUpdate;
        k2::dto::Key key = k2::dto::Key();
        std::promise<k2::PartialUpdateResult> prom;
        // K2_DEF_FMT(UpdateRequest, mtr, fieldsForUpdate, key);
    };

    inline std::queue<MyCollectionCreateRequest> collectionCreateQ;
    inline std::queue<MySchemaCreateRequest> SchemaCreateQ;
    inline std::queue<MySchemaGetRequest> SchemaGetQ;
    inline std::queue<MyWriteRequest> WriteRequestQ;
    inline std::queue<MyBeginTxnRequest> BeginTxnQ;
    inline std::queue<MyEndTxnRequest> EndTxnQ;
    inline std::queue<MyReadRequest> readTxQ;
    inline std::queue<MyUpdateRequest> UpdateTxQ;
    // range query
    inline std::queue<MyScanReadCreateRequest> scanReadCreateTxQ;
    inline std::queue<MyScanReadRequest> scanReadTxQ;

    //访问队列的所，访问队列前均加锁
    // static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    // Lock-free mutex
    class LFMutex
    {
    private:
        std::atomic_flag _flag = ATOMIC_FLAG_INIT;

    public:
        void lock()
        {
            while (_flag.test_and_set(std::memory_order_acquire))
                ;
        }
        void unlock()
        {
            _flag.clear(std::memory_order_release);
        }
    };

    // mutex for locking outgoing request queues
    inline LFMutex requestQMutex;

    inline volatile bool finish{false};
    template <typename Q, typename Request>
    void pushQ(Q &queue, Request &&r)
    {
        std::lock_guard lock{requestQMutex};
        // if (finish){    //返回错误信息
        //  r.prom->set_exception(std::make_exception_ptr(std::runtime_error("queue processing has been shutdown")));
        //  std::cout<<"queue processing has been shutdown"<<std::endl;
        //  }
        //  else{
        queue.push(std::forward<Request>(r));
        //   }
        // queue.push(std::forward<Request>(r));
        // queue.push(r);
    }

} // end of k2grapg