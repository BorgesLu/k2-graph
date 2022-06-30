/* Copyright (c) 2018 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "storage/StorageServiceHandler.h"
#include "base/Base.h"
#include "storage/query/QueryBoundProcessor.h"
#include "storage/query/QueryVertexPropsProcessor.h"
#include "storage/query/QueryEdgePropsProcessor.h"
#include "storage/query/QueryStatsProcessor.h"
#include "storage/query/GetUUIDProcessor.h"
#include "storage/query/ScanEdgeProcessor.h"
#include "storage/query/ScanVertexProcessor.h"
#include "storage/mutate/AddVerticesProcessor.h"
#include "storage/mutate/AddEdgesProcessor.h"
#include "storage/mutate/DeleteVerticesProcessor.h"
#include "storage/mutate/DeleteEdgesProcessor.h"
#include "storage/mutate/UpdateVertexProcessor.h"
#include "storage/mutate/UpdateEdgeProcessor.h"
#include "storage/kv/PutProcessor.h"
#include "storage/kv/GetProcessor.h"
#include "storage/admin/AdminProcessor.h"
#include "storage/admin/CreateCheckpointProcessor.h"
#include "storage/admin/DropCheckpointProcessor.h"
#include "storage/admin/SendBlockSignProcessor.h"
#include "storage/admin/RebuildTagIndexProcessor.h"
#include "storage/admin/RebuildEdgeIndexProcessor.h"
#include "storage/index/LookUpIndexProcessor.h"

// sleep
#include <unistd.h>

#include "storage/k2_queue_defs.h"
#include "dataman/SchemaWriter.h"
#include "dataman/NebulaCodecImpl.h"
// String to int
#include <stdlib.h>
// GraphView
#include "GraphView.h"
#include "K2Vertex.h"
#include <k2/common/Common.h>

#define RETURN_FUTURE(processor)     \
    auto f = processor->getFuture(); \
    processor->process(req);         \
    return f;

DEFINE_int32(vertex_cache_num, 16 * 1000 * 1000, "Total keys inside the cache");
DEFINE_int32(vertex_cache_bucket_exp, 4, "Total buckets number is 1 << cache_bucket_exp");
DEFINE_int32(reader_handlers, 32, "Total reader handlers");
DEFINE_string(reader_handlers_type, "cpu", "Type of reader handlers, options: cpu,io");

//写入时构建table，
//写入数据时解码数据 读数据时 编码数据都可以使用
// Todo:元数据的同步，schema改变，本地的信息也能同时更新
static std::unordered_map<int32_t, std::shared_ptr<k2::dto::Schema>> SchemaTable;
static std::unordered_map<int32_t, nebula::cpp2::Schema> NebulaSchemaTable;
// static int64_t total_vertex = 0;

static std::string SerializeSKVRecordToString(k2::dto::SKVRecord &record)
{
    const k2::dto::SKVRecord::Storage &storage = record.getStorage();
    // k2::BinaryAllocator MyDefaultAllocator(size_t default_size = 8196);
    // size_t default_size = 8196;
    // k2::Payload payload(k2::Payload::DefaultAllocator(default_size));
    k2::Payload payload(k2::Payload::DefaultAllocator);
    // k2::Payload::DefaultAllocator
    //  Since Storage itself contains a nested Payload, we cannot do anything fancy to avoid the extra
    //  copy to a new payload here, because the implementation of write() with a payload argument is to
    //  share the underlying buffers but here we need one continguous piece of memory
    payload.write(storage);
    payload.seek(0);
    std::string serialized(payload.getSize(), '\0');
    payload.read(serialized.data(), payload.getSize());

    return serialized;
}

// 从K2-platform读取Schema
static std::shared_ptr<k2::dto::Schema> GetSchemaFromK2(int64_t spaceID, int32_t schemaID)
{
    k2graph::MySchemaGetRequest request{
        .collectionName = std::to_string(spaceID),
        .schemaName = std::to_string(schemaID),
        .schemaVersion = 1,
        //.prom = new std::promise<k2::GetSchemaResult>()};
        //.prom =  std::promise<k2::GetSchemaResult>()};
        .prom = {}};
    std::cout << "L90\n";
    auto result = request.prom.get_future();
    pushQ(k2graph::SchemaGetQ, std::move(request));
    std::cout << "after pushQ L92\n\n";
    try
    {
        // auto result = request.prom->get_future();
        //  auto result = request.prom.get_future();
        // sleep(1);
        std::cout << "\n\n99\n";
        auto schemaResult = result.get();
        auto status = schemaResult.status;
        if (!status.is2xxOK())
        {
            std::cout << "获取schema时出错\n";
        }
        std::shared_ptr<k2::dto::Schema> schema = schemaResult.schema;
        std::cout << "\n\nget schema success\n\n";
        //跟新SchemaTable
        SchemaTable[schemaID] = schema;
        return schema;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
namespace nebula
{
    namespace storage
    {

        //遍历操作，从顶点ID，拿邻居边
        folly::Future<cpp2::QueryResponse>
        StorageServiceHandler::future_getBound(const cpp2::GetNeighborsRequest &req)
        {

            folly::Promise<cpp2::QueryResponse> promise_;
            auto f = promise_.getFuture();
            cpp2::QueryResponse resp_;
            cpp2::ResponseCommon responseCommon;
            std::vector<cpp2::ResultCode> failed_codes;

            //通过return_columns 来获取tagID?
            // auto colSize = req.get_return_columns().size();
            // std::cout << "getBound return_columns is " << colSize << "\n";
            cpp2::PropDef first_col = req.get_return_columns().front();
            auto col_name = first_col.get_name();
            auto col_edgeID = first_col.get_id().get_edge_type();
            // std::cout << "get bound return_columns name is: " << col_name << "\n";
            // std::cout << "get bound return_columns id is: " << col_edgeID << "\n";

            //构建返回的schema
            std::shared_ptr<k2::dto::Schema> schema;

            if (SchemaTable.find(col_edgeID) != SchemaTable.end())
            {
                schema = SchemaTable[col_edgeID];
            }
            else
            {
                schema = GetSchemaFromK2(req.get_space_id(), col_edgeID);
            }

            auto fields = schema->fields;

            /*
            nebula::SchemaWriter schemaWriter;

            //跳过前面的固定字段 PartID-VertexID-EdgeType-Rank-VertexID
            for (auto field = fields.begin() + 5; field != fields.end(); field++)
            {
                auto name = field->name.c_str();
                std::cout<<"L111 "<<"name is "<<name<<std::endl;
                auto type = field->type;
                std::cout<<"L111 "<<"type is "<<type<<std::endl;
                switch (type)
                { // VID类型对应INT64
                case k2::dto::FieldType::STRING:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::STRING);
                    break;
                case k2::dto::FieldType::INT16T:
                case k2::dto::FieldType::INT32T:
                case k2::dto::FieldType::INT64T:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::INT);
                    break;
                case k2::dto::FieldType::FLOAT:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::FLOAT);
                    break;
                case k2::dto::FieldType::DOUBLE:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::DOUBLE);
                    break;
                case k2::dto::FieldType::BOOL:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::BOOL);
                    break;
                default:
                    //构造错误信息,并返回
                    // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                    cpp2::ResultCode rescode;
                    rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                    failed_codes.emplace_back(rescode);
                    responseCommon.set_failed_codes(failed_codes);
                    resp_.set_result(responseCommon);
                    return resp_;
                }
            }

            nebula::cpp2::Schema edge_schema = schemaWriter.moveSchema();
            auto nebula_schema = std::make_shared<nebula::ResultSchemaProvider>(edge_schema);
            nebula::dataman::NebulaCodecImpl codec;
            */
            //构建返回的 vertex_schema  不需要返回schema 信息
            // std::unordered_map<nebula::cpp2::TagID, nebula::cpp2::Schema> my_edge_schema;
            // my_edge_schema.insert(std::make_pair(col_edgeID, edge_schema));

            //开始一个事务
            k2::K2TxnOptions options{};
            options.syncFinalize = true;
            k2graph::MyBeginTxnRequest qr{.opts = options,
                                          // .prom = new std::promise<k2::dto::K23SI_MTR>(),
                                          .prom = std::promise<k2::dto::K23SI_MTR>(),
                                          .startTime = k2::Clock::now()};
            pushQ(k2graph::BeginTxnQ, std::move(qr));
            k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
            try
            {
                // auto result = qr.prom->get_future();
                auto result = qr.prom.get_future();
                mtr = result.get();
            }
            catch (...)
            {
                //构造错误信息,并返回
                cpp2::ResultCode rescode;
                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                failed_codes.emplace_back(rescode);
                responseCommon.set_failed_codes(failed_codes);
                resp_.set_result(responseCommon);
                return resp_;
            }

            auto spaceID = req.space_id;
            std::string spaceName = std::to_string(spaceID);
            //保存返回的顶点和边的数据
            std::vector<cpp2::VertexData> scan_vertices;
            //返回的边的总数量，默认设为一
            int32_t edge_number = 1;

            for (auto iter = req.parts.begin(); iter != req.parts.end(); iter++)
            {
                auto vertexIDs = iter->second;
                int16_t partID = iter->first;
                for (auto vertexID : vertexIDs)
                {
                    // cpp2::VertexData vResp;
                    // process vertex 为每一个顶点遍历其邻居边
                    cpp2::VertexData vdata;
                    // std::cout << "L186 vertexID is" << vertexID << "\n";
                    vdata.set_vertex_id(vertexID);
                    std::vector<cpp2::EdgeData> edge_data_vector;
                    std::shared_ptr<k2::Query> scan = nullptr;
                    for (auto edgeType : req.edge_types)
                    {
                        // create query
                        k2graph::MyScanReadCreateRequest request{
                            .collectionName = std::to_string(spaceID),
                            .schemaName = std::to_string(edgeType),
                            // .prom = new std::promise<k2graph::CreateScanReadResult>()};
                            .prom = std::promise<k2graph::CreateScanReadResult>()};
                        pushQ(k2graph::scanReadCreateTxQ, std::move(request));
                        try
                        {
                            // auto scan_create_result = request.prom->get_future().get();
                            auto scan_create_result = request.prom.get_future().get();
                            auto status = scan_create_result.status;
                            if (!status.is2xxOK())
                            {
                                std::cout << "createQuery 错误"
                                          << "\n";
                                cpp2::ResultCode rescode;
                                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                failed_codes.emplace_back(rescode);
                                responseCommon.set_failed_codes(failed_codes);
                                resp_.set_result(responseCommon);
                                // return resp_;
                                promise_.setValue(std::move(resp_));
                                return f;
                            }
                            scan = scan_create_result.query;
                        }
                        catch (...)
                        {
                            //构造错误信息,并返回
                            //因为遇到异常，直接返回
                            cpp2::ResultCode rescode;
                            rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                            failed_codes.emplace_back(rescode);
                            responseCommon.set_failed_codes(failed_codes);
                            resp_.set_result(responseCommon);
                            return resp_;
                        }

                        std::string dstName = "SecondVertexID";
                        //通过投影，只获取dst这一列数据？
                        scan->addProjection(dstName);
                        std::shared_ptr<k2::dto::Schema> schema = scan->startScanRecord.schema;
                        //构造startRecord 和 endRecoed,二者相同
                        // partID--vertexID--edge--Type
                        k2::dto::SKVRecord startRecord(spaceName, schema);
                        startRecord.serializeNext<int16_t>(partID);
                        startRecord.serializeNext<int64_t>(vertexID);
                        startRecord.serializeNext<int32_t>(edgeType);
                        scan->startScanRecord = std::move(startRecord);
                        k2::dto::SKVRecord ednRecord(spaceName, schema);
                        ednRecord.serializeNext<int16_t>(partID);
                        ednRecord.serializeNext<int64_t>(vertexID);
                        ednRecord.serializeNext<int32_t>(edgeType);
                        scan->endScanRecord = std::move(ednRecord);

                        // range query
                        k2graph::MyScanReadRequest scan_request{
                            .mtr = mtr,
                            .query = std::move(scan),
                            // .prom = new std::promise<k2::QueryResult>()};
                            .prom = std::promise<k2::QueryResult>()};
                        pushQ(k2graph::scanReadTxQ, std::move(scan_request));

                        //  k2::QueryResult scan_result = scan_request.prom->get_future().get();
                        k2::QueryResult scan_result = scan_request.prom.get_future().get();
                        auto status = scan_result.status;
                        // std::cout << "\n255\n"
                        //   << status << "\n";
                        std::vector<k2::dto::SKVRecord> scan_result_records(std::move(scan_result.records));
                        // std::cout << "L255 读上来的数据个数 " << scan_result_records.size() << "\n";
                        edge_number = scan_result_records.size();
                        // std::vector<std::string> edge_vector;

                        bool isSucceed = true;
                        k2graph::MyEndTxnRequest end_txn_req{
                            .mtr = mtr,
                            .shouldCommit = isSucceed,
                            // .prom = new std::promise<k2::EndResult>()};
                            .prom = std::promise<k2::EndResult>()};

                        pushQ(k2graph::EndTxnQ, std::move(end_txn_req));

                        std::vector<cpp2::IdAndProp> idAndProps;
                        //读skvrecord, 构造返回值
                        for (auto edge = scan_result_records.begin(); edge != scan_result_records.end(); edge++)
                        {
                            // int64_t dstID = edge.deserializeField<int64_t>("SecondVertexID");
                            // int64_t dstID = static_cast<int64_t>(*edge);
                            // std::string edge_data = SerializeSKVRecordToString(*edge);

                            // int64_t dstID = atoi(edge_data.c_str());
                            std::string name = "SecondVertexID";
                            auto dstID = (*edge).deserializeField<int64_t>(name);
                            // std::cout << "\n"<<
                            //  "dstID form SKVRecord is " <<(int64_t)* dstID << "\n\n";

                            cpp2::IdAndProp idAndProp;
                            idAndProp.set_dst((int64_t)*dstID);
                            // std::cout << "\n"<<
                            // "dstID form idAndProp is " <<idAndProp.get_dst()<< "\n\n";

                            idAndProps.emplace_back(std::move(idAndProp));
                        }
                        cpp2::EdgeData edge_Data;
                        edge_Data.set_type(edgeType);
                        edge_Data.set_edges(std::move(idAndProps));
                        edge_data_vector.emplace_back(std::move(edge_Data));
                        // vdata.set_edge_data.emplace_back(std::move(edge_Data));

                    } // end of process vertex
                    vdata.set_edge_data(std::move(edge_data_vector));
                    scan_vertices.emplace_back(std::move(vdata));
                } // end of VID list

            } // end for map parts
            //结束事务
            bool isSucceed = true;
            k2graph::MyEndTxnRequest end_txn_req{
                .mtr = mtr,
                .shouldCommit = isSucceed,
                //.prom = new std::promise<k2::EndResult>()};
                .prom = std::promise<k2::EndResult>()};

            pushQ(k2graph::EndTxnQ, std::move(end_txn_req));

            // resp_.set_edge_schema(std::move(my_edge_schema));
            // std::cout << "L313 get_edge_schema()->size()" << resp_.get_edge_schema()->size() << "\n\n";
            // std::cout<<"scan_vertices is "<<scan_vertices.size()<<"\n\n";
            // int32_t edge_number = 1;
            resp_.set_total_edges(edge_number);
            resp_.set_vertices(std::move(scan_vertices));
            promise_.setValue(std::move(resp_));
            std::cout << "getBound end\n";
            return f;

        } // end for getBound

        /*
                folly::Future<cpp2::QueryStatsResponse>
                StorageServiceHandler::future_boundStats(const cpp2::GetNeighborsRequest& req) {
                    auto* processor = QueryStatsProcessor::instance(kvstore_,
                                                                    schemaMan_,
                                                                    &boundStatsQpsStat_,
                                                                    readerPool_.get(),
                                                                    &vertexCache_);
                    RETURN_FUTURE(processor);
                }
        */
        // 读到的SKVRecord 重构为 binary
        //  返回 query engine

        folly::Future<cpp2::QueryResponse>
        StorageServiceHandler::future_getProps(const cpp2::VertexPropRequest &req)
        {
            folly::Promise<cpp2::QueryResponse> promise_;
            auto f = promise_.getFuture();
            cpp2::QueryResponse resp_;
            cpp2::ResponseCommon responseCommon;
            std::vector<cpp2::ResultCode> failed_codes;

            //通过return_columns 来获取tagID
            // auto colSize = req.get_return_columns().size();
            // std::cout << "getProp return_columns is: " << colSize << "\n";

            //暂时只处理一次读一个顶点，colSize大小为1
            cpp2::PropDef first_col = req.get_return_columns().front();
            auto col_name = first_col.get_name();
            auto col_tagID = first_col.get_id().get_tag_id();
            // std::cout << "return_columns name is: " << col_name << "\n";
            // std::cout << "return_columns id is: " << col_tagID << "\n";

            //构建编码的schema
            std::shared_ptr<k2::dto::Schema> schema;
            schema = SchemaTable[col_tagID];

            auto fields = schema->fields;
            nebula::SchemaWriter schemaWriter;

            for (auto field = fields.begin(); field != fields.end(); field++)
            {
                //跳过Key partID VertexID tagID
                if (field == fields.begin())
                {
                    field++;
                    field++;
                    continue;
                }
                auto name = field->name.c_str();
                // std::cout << "field->type is in L291:" << field->type;
                auto type = field->type;
                switch (type)
                { // VID类型对应INT64
                case k2::dto::FieldType::STRING:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::STRING);
                    break;
                case k2::dto::FieldType::INT16T:
                case k2::dto::FieldType::INT32T:
                case k2::dto::FieldType::INT64T:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::INT);
                    break;
                case k2::dto::FieldType::FLOAT:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::FLOAT);
                    break;
                case k2::dto::FieldType::DOUBLE:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::DOUBLE);
                    break;
                case k2::dto::FieldType::BOOL:
                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::BOOL);
                    break;
                default:
                    //构造错误信息,并返回
                    // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                    cpp2::ResultCode rescode;
                    rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                    failed_codes.emplace_back(rescode);
                    responseCommon.set_failed_codes(failed_codes);
                    resp_.set_result(responseCommon);
                    return resp_;
                }
            }
            nebula::cpp2::Schema tag_schema = schemaWriter.moveSchema();
            auto nebula_schema = std::make_shared<nebula::ResultSchemaProvider>(tag_schema);
            nebula::dataman::NebulaCodecImpl codec;
            //构建返回的 vertex_schema
            std::unordered_map<nebula::cpp2::TagID, nebula::cpp2::Schema> my_vertex_schema;
            my_vertex_schema.insert(std::make_pair(col_tagID, tag_schema));
            // std::cout << "\n"
            //    << "my_vertex_schema size " << my_vertex_schema.size() << "\n";
            //  resp_.set_vertex_schema(std::move(my_vertex_schema));

            //开始一个事务 TODO:单独设置为一个函数
            k2::K2TxnOptions options{};
            options.syncFinalize = true;
            k2graph::MyBeginTxnRequest qr{.opts = options,
                                          //.prom = new std::promise<k2::dto::K23SI_MTR>(),
                                          .prom = std::promise<k2::dto::K23SI_MTR>(),
                                          .startTime = k2::Clock::now()};
            pushQ(k2graph::BeginTxnQ, std::move(qr));
            k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
            try
            {
                // auto result = qr.prom->get_future();
                auto result = qr.prom.get_future();
                mtr = result.get();
            }
            catch (...)
            {
                //构造错误信息,并返回
                cpp2::ResultCode rescode;
                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                failed_codes.emplace_back(rescode);
                responseCommon.set_failed_codes(failed_codes);
                resp_.set_result(responseCommon);
                return resp_;
            }

            std::vector<k2graph::MyReadRequest> request_list;
            std::vector<k2::dto::SKVRecord> skvrecord_list;

            //对每一个顶点构造一个skvrecord用于构造请求。
            auto spaceId = req.space_id;
            // std::vector<int64_t> Vertex_ID_Vec;
            int return_VID;

            for (auto iter = req.parts.begin(); iter != req.parts.end(); iter++)
            {
                auto verticeIDs = iter->second;

                // Vertex_ID_Vec(verticeIDs);
                for (auto vertexid : verticeIDs)
                {
                    //先获取schema
                    return_VID = vertexid;
                    // std::cout << "return VID is: " << return_VID << "\n";

                    std::shared_ptr<k2::dto::Schema> schema;
                    auto tagID = col_tagID;

                    //没找到,需要获取schema，之后可以单独修改成一个函数
                    k2graph::MySchemaGetRequest request{
                        .collectionName = std::to_string(spaceId),
                        .schemaName = std::to_string(tagID),
                        .schemaVersion = 1, //目前所有schema的version均为1，之后可能需要进一步修改
                                            // .prom = new std::promise<k2::GetSchemaResult>()};
                        .prom = std::promise<k2::GetSchemaResult>()};
                    pushQ(k2graph::SchemaGetQ, std::move(request));
                    try
                    {
                        // auto result = request.prom->get_future();
                        auto result = request.prom.get_future();
                        auto schemaResult = result.get();
                        auto status = schemaResult.status;
                        // std::cout << "\n\n\n\n"
                        //           << status << "\n\n\n\n";
                        if (!status.is2xxOK())
                        { //获取schema时出错
                            //构造错误信息,并返回
                            // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                            std::cout << "获取schema时出错\n";

                            cpp2::ResultCode rescode;
                            rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                            failed_codes.emplace_back(rescode);
                            responseCommon.set_failed_codes(failed_codes);
                            resp_.set_result(responseCommon);
                            // return resp_;
                            promise_.setValue(std::move(resp_));
                            return f;
                        }
                        schema = schemaResult.schema;
                    }
                    catch (...)
                    {
                        //构造错误信息,并返回
                        //因为遇到异常，直接返回
                        cpp2::ResultCode rescode;
                        rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                        failed_codes.emplace_back(rescode);
                        responseCommon.set_failed_codes(failed_codes);
                        resp_.set_result(responseCommon);
                        return resp_;
                    }
                    //构造查询用的skvrecord
                    k2::dto::SKVRecord skvRecord(std::to_string(spaceId), schema);
                    //已经将meta中前三个field全部设置成INT64
                    skvRecord.serializeNext<int16_t>(iter->first);
                    skvRecord.serializeNext<int64_t>(vertexid);
                    skvRecord.serializeNext<int32_t>(tagID);
                    skvrecord_list.push_back(std::move(skvRecord));
                }
            }

            for (auto iter = skvrecord_list.begin(); iter != skvrecord_list.end(); iter++)
            {
                k2::dto::SKVRecord _record = std::move(*(iter));
                k2::dto::Key _key = _record.getKey();
                k2graph::MyReadRequest read_request{
                    .mtr = mtr,
                    .record = k2::dto::SKVRecord(),
                    .key = std::move(_key),
                    .collectionName = std::to_string(spaceId),
                    //  .prom = new std::promise<k2::ReadResult<k2::dto::SKVRecord>>()};
                    .prom = std::promise<k2::ReadResult<k2::dto::SKVRecord>>()};
                pushQ(k2graph::readTxQ, std::move(read_request));
                request_list.push_back(std::move(read_request));
            }

            bool isSucceed = true;
            std::vector<cpp2::VertexData> vertices;
            for (auto request = request_list.begin(); request != request_list.end(); request++)
            {
                // auto result = request->prom->get_future();
                auto result = request->prom.get_future();
                auto MyReadResult = result.get();
                auto status = MyReadResult.status;
                if (!status.is2xxOK())
                {
                    isSucceed = false;
                    break;
                }
                std::cout << "\nL447\n"
                          << status << "\n";

                std::vector<boost::any> v_tag_data;
                for (auto field = fields.begin(); field != fields.end(); field++)
                {
                    if (field == fields.begin())
                    {
                        field++;
                        field++;
                        continue;
                    }
                    auto name = field->name.c_str();
                    std::cout << "name 460 is" << name << "\n";
                    //只支持 int_64的字段
                    auto skv_filed = MyReadResult.value.deserializeField<int64_t>(name);
                    std::cout << "name 463 is" << (int64_t)*skv_filed << "\n";
                    v_tag_data.emplace_back(*skv_filed);
                }

                auto result_tag_data = codec.encode(v_tag_data, nebula_schema);
                cpp2::VertexData vResp;
                //限定一个顶点只有一个tag
                std::vector<cpp2::TagData> tds;
                cpp2::TagData td;
                int32_t tagID = col_tagID;
                td.set_tag_id(tagID);
                // td.set_data(std::move(result_tag_data));
                td.set_data(std::move(result_tag_data));
                tds.emplace_back(std::move(td));
                // int64_t vid = 101;

                vResp.set_vertex_id(return_VID);
                vResp.set_tag_data(std::move(tds));

                vertices.emplace_back(std::move(vResp));

                std::cout << "vertices size is: " << vertices.size() << "\n";
            }

            k2graph::MyEndTxnRequest end_txn_req{
                .mtr = mtr,
                .shouldCommit = isSucceed,
                //  .prom = new std::promise<k2::EndResult>()};
                .prom = std::promise<k2::EndResult>()};

            pushQ(k2graph::EndTxnQ, std::move(end_txn_req));

            resp_.set_vertices(std::move(vertices));
            resp_.set_vertex_schema(std::move(my_vertex_schema));
            std::cout << "L500 " << resp_.get_vertex_schema()->size() << "\n\n";

            promise_.setValue(std::move(resp_));
            std::cout << "getProp end\n";
            return f;
        }

        folly::Future<cpp2::EdgePropResponse>
        StorageServiceHandler::future_getEdgeProps(const cpp2::EdgePropRequest &req)
        {
            folly::Promise<cpp2::EdgePropResponse> promise_;
            auto f = promise_.getFuture();
            cpp2::EdgePropResponse resp_;
        }

        static int countVertex = 0;
        folly::Future<cpp2::ExecResponse>
        StorageServiceHandler::future_addVertices(const cpp2::AddVerticesRequest &req)
        {
            //只支持一次插入一个顶点的函数
            std::cout << "part size is: " << req.parts.size() << std::endl;
            countVertex++;
            //  std::cout<<"countVertex is: "<<countVertex<<std::endl;
            if (countVertex % 500 == 0)
            {
                std::cout << countVertex << std::endl;
                sleep(5); //写得太快了？
            }
            folly::Promise<cpp2::ExecResponse> promise_;
            auto f = promise_.getFuture();
            cpp2::ExecResponse resp_;
            cpp2::ResponseCommon responseCommon;
            std::vector<cpp2::ResultCode> failed_codes;
            std::vector<k2::dto::SKVRecord> skvrecord_list;
            bool isSucceed = true;

            auto spaceId = req.space_id;
            std::cout << "l698\n";

            for (auto iter = req.parts.begin(); iter != req.parts.end(); iter++)
            {
                auto vertices = iter->second;
                for (auto vertex : vertices)
                {
                    auto tags = vertex.tags;
                    for (auto tag : tags)
                    {
                        std::shared_ptr<k2::dto::Schema> schema;
                        auto tagID = tag.tag_id;
                        if (SchemaTable.find(tagID) != SchemaTable.end())
                        {
                            // std::cout<<"跳过获取schema\n";
                            schema = SchemaTable[tagID];
                        }

                        else
                        {
                            try
                            {
                                schema = GetSchemaFromK2(spaceId, tagID);
                            }
                            catch (...)
                            {
                                //构造错误信息,并返回
                                //因为遇到异常，直接返回
                                cpp2::ResultCode rescode;
                                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                failed_codes.emplace_back(rescode);
                                responseCommon.set_failed_codes(failed_codes);
                                resp_.set_result(responseCommon);
                                return resp_;
                            }
                        }
                        std::cout << "L734\n\n";
                        // Nebula Schema用于解码
                        auto fields = schema->fields;
                        nebula::cpp2::Schema k2TagSchema;
                        if (NebulaSchemaTable.find(tagID) != NebulaSchemaTable.end())
                        {
                            k2TagSchema = NebulaSchemaTable[tagID];
                            std::cout << "跳过构造schema\n";
                        }
                        else
                        {
                            // std::cout << fields;
                            nebula::SchemaWriter schemaWriter; //用于构造nebula中的schema
                            for (auto field = fields.begin(); field != fields.end(); field++)
                            {
                                std::cout << "L 747\n";
                                if (field == fields.begin())
                                {
                                    field++;
                                    field++;
                                    continue;
                                }
                                auto name = field->name.c_str();
                                auto type = field->type;

                                switch (type)
                                { // VID类型对应INT64
                                case k2::dto::FieldType::STRING:
                                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::STRING);
                                    break;
                                case k2::dto::FieldType::INT16T:
                                case k2::dto::FieldType::INT32T:
                                case k2::dto::FieldType::INT64T:
                                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::INT);
                                    break;
                                case k2::dto::FieldType::FLOAT:
                                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::FLOAT);
                                    break;
                                case k2::dto::FieldType::DOUBLE:
                                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::DOUBLE);
                                    break;
                                case k2::dto::FieldType::BOOL:
                                    schemaWriter.appendCol(name, nebula::cpp2::SupportedType::BOOL);
                                    break;
                                default:
                                    //构造错误信息,并返回
                                    // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                                    cpp2::ResultCode rescode;
                                    rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                    failed_codes.emplace_back(rescode);
                                    responseCommon.set_failed_codes(failed_codes);
                                    resp_.set_result(responseCommon);
                                    return resp_;
                                }
                            }
                            std::cout << "L 787\n";
                            k2TagSchema = schemaWriter.moveSchema();
                            NebulaSchemaTable[tagID] = k2TagSchema;
                        }
                        auto nebula_schema = std::make_shared<nebula::ResultSchemaProvider>(k2TagSchema);
                        nebula::dataman::NebulaCodecImpl codec;
                        auto result = codec.decode(tag.get_props(), nebula_schema);
                        k2::dto::SKVRecord skvRecord(std::to_string(spaceId), schema);
                        skvRecord.serializeNext<int16_t>(iter->first);
                        skvRecord.serializeNext<int64_t>(vertex.get_id());
                        skvRecord.serializeNext<int32_t>(tag.get_tag_id());
                        std::cout << "L798\n";
                        for (auto field = fields.begin(); field != fields.end(); field++)
                        {
                            if (field == fields.begin())
                            {
                                field++;
                                field++;
                                continue;
                            }
                            auto name = field->name.c_str(); //获得name的std::string格式
                            auto type = field->type;
                            //在result中查找是否存在此field，不存在则查看是否可以置为null
                            auto value = result.value().find(name);
                            try
                            {
                                if (value != result.value().end())
                                { //查找到name，直接根据类型序列化值
                                    switch (type)
                                    {
                                    case k2::dto::FieldType::STRING:
                                        skvRecord.serializeNext<k2::String>(boost::any_cast<std::string>(value->second));
                                        break;
                                    case k2::dto::FieldType::INT16T:
                                        skvRecord.serializeNext<int16_t>(boost::any_cast<int16_t>(value->second));
                                        break;
                                    case k2::dto::FieldType::INT32T:
                                        skvRecord.serializeNext<int32_t>(boost::any_cast<int32_t>(value->second));
                                        break;
                                    case k2::dto::FieldType::INT64T:
                                        skvRecord.serializeNext<int64_t>(boost::any_cast<int32_t>(value->second));
                                        break;
                                    case k2::dto::FieldType::FLOAT:
                                        skvRecord.serializeNext<float>(boost::any_cast<float>(value->second));
                                        break;
                                    case k2::dto::FieldType::DOUBLE:
                                        skvRecord.serializeNext<double>(boost::any_cast<double>(value->second));
                                        break;
                                    case k2::dto::FieldType::BOOL:
                                        skvRecord.serializeNext<bool>(boost::any_cast<bool>(value->second));
                                        break;
                                    default:
                                        //构造错误信息,并返回
                                        // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                                        cpp2::ResultCode rescode;
                                        rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                        failed_codes.emplace_back(rescode);
                                        responseCommon.set_failed_codes(failed_codes);
                                        resp_.set_result(responseCommon);
                                        return resp_;
                                    }
                                }
                                else
                                {
                                    //未找到值序列化null
                                    skvRecord.serializeNull();
                                }
                            }
                            catch (...)
                            {
                                //构造错误信息,并返回
                                cpp2::ResultCode rescode;
                                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                failed_codes.emplace_back(rescode);
                                responseCommon.set_failed_codes(failed_codes);
                                resp_.set_result(responseCommon);
                                return resp_;
                            }
                        }
                        skvrecord_list.push_back(std::move(skvRecord));
                    }
                }
            }
            std::cout << "l860\n";
            k2::K2TxnOptions options{};
            options.syncFinalize = true;
            k2graph::MyBeginTxnRequest qr{.opts = options,
                                          //   .prom = new std::promise<k2::dto::K23SI_MTR>(),
                                          .prom = std::promise<k2::dto::K23SI_MTR>(),
                                          .startTime = k2::Clock::now()};
            auto BeginTxnQResult = qr.prom.get_future();
            std::cout << "880\n";
            pushQ(k2graph::BeginTxnQ, std::move(qr));

            k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
            try
            {
                
                // auto BeginTxnQResult = qr.prom.get_future();
                mtr = BeginTxnQResult.get();
                std::cout << "l885\n";
            }
            catch (...)
            {
                //构造错误信息,并返回
                std::cout << "l893\n";
                cpp2::ResultCode rescode;
                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                failed_codes.emplace_back(rescode);
                responseCommon.set_failed_codes(failed_codes);
                resp_.set_result(responseCommon);
                return resp_;
            }
            std::cout << "l898\n";
            k2graph::MyWriteRequest write_request{
                .mtr = mtr,
                .record = std::move(skvrecord_list[0]), //从上面的序列化得出
                                                        //  .prom = new std::promise<k2::WriteResult>()};
                .prom = std::promise<k2::WriteResult>()};
            std::cout << "l904\n";
            auto WQResult = write_request.prom.get_future();
            pushQ(k2graph::WriteRequestQ, std::move(write_request));
            std::cout << "l908\n";
            try
            {
                // auto result = write_request.prom->get_future();
                // auto result = write_request.prom.get_future();

                auto WriteResult = WQResult.get();
                auto status = WriteResult.status;
                std::cout << "l914\n";
                if (!status.is2xxOK())
                {
                    std::cout << "L918\n";
                    isSucceed = false;
                }
            }
            catch (...)
            {
                std::cout << "l923\n";
                isSucceed = false;
            }

            bool isendsucceed = true;
            k2graph::MyEndTxnRequest end_txn_req{
                .mtr = mtr,
                .shouldCommit = isSucceed,
                //.prom = new std::promise<k2::EndResult>()
                .prom = std::promise<k2::EndResult>()};
            std::cout << "l934\n";
            auto EndQResult = end_txn_req.prom.get_future();

            pushQ(k2graph::EndTxnQ, std::move(end_txn_req));

            // auto result = end_txn_req.prom->get_future();
            // auto result = end_txn_req.prom.get_future();

            auto EndResult = EndQResult.get();
            auto status = EndResult.status;
            if (!status.is2xxOK())
            {
                isendsucceed = false;
            }
            cpp2::ResultCode rescode;
            if (!isendsucceed || !isSucceed)
            {
                std::cout << "error L955\n\n\n\n";
                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                failed_codes.emplace_back(rescode);
                responseCommon.set_failed_codes(failed_codes);
                resp_.set_result(responseCommon);
                promise_.setValue(std::move(resp_));
                return f;
            }
            responseCommon.set_failed_codes(failed_codes);
            responseCommon.set_latency_in_us(10); // TODO
            resp_.set_result(responseCommon);
            promise_.setValue(std::move(resp_));
            return f;
        }

        // folly::Future<cpp2::ExecResponse>
        // StorageServiceHandler::future_addVertices(const cpp2::AddVerticesRequest &req)
        // {

        //     // TODO:调整构造skvrecord和开始事务的顺序 :将request_list改成skvrecord_list, endtxn失败时，多尝试几次。
        //     //用于返回结果
        //     //暂时不处理latency，leader，partID字段

        //     //sleep 10s ,验证是否可以写入
        //     //sleep(1);

        //     folly::Promise<cpp2::ExecResponse> promise_;
        //     auto f = promise_.getFuture();
        //     cpp2::ExecResponse resp_;
        //     cpp2::ResponseCommon responseCommon;
        //     std::vector<cpp2::ResultCode> failed_codes;

        //     //在内部做一个从tagid到schema的映射，以防多个相同tag的结点都从数据库中请求相同的schema
        //     // std::unordered_map<int32_t, std::shared_ptr<k2::dto::Schema>> table;
        //     std::vector<k2graph::MyWriteRequest> request_list;
        //     std::vector<k2::dto::SKVRecord> skvrecord_list;

        //     auto spaceId = req.space_id;

        //     //构造skvrecord
        //     for (auto iter = req.parts.begin(); iter != req.parts.end(); iter++)
        //     {
        //         auto vertices = iter->second;
        //         int64_t batch_vertex_size = vertices.size();
        //         std::cout<<"batch_vertex_size is "<<batch_vertex_size<<"\n";
        //         total_vertex += batch_vertex_size;
        //         std::cout<<"total_vertex is "<<total_vertex<<"\n";
        //         for (auto vertex : vertices)
        //         {

        //          auto tags = vertex.tags;
        //             //每个tag构造一个add请求
        //             for (auto tag : tags)
        //             {
        //                 //先获取schema
        //                 std::shared_ptr<k2::dto::Schema> schema;
        //                 auto tagID = tag.tag_id;
        //                 if (SchemaTable.find(tagID) != SchemaTable.end())
        //                 {
        //                     schema = SchemaTable[tagID];
        //                 }
        //                 else
        //                 { //没找到,需要获取schema，之后可以单独修改成一个函数
        //                     try
        //                     {schema = GetSchemaFromK2(spaceId,tagID);}

        //                    // k2graph::MySchemaGetRequest request{
        //                     //    .collectionName = std::to_string(spaceId),
        //                      //   .schemaName = std::to_string(tagID),
        //                      //   .schemaVersion = 1, //目前所有schema的version均为1，之后可能需要进一步修改
        //                      //   .prom = new std::promise<k2::GetSchemaResult>()};
        //                     //pushQ(k2graph::SchemaGetQ, request);
        //                     //try
        //                    // {
        //                      //   auto result = request.prom->get_future();
        //                      //   auto schemaResult = result.get();
        //                       //  auto status = schemaResult.status;
        //                       //  if (!status.is2xxOK())
        //                       //  { //获取schema时出错
        //                             //构造错误信息,并返回
        //                             // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
        //                             //std::cout << "获取schema时出错\n";

        //                         //    cpp2::ResultCode rescode;
        //                         //    rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
        //                         //    failed_codes.emplace_back(rescode);
        //                         //    responseCommon.set_failed_codes(failed_codes);
        //                          //   resp_.set_result(responseCommon);
        //                             // return resp_;
        //                          //   promise_.setValue(std::move(resp_));
        //                            // return f;
        //                        // }
        //                        // schema = schemaResult.schema;
        //                        // SchemaTable[tagID] = schemaResult.schema;
        //                     //}
        //                    catch (...)
        //                      {
        //                         //构造错误信息,并返回
        //                         //因为遇到异常，直接返回
        //                         cpp2::ResultCode rescode;
        //                         rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
        //                         failed_codes.emplace_back(rescode);
        //                         responseCommon.set_failed_codes(failed_codes);
        //                         resp_.set_result(responseCommon);
        //                         return resp_;
        //                     }

        //                 }

        //                 //Nebula Schema用于解码
        //                 auto fields = schema->fields;
        //                 nebula::cpp2::Schema k2TagSchema;
        //                  if (NebulaSchemaTable.find(tagID) != NebulaSchemaTable.end())
        //                 {
        //                     k2TagSchema = NebulaSchemaTable[tagID];
        //                 }

        //                 else{

        //                // std::cout << fields;
        //                     nebula::SchemaWriter schemaWriter; //用于构造nebula中的schema
        //                     for (auto field = fields.begin(); field != fields.end(); field++)
        //                     {
        //                         if (field == fields.begin())
        //                         {
        //                             field++;
        //                             field++;
        //                             continue;
        //                         }
        //                         auto name = field->name.c_str();
        //                         auto type = field->type;

        //                         switch (type)
        //                         { // VID类型对应INT64
        //                         case k2::dto::FieldType::STRING:
        //                             schemaWriter.appendCol(name, nebula::cpp2::SupportedType::STRING);
        //                             break;
        //                         case k2::dto::FieldType::INT16T:
        //                         case k2::dto::FieldType::INT32T:
        //                         case k2::dto::FieldType::INT64T:
        //                             schemaWriter.appendCol(name, nebula::cpp2::SupportedType::INT);
        //                             break;
        //                         case k2::dto::FieldType::FLOAT:
        //                             schemaWriter.appendCol(name, nebula::cpp2::SupportedType::FLOAT);
        //                             break;
        //                         case k2::dto::FieldType::DOUBLE:
        //                             schemaWriter.appendCol(name, nebula::cpp2::SupportedType::DOUBLE);
        //                             break;
        //                         case k2::dto::FieldType::BOOL:
        //                             schemaWriter.appendCol(name, nebula::cpp2::SupportedType::BOOL);
        //                             break;
        //                         default:
        //                             //构造错误信息,并返回
        //                             // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
        //                             cpp2::ResultCode rescode;
        //                             rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
        //                             failed_codes.emplace_back(rescode);
        //                             responseCommon.set_failed_codes(failed_codes);
        //                             resp_.set_result(responseCommon);
        //                             return resp_;
        //                         }
        //                     }
        //                     k2TagSchema = schemaWriter.moveSchema();
        //                     NebulaSchemaTable[tagID] = k2TagSchema;
        //                 }

        //                 //auto nebula_schema = std::make_shared<nebula::ResultSchemaProvider>(schemaWriter.moveSchema());
        //                 auto nebula_schema = std::make_shared<nebula::ResultSchemaProvider>(k2TagSchema);
        //                 nebula::dataman::NebulaCodecImpl codec;
        //                 auto result = codec.decode(tag.get_props(), nebula_schema);
        //                // std::cout << "\n\n\n\nL198\n\n\n\n";
        //                 //构造skvrecord
        //                 k2::dto::SKVRecord skvRecord(std::to_string(spaceId), schema);
        //                 //已经将meta中前三个field全部设置成INT64
        //                 skvRecord.serializeNext<int16_t>(iter->first);
        //                 skvRecord.serializeNext<int64_t>(vertex.get_id());
        //                 skvRecord.serializeNext<int32_t>(tag.get_tag_id());
        //                // std::cout << "\n\n\n\nL206\n\n\n\n"
        //                 //          << result.value().begin()->first << "\n"
        //                 //          << (result.value().begin()->second.type() == typeid(int32_t)) << "\n";
        //                 for (auto field = fields.begin(); field != fields.end(); field++)
        //                 {
        //                     if (field == fields.begin())
        //                     {
        //                         field++;
        //                         field++;
        //                         continue;
        //                     }
        //                    // std::cout << "\n\n\n\nL213\n\n\n\n"
        //                     //          << field->name.c_str() << "\n";
        //                     auto name = field->name.c_str(); //获得name的std::string格式
        //                     auto type = field->type;
        //                     //在result中查找是否存在此field，不存在则查看是否可以置为null
        //                     auto value = result.value().find(name);
        //                     try
        //                     {
        //                         if (value != result.value().end())
        //                         { //查找到name，直接根据类型序列化值
        //                             switch (type)
        //                             {
        //                             case k2::dto::FieldType::STRING:
        //                                 skvRecord.serializeNext<k2::String>(boost::any_cast<std::string>(value->second));
        //                                 break;
        //                             case k2::dto::FieldType::INT16T:
        //                                 skvRecord.serializeNext<int16_t>(boost::any_cast<int16_t>(value->second));
        //                                 break;
        //                             case k2::dto::FieldType::INT32T:
        //                                 skvRecord.serializeNext<int32_t>(boost::any_cast<int32_t>(value->second));
        //                                 break;
        //                             case k2::dto::FieldType::INT64T:
        //                                 skvRecord.serializeNext<int64_t>(boost::any_cast<int32_t>(value->second));
        //                                 break;
        //                             case k2::dto::FieldType::FLOAT:
        //                                 skvRecord.serializeNext<float>(boost::any_cast<float>(value->second));
        //                                 break;
        //                             case k2::dto::FieldType::DOUBLE:
        //                                 skvRecord.serializeNext<double>(boost::any_cast<double>(value->second));
        //                                 break;
        //                             case k2::dto::FieldType::BOOL:
        //                                 skvRecord.serializeNext<bool>(boost::any_cast<bool>(value->second));
        //                                 break;
        //                             default:
        //                                 //构造错误信息,并返回
        //                                 // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
        //                                 cpp2::ResultCode rescode;
        //                                 rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
        //                                 failed_codes.emplace_back(rescode);
        //                                 responseCommon.set_failed_codes(failed_codes);
        //                                 resp_.set_result(responseCommon);
        //                                 return resp_;
        //                             }
        //                         }
        //                         else
        //                         {
        //                             //未找到值序列化null
        //                             skvRecord.serializeNull();
        //                         }
        //                     }
        //                     catch (...)
        //                     {
        //                         //构造错误信息,并返回
        //                         cpp2::ResultCode rescode;
        //                         rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
        //                         failed_codes.emplace_back(rescode);
        //                         responseCommon.set_failed_codes(failed_codes);
        //                         resp_.set_result(responseCommon);
        //                         return resp_;
        //                     }
        //                 }

        //                 skvrecord_list.push_back(std::move(skvRecord));
        //             }
        //         }
        //     }

        //     //std::cout << "begin Txn\n";

        //     //开始一个事务
        //     k2::K2TxnOptions options{};
        //     options.syncFinalize = true;
        //     k2graph::MyBeginTxnRequest qr{.opts = options, .prom = new std::promise<k2::dto::K23SI_MTR>(), .startTime = k2::Clock::now()};
        //     pushQ(k2graph::BeginTxnQ, std::move(qr));
        //     k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
        //     try
        //     {
        //         auto result = qr.prom->get_future();
        //         mtr = result.get();
        //     }
        //     catch (...)
        //     {
        //         //构造错误信息,并返回
        //         cpp2::ResultCode rescode;
        //         rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
        //         failed_codes.emplace_back(rescode);
        //         responseCommon.set_failed_codes(failed_codes);
        //         resp_.set_result(responseCommon);
        //         return resp_;
        //     }
        //     //将所有req push进队列
        //    // std::cout << "push queue\n";

        //     for (auto iter = skvrecord_list.begin(); iter != skvrecord_list.end(); iter++)
        //     {
        //         k2graph::MyWriteRequest write_request{
        //             .mtr = mtr,
        //             .record = std::move(*(iter)), //从上面的序列化得出
        //             .prom = new std::promise<k2::WriteResult>()};
        //         pushQ(k2graph::WriteRequestQ, std::move(write_request));
        //         request_list.push_back(std::move(write_request));
        //     }
        //    // std::cout << "L292\n";
        //     bool isSucceed = true;
        //     for (auto request = request_list.begin(); request != request_list.end(); request++)
        //     {
        //         try
        //         {
        //             auto result = request->prom->get_future();
        //             auto WriteResult = result.get();
        //             auto status = WriteResult.status;
        //             if (!status.is2xxOK())
        //             {
        //                 //std::cout << "L301\n";
        //                 isSucceed = false;
        //                 break;
        //             }
        //            // std::cout << "\nL304\n"
        //                       //<< status << "\n";
        //         }
        //         catch (...)
        //         {
        //             isSucceed = false;
        //             break;
        //         }
        //     }
        //     k2graph::MyEndTxnRequest end_txn_req{
        //         .mtr = mtr,
        //         .shouldCommit = isSucceed,
        //         .prom = new std::promise<k2::EndResult>()};

        //     pushQ(k2graph::EndTxnQ, end_txn_req);
        //     int limit = 2; //设置尝试endtxn的最大次数
        //     bool isendsucceed;
        //    // std::cout << "L319\n"
        //            //   << isSucceed;
        //     do
        //     {
        //         try
        //         {
        //           //  std::cout << "L320\n";
        //             auto result = end_txn_req.prom->get_future();
        //             auto EndResult = result.get();
        //             auto status = EndResult.status;
        //             if (!status.is2xxOK())
        //             { //没有成功,重新push
        //                 isendsucceed = false;
        //                 pushQ(k2graph::EndTxnQ, end_txn_req);
        //             }
        //             else
        //             {
        //                 // end事务成功，直接返回上面得到的写入的结果,即写入成功，返回成功，否则失败。
        //                 isendsucceed = true;
        //             }
        //         }
        //         catch (...)
        //         {
        //             isendsucceed = false;
        //             pushQ(k2graph::EndTxnQ, end_txn_req);
        //         }
        //         limit--;
        //     } while ((limit > 0) && !isendsucceed); //还有剩余尝试次数，且失败时继续执行
        //    // std::cout << "L342\n"
        //              // << isendsucceed << "\n";
        //     cpp2::ResultCode rescode;
        //     if (!isendsucceed || !isSucceed)
        //     {
        //        // std::cout << "L345\n\n\n\n";
        //         rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
        //         failed_codes.emplace_back(rescode);
        //     }
        //     else
        //     {
        //        // std::cout << "L349\n\n\n\n"
        //          //         << "success"
        //           //        << "\n\n\n\n";
        //     }
        //     responseCommon.set_failed_codes(failed_codes);
        //     responseCommon.set_latency_in_us(10); // TODO
        //     resp_.set_result(responseCommon);
        //     //返回一个future
        //     promise_.setValue(std::move(resp_));
        //     //std::cout << "addvertex end\n";
        //     return f;
        // }

        folly::Future<cpp2::ExecResponse>
        StorageServiceHandler::future_addEdges(const cpp2::AddEdgesRequest &req)
        {
            // std::cout << "future_addEdges called \n";

            folly::Promise<cpp2::ExecResponse> promise_;
            auto f = promise_.getFuture();
            cpp2::ExecResponse resp_;
            cpp2::ResponseCommon responseCommon;
            std::vector<cpp2::ResultCode> failed_codes;

            // edgeType(int32) <----> schema(k2) 映射表
            // std::unordered_map<int32_t, std::shared_ptr<k2::dto::Schema>> EdgeTypeTable;
            std::vector<k2graph::MyWriteRequest> request_list;
            std::vector<k2::dto::SKVRecord> skvrecord_list;

            auto spaceID = req.space_id;

            for (auto iter = req.parts.begin(); iter != req.parts.end(); iter++)
            {
                auto edges = iter->second;
                for (auto edge : edges)
                {
                    // int32_t edgeType = edge.key.edge_type;
                    int32_t edgeType = edge.get_key().get_edge_type();
                    std::shared_ptr<k2::dto::Schema> schema;
                    //  std::cout << "edgeType ID is:" << edgeType << "\n";
                    if (edgeType < 0)
                    {
                        continue; //不保存负向的边
                    }

                    if (SchemaTable.find(edgeType) != SchemaTable.end())
                    {
                        schema = SchemaTable[edgeType];
                    }
                    else
                    {
                        //去 k2 获取 对应的 schema
                        k2graph::MySchemaGetRequest request{
                            .collectionName = std::to_string(spaceID),
                            .schemaName = std::to_string(edgeType),
                            .schemaVersion = 1,
                            // .prom = new std::promise<k2::GetSchemaResult>()};
                            .prom = {}};

                        auto result = request.prom.get_future();

                        pushQ(k2graph::SchemaGetQ, std::move(request));
                        try
                        {
                            // auto result = request.prom->get_future();
                            // auto result = request.prom.get_future();
                            // std::future<k2::GetSchemaResult> result = request.prom.get_future();
                            auto schemaResult = result.get();
                            auto status = schemaResult.status;
                            if (!status.is2xxOK())
                            { //获取schema时出错
                                //构造错误信息,并返回
                                // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                                std::cout << "code is: " << status.code << std::endl;
                                std::cout << "获取schema时出错\n";

                                cpp2::ResultCode rescode;
                                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                failed_codes.emplace_back(rescode);
                                responseCommon.set_failed_codes(failed_codes);
                                resp_.set_result(responseCommon);
                                // return resp_;
                                promise_.setValue(std::move(resp_));
                                return f;
                            }
                            schema = schemaResult.schema;
                            SchemaTable[edgeType] = schemaResult.schema;
                        }
                        catch (...)
                        {
                            //构造错误信息,并返回
                            //因为遇到异常，直接返回
                            cpp2::ResultCode rescode;
                            rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                            failed_codes.emplace_back(rescode);
                            responseCommon.set_failed_codes(failed_codes);
                            resp_.set_result(responseCommon);
                            return resp_;
                        }
                    }

                    auto fields = schema->fields;
                    nebula::SchemaWriter schemaWriter;
                    //跳过前面的固定字段 PartID-VertexID-EdgeType-Rank-VertexID
                    for (auto field = fields.begin() + 5; field != fields.end(); field++)
                    {
                        auto name = field->name.c_str();
                        auto type = field->type;
                        switch (type)
                        { // VID类型对应INT64
                        case k2::dto::FieldType::STRING:
                            schemaWriter.appendCol(name, nebula::cpp2::SupportedType::STRING);
                            break;
                        case k2::dto::FieldType::INT16T:
                        case k2::dto::FieldType::INT32T:
                        case k2::dto::FieldType::INT64T:
                            schemaWriter.appendCol(name, nebula::cpp2::SupportedType::INT);
                            break;
                        case k2::dto::FieldType::FLOAT:
                            schemaWriter.appendCol(name, nebula::cpp2::SupportedType::FLOAT);
                            break;
                        case k2::dto::FieldType::DOUBLE:
                            schemaWriter.appendCol(name, nebula::cpp2::SupportedType::DOUBLE);
                            break;
                        case k2::dto::FieldType::BOOL:
                            schemaWriter.appendCol(name, nebula::cpp2::SupportedType::BOOL);
                            break;
                        default:
                            //构造错误信息,并返回
                            // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                            cpp2::ResultCode rescode;
                            rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                            failed_codes.emplace_back(rescode);
                            responseCommon.set_failed_codes(failed_codes);
                            resp_.set_result(responseCommon);
                            return resp_;
                        }
                    }
                    auto nebula_schema = std::make_shared<nebula::ResultSchemaProvider>(schemaWriter.moveSchema());
                    nebula::dataman::NebulaCodecImpl codec;
                    auto result = codec.decode(edge.get_props(), nebula_schema);
                    k2::dto::SKVRecord skvRecord(std::to_string(spaceID), schema);

                    skvRecord.serializeNext<int16_t>(iter->first);
                    skvRecord.serializeNext<int64_t>(edge.get_key().get_src());
                    skvRecord.serializeNext<int32_t>(edgeType);
                    skvRecord.serializeNext<int64_t>(edge.get_key().get_ranking());
                    skvRecord.serializeNext<int64_t>(edge.get_key().get_dst());

                    for (auto field = fields.begin() + 5; field != fields.end(); field++)
                    {
                        auto name = field->name.c_str(); //获得name的std::string格式
                        auto type = field->type;
                        auto value = result.value().find(name);

                        try
                        {
                            if (value != result.value().end())
                            { //查找到name，直接根据类型序列化值
                                switch (type)
                                {
                                case k2::dto::FieldType::STRING:
                                    skvRecord.serializeNext<k2::String>(boost::any_cast<std::string>(value->second));
                                    break;
                                case k2::dto::FieldType::INT16T:
                                    skvRecord.serializeNext<int16_t>(boost::any_cast<int16_t>(value->second));
                                    break;
                                case k2::dto::FieldType::INT32T:
                                    skvRecord.serializeNext<int32_t>(boost::any_cast<int32_t>(value->second));
                                    break;
                                case k2::dto::FieldType::INT64T:
                                    skvRecord.serializeNext<int64_t>(boost::any_cast<int32_t>(value->second));
                                    break;
                                case k2::dto::FieldType::FLOAT:
                                    skvRecord.serializeNext<float>(boost::any_cast<float>(value->second));
                                    break;
                                case k2::dto::FieldType::DOUBLE:
                                    skvRecord.serializeNext<double>(boost::any_cast<double>(value->second));
                                    break;
                                case k2::dto::FieldType::BOOL:
                                    skvRecord.serializeNext<bool>(boost::any_cast<bool>(value->second));
                                    break;
                                default:
                                    //构造错误信息,并返回
                                    // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                                    cpp2::ResultCode rescode;
                                    rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                    failed_codes.emplace_back(rescode);
                                    responseCommon.set_failed_codes(failed_codes);
                                    resp_.set_result(responseCommon);
                                    return resp_;
                                }
                            }
                            else
                            {
                                //未找到值序列化null
                                skvRecord.serializeNull();
                            }
                        }
                        catch (...)
                        {
                            //构造错误信息,并返回
                            cpp2::ResultCode rescode;
                            rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                            failed_codes.emplace_back(rescode);
                            responseCommon.set_failed_codes(failed_codes);
                            resp_.set_result(responseCommon);
                            return resp_;
                        }
                    }
                    skvrecord_list.push_back(std::move(skvRecord));
                }
            } // end of Construct skvrecord

            // std::cout << "begin Txn\n";

            k2::K2TxnOptions options{};
            options.syncFinalize = true;
            k2graph::MyBeginTxnRequest qr{.opts = options,
                                          //.prom = new std::promise<k2::dto::K23SI_MTR>(),
                                          .prom = std::promise<k2::dto::K23SI_MTR>(),
                                          .startTime = k2::Clock::now()};
            pushQ(k2graph::BeginTxnQ, std::move(qr));
            k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
            try
            {
                // auto result = qr.prom->get_future();
                auto result = qr.prom.get_future();
                mtr = result.get();
            }
            catch (...)
            {
                //构造错误信息,并返回
                cpp2::ResultCode rescode;
                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                failed_codes.emplace_back(rescode);
                responseCommon.set_failed_codes(failed_codes);
                resp_.set_result(responseCommon);
                return resp_;
            }

            //将所有req push进队列
            //  std::cout << "push queue\n";

            for (auto iter = skvrecord_list.begin(); iter != skvrecord_list.end(); iter++)
            {
                k2graph::MyWriteRequest write_request{
                    .mtr = mtr,
                    .record = std::move(*(iter)), //从上面的序列化得出
                    //.prom = new std::promise<k2::WriteResult>()};
                    .prom = std::promise<k2::WriteResult>()};
                pushQ(k2graph::WriteRequestQ, std::move(write_request));
                request_list.push_back(std::move(write_request));
            }
            bool isSucceed = true;

            for (auto request = request_list.begin(); request != request_list.end(); request++)
            {
                try
                {
                    // auto result = request->prom->get_future();
                    auto result = request->prom.get_future();
                    auto WriteResult = result.get();
                    auto status = WriteResult.status;
                    if (!status.is2xxOK())
                    {
                        // std::cout << "L301\n";
                        isSucceed = false;
                        break;
                    }
                    // std::cout << "\nL304\n"
                    //     << status << "\n";
                }
                catch (...)
                {
                    isSucceed = false;
                    break;
                }
            }

            k2graph::MyEndTxnRequest end_txn_req{
                .mtr = mtr,
                .shouldCommit = isSucceed,
                //.prom = new std::promise<k2::EndResult>()};
                .prom = std::promise<k2::EndResult>()};
            // std::cout << "\nL304\n"
            //  << end_txn_req.mtr << "\n"
            //<< isSucceed << "\n";
            pushQ(k2graph::EndTxnQ, std::move(end_txn_req));
            int limit = 3; //设置尝试endtxn的最大次数
            bool isendsucceed;

            do
            {
                try
                {
                    // std::cout << "L320\n";
                    // auto result = end_txn_req.prom->get_future();
                    auto result = end_txn_req.prom.get_future();
                    auto EndResult = result.get();
                    auto status = EndResult.status;
                    if (!status.is2xxOK())
                    { //没有成功,重新push
                        isendsucceed = false;
                        pushQ(k2graph::EndTxnQ, std::move(end_txn_req));
                    }
                    else
                    {
                        // end事务成功，直接返回上面得到的写入的结果,即写入成功，返回成功，否则失败。
                        isendsucceed = true;
                    }
                }
                catch (...)
                {
                    isendsucceed = false;
                    pushQ(k2graph::EndTxnQ, std::move(end_txn_req));
                }
                limit--;
            } while ((limit > 0) && !isendsucceed); //还有剩余尝试次数，且失败时继续执行

            cpp2::ResultCode rescode;
            if (!isendsucceed || !isSucceed)
            {
                // std::cout << "L345\n\n\n\n";
                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                failed_codes.emplace_back(rescode);
            }
            else
            {
                // std::cout << "L349\n\n\n\n"
                //          << "success"
                //         << "\n\n\n\n";
            }

            responseCommon.set_failed_codes(failed_codes);
            responseCommon.set_latency_in_us(10); // TODO
            resp_.set_result(responseCommon);
            //返回一个future
            promise_.setValue(std::move(resp_));
            // std::cout << "addedge end\n";
            return f;
        }
        /*
                folly::Future<cpp2::ExecResponse>
                StorageServiceHandler::future_deleteVertices(const cpp2::DeleteVerticesRequest& req) {
                    auto* processor = DeleteVerticesProcessor::instance(kvstore_,
                                                                        schemaMan_,
                                                                        indexMan_,
                                                                        &delVertexQpsStat_,
                                                                        &vertexCache_);
                    RETURN_FUTURE(processor);
                }

                folly::Future<cpp2::ExecResponse>
                StorageServiceHandler::future_deleteEdges(const cpp2::DeleteEdgesRequest& req) {
                    auto* processor = DeleteEdgesProcessor::instance(kvstore_, schemaMan_, indexMan_);
                    RETURN_FUTURE(processor);
                }
*/
        // folly::Future<cpp2::UpdateResponse>
        // StorageServiceHandler::future_updateVertex(const cpp2::UpdateVertexRequest& req) {
        // auto* processor = UpdateVertexProcessor::instance(kvstore_,
        //                                                   schemaMan_,
        //                                                   indexMan_,
        //                                                   &updateVertexQpsStat_,
        //                                                   &vertexCache_);
        // RETURN_FUTURE(processor);
        //先根据
        // }
        /*
                        folly::Future<cpp2::UpdateResponse>
                        StorageServiceHandler::future_updateEdge(const cpp2::UpdateEdgeRequest& req) {
                            auto* processor = UpdateEdgeProcessor::instance(kvstore_,
                                                                            schemaMan_,
                                                                            indexMan_,
                                                                            &updateEdgeQpsStat_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::ScanEdgeResponse>
                        StorageServiceHandler::future_scanEdge(const cpp2::ScanEdgeRequest& req) {
                            auto* processor = ScanEdgeProcessor::instance(kvstore_, schemaMan_, &scanEdgeQpsStat_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::ScanVertexResponse>
                        StorageServiceHandler::future_scanVertex(const cpp2::ScanVertexRequest& req) {
                            auto* processor = ScanVertexProcessor::instance(kvstore_, schemaMan_, &scanVertexQpsStat_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_transLeader(const cpp2::TransLeaderReq& req) {
                            auto* processor = TransLeaderProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_addPart(const cpp2::AddPartReq& req) {
                            auto* processor = AddPartProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_addLearner(const cpp2::AddLearnerReq& req) {
                            auto* processor = AddLearnerProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_waitingForCatchUpData(const cpp2::CatchUpDataReq& req) {
                            auto* processor = WaitingForCatchUpDataProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_removePart(const cpp2::RemovePartReq& req) {
                            auto* processor = RemovePartProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_memberChange(const cpp2::MemberChangeReq& req) {
                            auto* processor = MemberChangeProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_checkPeers(const cpp2::CheckPeersReq& req) {
                            auto* processor = CheckPeersProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::GetLeaderResp>
                        StorageServiceHandler::future_getLeaderPart(const cpp2::GetLeaderReq& req) {
                            auto* processor = GetLeaderProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::ExecResponse>
                        StorageServiceHandler::future_put(const cpp2::PutRequest& req) {
                            auto* processor = PutProcessor::instance(kvstore_, schemaMan_, &putKvQpsStat_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::GeneralResponse>
                        StorageServiceHandler::future_get(const cpp2::GetRequest& req) {
                            auto* processor = GetProcessor::instance(kvstore_, schemaMan_, &getKvQpsStat_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::GetUUIDResp>
                        StorageServiceHandler::future_getUUID(const cpp2::GetUUIDReq& req) {
                            auto* processor = GetUUIDProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_createCheckpoint(const cpp2::CreateCPRequest& req) {
                            auto* processor = CreateCheckpointProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_dropCheckpoint(const cpp2::DropCPRequest& req) {
                            auto* processor = DropCheckpointProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_blockingWrites(const cpp2::BlockingSignRequest& req) {
                            auto* processor = SendBlockSignProcessor::instance(kvstore_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_rebuildTagIndex(const cpp2::RebuildIndexRequest& req) {
                            auto* processor = RebuildTagIndexProcessor::instance(kvstore_,
                                                                                 schemaMan_,
                                                                                 indexMan_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::AdminExecResp>
                        StorageServiceHandler::future_rebuildEdgeIndex(const cpp2::RebuildIndexRequest& req) {
                            auto* processor = RebuildEdgeIndexProcessor::instance(kvstore_,
                                                                                  schemaMan_,
                                                                                  indexMan_);
                            RETURN_FUTURE(processor);
                        }

                        folly::Future<cpp2::LookUpIndexResp>
                        StorageServiceHandler::future_lookUpIndex(const cpp2::LookUpIndexRequest& req) {
                            auto* processor = LookUpIndexProcessor::instance(kvstore_,
                                                                             schemaMan_,
                                                                             indexMan_,
                                                                             &lookupVerticesQpsStat_,
                                                                             &vertexCache_);
                            RETURN_FUTURE(processor);
                        }
                        */
    } // namespace storage
} // namespace nebula
