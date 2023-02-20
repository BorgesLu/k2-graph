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
#include "common/filter/Expressions.h"
// String to int
#include <stdlib.h>
#include <map>
#include <set>
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
    auto result = request.prom.get_future();
    pushQ(k2graph::SchemaGetQ, std::move(request));
    try
    {
        // auto result = request.prom->get_future();
        //  auto result = request.prom.get_future();
        // sleep(1);
        auto schemaResult = result.get();
        auto status = schemaResult.status;
        if (!status.is2xxOK())
        {
            std::cout << "l105 获取schema时出错\n";
            std::cout << status<<"\n\n";
        }
        std::shared_ptr<k2::dto::Schema> schema = schemaResult.schema;
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
            cpp2::PropDef first_col = req.get_return_columns().front();
            auto col_name = first_col.get_name();
            auto col_edgeID = first_col.get_id().get_edge_type();

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
                                          .prom = {},
                                          .startTime = k2::Clock::now()};
            auto BeginTxnQResult = qr.prom.get_future();
            pushQ(k2graph::BeginTxnQ, std::move(qr));
            k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
            try
            {
                mtr = BeginTxnQResult.get().GetMtr();
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
                        auto ScanReadResult = request.prom.get_future();
                        pushQ(k2graph::scanReadCreateTxQ, std::move(request));
                        try
                        {
                            // auto scan_create_result = request.prom->get_future().get();
                            auto scan_create_result = ScanReadResult.get();
                            auto status = scan_create_result.status;
                            if (!status.is2xxOK())
                            {
                                // std::cout << "l263 createQuery 错误"
                                //           << "\n";
                                // cpp2::ResultCode rescode;
                                // rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                // failed_codes.emplace_back(rescode);
                                // responseCommon.set_failed_codes(failed_codes);
                                // resp_.set_result(responseCommon);
                                // // return resp_;
                                // promise_.setValue(std::move(resp_));
                                // return f;
                                resp_.set_total_edges(0);
                                resp_.set_vertices(std::move(scan_vertices));
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
                        
                        scan->setLimit(-1);

                        // range query
                        k2graph::MyScanReadRequest scan_request{
                            .mtr = mtr,
                            .query = std::move(scan),
                            // .prom = new std::promise<k2::QueryResult>()};
                            .prom = std::promise<k2::QueryResult>()};
                        auto ReadResult = scan_request.prom.get_future();
                        pushQ(k2graph::scanReadTxQ, std::move(scan_request));

                        //  k2::QueryResult scan_result = scan_request.prom->get_future().get();
                        k2::QueryResult scan_result = ReadResult.get();
                        auto status = scan_result.status;
                        std::vector<k2::dto::SKVRecord> scan_result_records(std::move(scan_result.records));
                        // std::cout << "L255 读上来的数据个数 " << scan_result_records.size() << "\n";
                        edge_number = scan_result_records.size();
                        // std::vector<std::string> edge_vector;

                        // bool isSucceed = true;
                        
                       // k2graph::MyEndTxnRequest end_txn_req{
                      //      .mtr = mtr,
                      //      .shouldCommit = isSucceed,
                      //      // .prom = new std::promise<k2::EndResult>()};
                      //      .prom = std::promise<k2::EndResult>()};

                     //   pushQ(k2graph::EndTxnQ, std::move(end_txn_req));

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
            auto EndTxnQResult = end_txn_req.prom.get_future();
            pushQ(k2graph::EndTxnQ, std::move(end_txn_req));

            // resp_.set_edge_schema(std::move(my_edge_schema));
            // std::cout << "L313 get_edge_schema()->size()" << resp_.get_edge_schema()->size() << "\n\n";
            // std::cout<<"scan_vertices is "<<scan_vertices.size()<<"\n\n";
            // int32_t edge_number = 1;
            resp_.set_total_edges(edge_number);
            resp_.set_vertices(std::move(scan_vertices));
            promise_.setValue(std::move(resp_));
            return f;

        } // end for getBound

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

            //暂时只处理一次读一个顶点，colSize大小为1
            cpp2::PropDef first_col = req.get_return_columns().front();
            auto col_name = first_col.get_name();
            auto col_tagID = first_col.get_id().get_tag_id();

            //构建编码的schema
            std::shared_ptr<k2::dto::Schema> schema;
            schema = SchemaTable[col_tagID];

            auto fields = schema->fields;
            nebula::SchemaWriter schemaWriter;

            for (auto field = fields.begin()+3; field != fields.end(); field++)
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
            nebula::cpp2::Schema tag_schema = schemaWriter.moveSchema();
            auto nebula_schema = std::make_shared<nebula::ResultSchemaProvider>(tag_schema);
            nebula::dataman::NebulaCodecImpl codec;
            //构建返回的 vertex_schema
            std::unordered_map<nebula::cpp2::TagID, nebula::cpp2::Schema> my_vertex_schema;
            my_vertex_schema.insert(std::make_pair(col_tagID, tag_schema));
            

            //开始一个事务 TODO:单独设置为一个函数
            k2::K2TxnOptions options{};
            options.syncFinalize = true;
            k2graph::MyBeginTxnRequest qr{.opts = options,
                                          //.prom = new std::promise<k2::dto::K23SI_MTR>(),
                                          .prom = {},
                                          .startTime = k2::Clock::now()};

            auto GetPropResult = qr.prom.get_future();
            pushQ(k2graph::BeginTxnQ, std::move(qr));
            k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
            try
            {
                // auto result = qr.prom->get_future();
                // auto GetPropResult = qr.prom.get_future();
                // mtr = result.get();
                mtr = GetPropResult.get().GetMtr();
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
                  auto SchemaGetResult = request.prom.get_future();
                    pushQ(k2graph::SchemaGetQ, std::move(request));
                    try
                    {
                        // auto result = request.prom->get_future();
                       // auto SchemaGetResult = request.prom.get_future();
                        auto schemaResult = SchemaGetResult.get();
                        auto status = schemaResult.status;
                        if (!status.is2xxOK())
                        { //获取schema时出错
                            //构造错误信息,并返回
                            // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                            std::cout << "l559获取schema时出错\n";

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

            //只能一次读一条数据
            auto iter = skvrecord_list.begin();
            k2::dto::SKVRecord _record = std::move(*(iter));
                            k2::dto::Key _key = _record.getKey();
                            k2graph::MyReadRequest read_request{
                                .mtr = mtr,
                                .record = k2::dto::SKVRecord(),
                                .key = std::move(_key),
                                .collectionName = std::to_string(spaceId),
                                //  .prom = new std::promise<k2::ReadResult<k2::dto::SKVRecord>>()};
                                .prom = std::promise<k2::ReadResult<k2::dto::SKVRecord>>()};

                            auto ReadResult = read_request.prom.get_future();
            pushQ(k2graph::readTxQ, std::move(read_request));
            bool isSucceed = true;
            std::vector<cpp2::VertexData> vertices;
             auto MyReadResult = ReadResult.get();
                auto status = MyReadResult.status;
                if (!status.is2xxOK())
                {
                    isSucceed = false;
                   // break;
                }

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
                    //只支持 int_64的字段
                    auto skv_filed = MyReadResult.value.deserializeField<int64_t>(name);
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


 /*           for (auto iter = skvrecord_list.begin(); iter != skvrecord_list.end(); iter++)
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

                auto ReadResult = read_request.prom.get_future();
                pushQ(k2graph::readTxQ, std::move(read_request));
                request_list.push_back(std::move(read_request));
            }

            bool isSucceed = true;
            std::vector<cpp2::VertexData> vertices;
            for (auto request = request_list.begin(); request != request_list.end(); request++)
            {
                // auto result = request->prom->get_future();
               // auto ReadResult = request->prom.get_future();
                auto MyReadResult = ReadResult.get();
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
*/
            k2graph::MyEndTxnRequest end_txn_req{
                .mtr = mtr,
                .shouldCommit = isSucceed,
                //  .prom = new std::promise<k2::EndResult>()};
                .prom = std::promise<k2::EndResult>()};

            pushQ(k2graph::EndTxnQ, std::move(end_txn_req));

            resp_.set_vertices(std::move(vertices));
            resp_.set_vertex_schema(std::move(my_vertex_schema));
            promise_.setValue(std::move(resp_));
            return f;
        }

        folly::Future<cpp2::EdgePropResponse>
        StorageServiceHandler::future_getEdgeProps(const cpp2::EdgePropRequest &req)
        {
            folly::Promise<cpp2::EdgePropResponse> promise_;
            auto f = promise_.getFuture();
            cpp2::EdgePropResponse resp_;
            return f;
        }


//add Vertex
        // static int countVertex = 0;
        folly::Future<cpp2::ExecResponse>
        StorageServiceHandler::future_addVertices(const cpp2::AddVerticesRequest &req)
        {
            //只支持一次插入一个顶点的函数
            // std::cout << "part size is: " << req.parts.size() << std::endl;
            // int partNum = 0;
            // for(auto it = req.parts.begin();it!=req.parts.end();++it){
            //     std::cout<<"Part is: "<<partNum<<std::endl;
            //     partNum++;
            //   std::cout << "vertex size " << it->second.size() << std::endl;
            // }
            // countVertex++;
            // if (countVertex % 10 == 0)
            // {
            //     std::cout << countVertex << std::endl; //
            //     // sleep(1); //写得太快了？
            // }
            folly::Promise<cpp2::ExecResponse> promise_;
            auto f = promise_.getFuture();
            cpp2::ExecResponse resp_;
            cpp2::ResponseCommon responseCommon;
            std::vector<cpp2::ResultCode> failed_codes;
            std::vector<k2::dto::SKVRecord> skvrecord_list;
            std::vector<k2::dto::SKVRecord> VID2TagID_list;
            bool isSucceed = true;

            auto spaceId = req.space_id;
            //构造系统schema：VID->TagID
            k2::dto::Schema Vertex2Schema;
            Vertex2Schema.name = std::to_string(1);
            Vertex2Schema.version = 1;
            Vertex2Schema.fields = std::vector<k2::dto::SchemaField>{
                {k2::dto::FieldType::INT64T, "VertexID", false, false},
                {k2::dto::FieldType::INT32T, "TagID", false, false}
            };
            Vertex2Schema.setPartitionKeyFieldsByName(std::vector<k2::String>{"VertexID"});
            Vertex2Schema.setRangeKeyFieldsByName(std::vector<k2::String>{"TagID"});
            for (auto iter = req.parts.begin(); iter != req.parts.end(); iter++)
            {
                auto vertices = iter->second;
                for (auto vertex : vertices)
                {
                    auto tags = vertex.tags;
                    auto VID = vertex.get_id();
                    for (auto tag : tags)
                    {
                        //TODO优化
                        //写入vertexID->tagID的信息
                        k2::dto::SKVRecord VID2TagID(std::to_string(spaceId), std::make_shared<k2::dto::Schema>(Vertex2Schema));
                        VID2TagID.serializeNext<int64_t>(VID);
                        VID2TagID.serializeNext<int32_t>(tag.get_tag_id());
                        VID2TagID_list.push_back(std::move(VID2TagID));

                        std::shared_ptr<k2::dto::Schema> schema;
                        auto tagID = tag.tag_id;
                        if (SchemaTable.find(tagID) != SchemaTable.end())
                        {
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
                        // Nebula Schema用于解码
                        auto fields = schema->fields;
                        nebula::cpp2::Schema k2TagSchema;
                        if (NebulaSchemaTable.find(tagID) != NebulaSchemaTable.end())
                        {
                            k2TagSchema = NebulaSchemaTable[tagID];
                        }
                        else
                        {
                            // std::cout << fields;
                            nebula::SchemaWriter schemaWriter; //用于构造nebula中的schema
                            for (auto field = fields.begin(); field != fields.end(); field++)
                            {
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
                            k2TagSchema = schemaWriter.moveSchema();
                            NebulaSchemaTable[tagID] = k2TagSchema;
                        }
                        auto nebula_schema = std::make_shared<nebula::ResultSchemaProvider>(k2TagSchema);
                        nebula::dataman::NebulaCodecImpl codec;
                        auto result = codec.decode(tag.get_props(), nebula_schema);
                        k2::dto::SKVRecord skvRecord(std::to_string(spaceId), schema);

                        skvRecord.serializeNext<int16_t>(iter->first);
                        skvRecord.serializeNext<int64_t>(VID);
                        skvRecord.serializeNext<int32_t>(tag.get_tag_id());
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
           // usleep(90000);
            // sleep(1);
            
            k2::K2TxnOptions options{};
            options.syncFinalize = true;
            auto start = k2::Clock::now();
            k2graph::MyBeginTxnRequest qr{.opts = options,
                                          .prom = {},
                                          //.startTime = k2::Clock::now()};
                                          .startTime = start};
            
            
            auto BeginTxnQResult = qr.prom.get_future();
            
         //   sleep(1);
            
            pushQ(k2graph::BeginTxnQ, std::move(qr));

            k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
            try
            {
                
                // auto BeginTxnQResult = qr.prom.get_future();
                //mtr = BeginTxnQResult.get();


                mtr = BeginTxnQResult.get().GetMtr();
                // std::cout << "l978 begin Txn success\n";
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

            //
            auto batchIt = req.parts.begin();
            int batchSize = batchIt->second.size();
            // std::cout<<"batchSize is: "<<batchSize<<std::endl;
            for(int i=0;i<batchSize;i++){

            k2graph::MyWriteRequest write_request{
                .mtr = mtr,
                .record = std::move(skvrecord_list[i]), //从上面的序列化得出
                .prom = std::promise<k2::WriteResult>()};
            k2graph::MyWriteRequest VID2TagID_request{
                .mtr = mtr,
                .record = std::move(VID2TagID_list[i]), //从上面的序列化得出
                .prom = std::promise<k2::WriteResult>()
            };
            auto WQResult = write_request.prom.get_future();
            auto WQResult1 = VID2TagID_request.prom.get_future();
            pushQ(k2graph::WriteRequestQ, std::move(write_request));
            pushQ(k2graph::WriteRequestQ, std::move(VID2TagID_request));
            try
            {
                auto WriteResult = WQResult.get();
                auto status = WriteResult.status;
                auto WriteResult1 = WQResult1.get();
                auto status1 = WriteResult1.status;
                std::cout << "l989 " << status1 << std::endl;
                if (!status.is2xxOK())
                {
                    isSucceed = false;
                }
            }
            catch (...)
            {
                isSucceed = false;
            }

            }//end for batch

            bool isendsucceed = true;
            k2graph::MyEndTxnRequest end_txn_req{
                .mtr = mtr,
                .shouldCommit = isSucceed,
                //.prom = new std::promise<k2::EndResult>()
                .prom = std::promise<k2::EndResult>()};

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
        
        folly::Future<cpp2::ExecResponse>
        StorageServiceHandler::future_addEdges(const cpp2::AddEdgesRequest &req)
        {
            // std::cout << "future_addEdges called \n";

            folly::Promise<cpp2::ExecResponse> promise_;
            auto f = promise_.getFuture();
            cpp2::ExecResponse resp_;
            cpp2::ResponseCommon responseCommon;
            std::vector<cpp2::ResultCode> failed_codes;
            bool isSucceed = true;
            std::map<int64_t, std::vector<int32_t>> VID2EdgeType;
            // edgeType(int32) <----> schema(k2) 映射表
            // std::unordered_map<int32_t, std::shared_ptr<k2::dto::Schema>> EdgeTypeTable;
            std::vector<k2::dto::SKVRecord> skvrecord_list;
            std::vector<k2::dto::SKVRecord> in_edge_skvrecord_list;

            auto spaceID = req.space_id;

            for (auto iter = req.parts.begin(); iter != req.parts.end(); iter++)
            {
                auto edges = iter->second;
                for (auto edge : edges)
                {
                    // int32_t edgeType = edge.key.edge_type;
                    int32_t edgeType = edge.get_key().get_edge_type();
                    auto srcID = edge.get_key().get_src();
                    auto dstID = edge.get_key().get_dst();
                    //将VID与EdgeType关联Src->edgeType Dst->-EdgeType
                    if(VID2EdgeType.count(srcID) > 0){
                        VID2EdgeType[srcID].push_back(edgeType);
                    }
                    else {
                        VID2EdgeType[srcID] = {edgeType};
                    }
                    if(VID2EdgeType.count(dstID) > 0){
                        VID2EdgeType[dstID].push_back(-edgeType);
                    }
                    else {
                        VID2EdgeType[dstID] = {-edgeType};
                    }
                    std::shared_ptr<k2::dto::Schema> schema;
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
                        try
                        {
                            schema = GetSchemaFromK2(spaceID, edgeType);
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
                    nebula::cpp2::Schema k2TagSchema;
                    if (NebulaSchemaTable.find(edgeType) != NebulaSchemaTable.end())
                    {
                        k2TagSchema = NebulaSchemaTable[edgeType];
                    }
                    else
                    {
                        nebula::SchemaWriter schemaWriter; //用于构造nebula中的schema
                        for (auto field = fields.begin()+5; field != fields.end(); field++)
                        {
                            //跳过前面的固定字段 PartID-VertexID-EdgeType-Rank-VertexID
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
                        k2TagSchema = schemaWriter.moveSchema();
                        NebulaSchemaTable[edgeType] = k2TagSchema;
                    }
                    auto nebula_schema = std::make_shared<nebula::ResultSchemaProvider>(k2TagSchema);
                    nebula::dataman::NebulaCodecImpl codec;
                    auto result = codec.decode(edge.get_props(), nebula_schema);
                    k2::dto::SKVRecord skvRecord(std::to_string(spaceID), schema);
                    k2::dto::SKVRecord in_edge_skvRecord(std::to_string(spaceID), schema);
                    skvRecord.serializeNext<int16_t>(iter->first);
                    skvRecord.serializeNext<int64_t>(srcID);
                    skvRecord.serializeNext<int32_t>(edgeType);
                    skvRecord.serializeNext<int64_t>(edge.get_key().get_ranking());
                    skvRecord.serializeNext<int64_t>(dstID);
                    
                    in_edge_skvRecord.serializeNext<int16_t>(iter->first);
                    in_edge_skvRecord.serializeNext<int64_t>(dstID);
                    in_edge_skvRecord.serializeNext<int32_t>(-edgeType);
                    in_edge_skvRecord.serializeNext<int64_t>(edge.get_key().get_ranking());
                    in_edge_skvRecord.serializeNext<int64_t>(srcID);


                    for (auto field = fields.begin()+5; field != fields.end(); field++)
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
                                    in_edge_skvRecord.serializeNext<k2::String>(boost::any_cast<std::string>(value->second));
                                    break;
                                case k2::dto::FieldType::INT16T:
                                    skvRecord.serializeNext<int16_t>(boost::any_cast<int16_t>(value->second));
                                    in_edge_skvRecord.serializeNext<int16_t>(boost::any_cast<int16_t>(value->second));
                                    break;
                                case k2::dto::FieldType::INT32T:
                                    skvRecord.serializeNext<int32_t>(boost::any_cast<int32_t>(value->second));
                                    in_edge_skvRecord.serializeNext<int32_t>(boost::any_cast<int32_t>(value->second));
                                    break;
                                case k2::dto::FieldType::INT64T:
                                    skvRecord.serializeNext<int64_t>(boost::any_cast<int32_t>(value->second));
                                    in_edge_skvRecord.serializeNext<int64_t>(boost::any_cast<int32_t>(value->second));
                                    break;
                                case k2::dto::FieldType::FLOAT:
                                    skvRecord.serializeNext<float>(boost::any_cast<float>(value->second));
                                    in_edge_skvRecord.serializeNext<float>(boost::any_cast<float>(value->second));
                                    break;
                                case k2::dto::FieldType::DOUBLE:
                                    skvRecord.serializeNext<double>(boost::any_cast<double>(value->second));
                                    in_edge_skvRecord.serializeNext<double>(boost::any_cast<double>(value->second));
                                    break;
                                case k2::dto::FieldType::BOOL:
                                    skvRecord.serializeNext<bool>(boost::any_cast<bool>(value->second));
                                    in_edge_skvRecord.serializeNext<bool>(boost::any_cast<bool>(value->second));
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
                                in_edge_skvRecord.serializeNull();
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
                    in_edge_skvrecord_list.push_back(std::move(in_edge_skvRecord));
                }
            } // end of Construct skvrecord

            usleep(90000);
            k2::K2TxnOptions options{};
            options.syncFinalize = true;
            k2graph::MyBeginTxnRequest qr{.opts = options,
                                          .prom = {},
                                          .startTime = k2::Clock::now()};
            auto BeginTxnQResult = qr.prom.get_future();
            pushQ(k2graph::BeginTxnQ, std::move(qr));
            k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
            try
            {
                mtr = BeginTxnQResult.get().GetMtr();
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

            //将所有VID->EdgeType写入系统预留Schema std::map<int64_t, std::vector<int32_t>> VID2EdgeType;
            k2::dto::Schema Vertex2Edge;
            Vertex2Edge.name = std::to_string(2);
            Vertex2Edge.version = 1;
            Vertex2Edge.fields = std::vector<k2::dto::SchemaField>{
                {k2::dto::FieldType::INT64T, "VertexID", false, false},
                {k2::dto::FieldType::INT32T, "EdgeType", false, false}
            };
            Vertex2Edge.setPartitionKeyFieldsByName(std::vector<k2::String>{"VertexID"});
            Vertex2Edge.setRangeKeyFieldsByName(std::vector<k2::String>{"EdgeType"});

            for(auto item = VID2EdgeType.begin(); item != VID2EdgeType.end(); item++) {
                auto VID = item->first;
                set<int32_t> eIDs(item->second.begin(),item->second.end());
                for(auto eID = eIDs.begin(); eID != eIDs.end(); eID++){
                    k2::dto::SKVRecord VID2EID(std::to_string(spaceID), std::make_shared<k2::dto::Schema>(Vertex2Edge));
                    VID2EID.serializeNext<int64_t>(VID);
                    VID2EID.serializeNext<int32_t>(*eID);
                    k2graph::MyWriteRequest write_request{
                        .mtr = mtr,
                        .record = std::move(VID2EID),
                        .prom = std::promise<k2::WriteResult>()
                    };
                    pushQ(k2graph::WriteRequestQ, std::move(write_request));
                }
            }
            //将所有req push进队列 TODO
            auto batchIt = req.parts.begin();
            int batchSize = batchIt->second.size();
            for(int i=0;i<batchSize/2;i++){
            k2graph::MyWriteRequest write_request{
                .mtr = mtr,
                .record = std::move(skvrecord_list[i]),
                .prom = std::promise<k2::WriteResult>()
            };
            k2graph::MyWriteRequest write_request1{
                .mtr = mtr,
                .record = std::move(in_edge_skvrecord_list[i]),
                .prom = std::promise<k2::WriteResult>()
            };
            auto WQResult = write_request.prom.get_future();
            pushQ(k2graph::WriteRequestQ, std::move(write_request));
            pushQ(k2graph::WriteRequestQ, std::move(write_request1));
            try
            {

                auto WriteResult = WQResult.get();
                auto status = WriteResult.status;
                if (!status.is2xxOK())
                {
                    isSucceed = false;
                }
            }
            catch (...)
            {
                isSucceed = false;
            }
            }
            bool isendsucceed = true;
            k2graph::MyEndTxnRequest end_txn_req{
                .mtr = mtr,
                .shouldCommit = isSucceed,
                //.prom = new std::promise<k2::EndResult>()};
                .prom = std::promise<k2::EndResult>()};
            auto EndQResult = end_txn_req.prom.get_future();
            pushQ(k2graph::EndTxnQ, std::move(end_txn_req));
            auto EndResult = EndQResult.get();
            auto status = EndResult.status;
            if (!status.is2xxOK())
            {
                isendsucceed = false;
                std::cout << "end failed";
            }
            cpp2::ResultCode rescode;
            if (!isendsucceed || !isSucceed)
            {
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
            //返回一个future
            promise_.setValue(std::move(resp_));
            // std::cout << "addedge end\n";
            return f;
        }

            folly::Future<cpp2::ExecResponse>
            StorageServiceHandler::future_deleteVertices(const cpp2::DeleteVerticesRequest& req) {
                //删除是通过写操作实现
                folly::Promise<cpp2::ExecResponse> promise_;
                auto f = promise_.getFuture();
                cpp2::ExecResponse resp_;
                cpp2::ResponseCommon responseCommon;
                std::vector<cpp2::ResultCode> failed_codes;
                //开始一个事务
                k2::K2TxnOptions options{};
                options.syncFinalize = true;
                k2graph::MyBeginTxnRequest qr{.opts = options,
                                            .prom = {},
                                            .startTime = k2::Clock::now()};
                auto BeginTxnQResult = qr.prom.get_future();
                pushQ(k2graph::BeginTxnQ, std::move(qr));
                k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
                try
                {
                    mtr = BeginTxnQResult.get().GetMtr();
                }
                catch (...)
                {
                    //构造错误信息,并返回
                    cpp2::ResultCode rescode;
                    rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                    failed_codes.emplace_back(rescode);
                    responseCommon.set_failed_codes(failed_codes);
                    resp_.set_result(responseCommon);
                    promise_.setValue(std::move(resp_));
                    return f;
                }
                //暂时只支持删除一个顶点
                auto spaceID = req.space_id;
                for (auto iter = req.parts.begin(); iter != req.parts.end(); iter++) {
                    auto partID = iter->first;
                    auto vIDs = iter->second;
                    for(auto vID : vIDs) {
                        auto vertexID = vID;
                        k2graph::MyScanReadCreateRequest request{
                            .collectionName = std::to_string(spaceID),
                            .schemaName = std::to_string(2),//存储Vertex和EdgeType的对应关系的schema
                            // .prom = new std::promise<k2graph::CreateScanReadResult>()};
                            .prom = std::promise<k2graph::CreateScanReadResult>()
                        };
                        auto ScanReadResult = request.prom.get_future();
                        std::shared_ptr<k2::Query> scan = nullptr;
                        pushQ(k2graph::scanReadCreateTxQ, std::move(request));

                        try {
                            // auto scan_create_result = request.prom->get_future().get();
                            auto scan_create_result = ScanReadResult.get();
                            auto status = scan_create_result.status;
                            if (!status.is2xxOK()) {
                                std::cout << "l1419 createQuery 错误"
                                        << "\n";
                                cpp2::ResultCode rescode;
                                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                failed_codes.emplace_back(rescode);
                                responseCommon.set_failed_codes(failed_codes);
                                resp_.set_result(responseCommon);
                                promise_.setValue(std::move(resp_));
                                return f;
                            }
                            scan = scan_create_result.query;
                        }
                        catch (...) {
                            cpp2::ResultCode rescode;
                            rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                            failed_codes.emplace_back(rescode);
                            responseCommon.set_failed_codes(failed_codes);
                            resp_.set_result(responseCommon);
                            promise_.setValue(std::move(resp_));
                            return f;
                        }
                        k2::dto::Schema Vertex2Edge;
                        Vertex2Edge.name = std::to_string(2);
                        Vertex2Edge.version = 1;
                        Vertex2Edge.fields = std::vector<k2::dto::SchemaField>{
                            {k2::dto::FieldType::INT64T, "VertexID", false, false},
                            {k2::dto::FieldType::INT32T, "EdgeType", false, false}
                        };
                        Vertex2Edge.setPartitionKeyFieldsByName(std::vector<k2::String>{"VertexID"});
                        Vertex2Edge.setRangeKeyFieldsByName(std::vector<k2::String>{"EdgeType"});
                        k2::dto::SKVRecord startRecord(std::to_string(spaceID), std::make_shared<k2::dto::Schema>(Vertex2Edge));
                        k2::dto::SKVRecord endRecord(std::to_string(spaceID), std::make_shared<k2::dto::Schema>(Vertex2Edge));
                        startRecord.serializeNext<int64_t>(vertexID);
                        endRecord.serializeNext<int64_t>(vertexID);
                        scan->startScanRecord = std::move(startRecord);
                        scan->endScanRecord = std::move(endRecord);
                        scan->setLimit(-1);
                        // range query
                        k2graph::MyScanReadRequest scan_request{
                            .mtr = mtr,
                            .query = std::move(scan),
                            // .prom = new std::promise<k2::QueryResult>()};
                            .prom = std::promise<k2::QueryResult>()
                        };
                        auto readResult = scan_request.prom.get_future();
                        pushQ(k2graph::scanReadTxQ, std::move(scan_request));
                        k2::QueryResult scan_result = readResult.get();
                        auto status = scan_result.status;
                        std::vector<k2::dto::SKVRecord> vID2Edgetype(std::move(scan_result.records));

                        
                        k2graph::MyScanReadCreateRequest request1{
                            .collectionName = std::to_string(spaceID),
                            .schemaName = std::to_string(1),//存储Vertex和tag的对应关系的schema
                            .prom = std::promise<k2graph::CreateScanReadResult>()
                        };
                        auto ScanReadResult1 = request1.prom.get_future();
                        scan = nullptr;
                        pushQ(k2graph::scanReadCreateTxQ, std::move(request1));

                        try {
                            // auto scan_create_result = request.prom->get_future().get();
                            auto scan_create_result1 = ScanReadResult1.get();
                            auto status1 = scan_create_result1.status;
                            if (!status1.is2xxOK()) {
                                std::cout << "l1484 createQuery 错误"
                                        << "\n";
                                cpp2::ResultCode rescode;
                                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                failed_codes.emplace_back(rescode);
                                responseCommon.set_failed_codes(failed_codes);
                                resp_.set_result(responseCommon);
                                promise_.setValue(std::move(resp_));
                                return f;
                            }
                            scan = scan_create_result1.query;
                        }
                        catch (...) {
                            cpp2::ResultCode rescode;
                            rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                            failed_codes.emplace_back(rescode);
                            responseCommon.set_failed_codes(failed_codes);
                            resp_.set_result(responseCommon);
                            promise_.setValue(std::move(resp_));
                            return f;
                        }
                        k2::dto::Schema Vertex2Schema;
                        Vertex2Schema.name = std::to_string(1);
                        Vertex2Schema.version = 1;
                        Vertex2Schema.fields = std::vector<k2::dto::SchemaField>{
                            {k2::dto::FieldType::INT64T, "VertexID", false, false},
                            {k2::dto::FieldType::INT32T, "TagID", false, false}
                        };
                        Vertex2Schema.setPartitionKeyFieldsByName(std::vector<k2::String>{"VertexID"});
                        Vertex2Schema.setRangeKeyFieldsByName(std::vector<k2::String>{"TagID"});
                        k2::dto::SKVRecord startRecord1(std::to_string(spaceID), std::make_shared<k2::dto::Schema>(Vertex2Schema));
                        k2::dto::SKVRecord endRecord1(std::to_string(spaceID), std::make_shared<k2::dto::Schema>(Vertex2Schema));
                        startRecord1.serializeNext<int64_t>(vertexID);
                        endRecord1.serializeNext<int64_t>(vertexID);
                        scan->startScanRecord = std::move(startRecord1);
                        scan->endScanRecord = std::move(endRecord1);
                        scan->setLimit(-1);
                        // range query
                        k2graph::MyScanReadRequest scan_request1{
                            .mtr = mtr,
                            .query = std::move(scan),
                            // .prom = new std::promise<k2::QueryResult>()};
                            .prom = std::promise<k2::QueryResult>()
                        };
                        auto readResult1 = scan_request1.prom.get_future();
                        pushQ(k2graph::scanReadTxQ, std::move(scan_request1));
                        k2::QueryResult scan_result1 = readResult1.get();
                        auto status1 = scan_result1.status;
                        std::vector<k2::dto::SKVRecord> vID2TagID(std::move(scan_result1.records));
                        
                        //构造需要删除的SKVRecord
                        for (auto index = vID2TagID.begin(); index != vID2TagID.end(); index++) {
                            auto record = std::move(*index);
                        // for(auto record: vID2TagID) {
                            //删除对应的顶点 和 该meta数据
                            auto tagid = *(record.deserializeField<int32_t>("TagID"));
                            std::shared_ptr<k2::dto::Schema>  vertex_schema = GetSchemaFromK2(spaceID, tagid);
                            k2::dto::SKVRecord vertexRecord(std::to_string(spaceID), vertex_schema);
                            vertexRecord.serializeNext<int16_t>(partID);
                            vertexRecord.serializeNext<int64_t>(vertexID);
                            vertexRecord.serializeNext<int32_t>(tagid);
                            k2graph::MyWriteRequest delete_request{
                                .mtr = mtr,
                                .erase = true,
                                .record = std::move(vertexRecord), //从上面的序列化得出
                                .prom = std::promise<k2::WriteResult>()
                            };
                            auto DQResult = delete_request.prom.get_future();
                            pushQ(k2graph::WriteRequestQ, std::move(delete_request));
                            auto deleteResult = DQResult.get();
                            auto status = deleteResult.status;
                            k2graph::MyWriteRequest delete_request1{
                                .mtr = mtr,
                                .erase = true,
                                .record = std::move(record), //从上面的序列化得出
                                .prom = std::promise<k2::WriteResult>()
                            };
                            DQResult = delete_request1.prom.get_future();
                            pushQ(k2graph::WriteRequestQ, std::move(delete_request1));
                            deleteResult = DQResult.get();
                            status = deleteResult.status;
                        }
                        for (auto index = vID2Edgetype.begin(); index != vID2Edgetype.end(); index++) {
                            auto record = std::move(*index);
                        // for(auto record: vID2Edgetype) {
                            //删除查找到的边类型和该meta记录
                            auto edgetype = *(record.deserializeField<int32_t>("EdgeType"));
                            auto edge_schema = GetSchemaFromK2(spaceID, edgetype);
                            //范围查询顶点关联的EdgeType的边
                            k2graph::MyScanReadCreateRequest readEdges{
                                .collectionName = std::to_string(spaceID),
                                .schemaName = std::to_string(edgetype),//存储Vertex和EdgeType的对应关系的schema
                                // .prom = new std::promise<k2graph::CreateScanReadResult>()};
                                .prom = std::promise<k2graph::CreateScanReadResult>()
                            };
                            auto readEdgesResult = readEdges.prom.get_future();
                            std::shared_ptr<k2::Query> scan = nullptr;
                            pushQ(k2graph::scanReadCreateTxQ, std::move(readEdges));
                            auto read_edge_result = readEdgesResult.get();
                            auto readEdgestatus = read_edge_result.status;
                            scan = read_edge_result.query;
                            k2::dto::SKVRecord startRecord(std::to_string(spaceID), edge_schema);
                            k2::dto::SKVRecord endRecord(std::to_string(spaceID), edge_schema);
                            startRecord.serializeNext<int16_t>(partID);
                            endRecord.serializeNext<int16_t>(partID);
                            startRecord.serializeNext<int64_t>(vertexID);
                            endRecord.serializeNext<int64_t>(vertexID);
                            startRecord.serializeNext<int32_t>(edgetype);
                            endRecord.serializeNext<int32_t>(edgetype);
                            scan->startScanRecord = std::move(startRecord);
                            scan->endScanRecord = std::move(endRecord);
                            scan->setLimit(-1);
                            k2graph::MyScanReadRequest scan_edge_request{
                                .mtr = mtr,
                                .query = std::move(scan),
                                // .prom = new std::promise<k2::QueryResult>()};
                                .prom = std::promise<k2::QueryResult>()
                            };
                            auto readResult = scan_edge_request.prom.get_future();
                            pushQ(k2graph::scanReadTxQ, std::move(scan_edge_request));
                            k2::QueryResult scan_result = readResult.get();
                            auto scanStatus = scan_result.status;
                            std::vector<k2::dto::SKVRecord> edges(std::move(scan_result.records));
                            for (auto index = edges.begin(); index != edges.end(); index++) {
                                auto edge = std::move(*index);
                            // for(auto edge: edges) {
                                //删除相应的边和对应的相反的边
                                int64_t dstID = *(edge.deserializeField<int64_t>("SecondVertexID"));
                                int64_t rank = *(edge.deserializeField<int64_t>("Rank"));
                                //构造相反的边
                                k2::dto::SKVRecord reverse_edge(std::to_string(spaceID), edge_schema);
                                reverse_edge.serializeNext<int16_t>(partID);
                                reverse_edge.serializeNext<int64_t>(dstID);
                                reverse_edge.serializeNext<int32_t>(-edgetype);
                                reverse_edge.serializeNext<int64_t>(rank);
                                reverse_edge.serializeNext<int64_t>(vertexID);
                                k2graph::MyWriteRequest delete_inedge_request{
                                    .mtr = mtr,
                                    .erase = true,
                                    .record = std::move(edge), //从上面的序列化得出
                                    .prom = std::promise<k2::WriteResult>()
                                };
                                k2graph::MyWriteRequest delete_outedge_request{
                                    .mtr = mtr,
                                    .erase = true,
                                    .record = std::move(reverse_edge), //从上面的序列化得出
                                    .prom = std::promise<k2::WriteResult>()
                                };
                                auto DQResult1 = delete_inedge_request.prom.get_future();
                                pushQ(k2graph::WriteRequestQ, std::move(delete_inedge_request));
                                auto DQResult2 = delete_outedge_request.prom.get_future();
                                pushQ(k2graph::WriteRequestQ, std::move(delete_outedge_request));
                                auto deleteResult1 = DQResult1.get();
                                status = deleteResult1.status;
                                auto deleteResult2 = DQResult2.get();
                                status = deleteResult2.status;
                            }
                            k2graph::MyWriteRequest delete_request1{
                                .mtr = mtr,
                                .erase = true,
                                .record = std::move(record), //从上面的序列化得出
                                .prom = std::promise<k2::WriteResult>()
                            };
                            auto DQResult = delete_request1.prom.get_future();
                            pushQ(k2graph::WriteRequestQ, std::move(delete_request1));
                            auto deleteResult = DQResult.get();
                            status = deleteResult.status;
                        }
                    }
                }
                k2graph::MyEndTxnRequest end_txn_req{
                    .mtr = mtr,
                    .shouldCommit = true,
                    //.prom = new std::promise<k2::EndResult>()
                    .prom = std::promise<k2::EndResult>()};
                auto EndQResult = end_txn_req.prom.get_future();
                pushQ(k2graph::EndTxnQ, std::move(end_txn_req));
                auto EndResult = EndQResult.get();
                auto Endstatus = EndResult.status;
                responseCommon.set_failed_codes(failed_codes);
                responseCommon.set_latency_in_us(10);
                resp_.set_result(responseCommon);
                promise_.setValue(std::move(resp_));
                return f;
            }
       /*
                folly::Future<cpp2::ExecResponse>
                StorageServiceHandler::future_deleteEdges(const cpp2::DeleteEdgesRequest& req) {
                    auto* processor = DeleteEdgesProcessor::instance(kvstore_, schemaMan_, indexMan_);
                    RETURN_FUTURE(processor);
                }
*/
        folly::Future<cpp2::UpdateResponse>
        StorageServiceHandler::future_updateVertex(const cpp2::UpdateVertexRequest& req) {
            folly::Promise<cpp2::UpdateResponse> promise_;
            auto f = promise_.getFuture();
            cpp2::UpdateResponse resp_;
            cpp2::ResponseCommon responseCommon;
            std::vector<cpp2::ResultCode> failed_codes;
            // bool isSucceed = true;

            std::vector<uint32_t> fieldsForUpdate;
            auto spaceID = req.space_id;
            //先考虑一个update请求只有一个更新请求,也就是只针对一个顶点的某个tag的属性进行修改
            auto vertexID = req.vertex_id;
            auto partID   = req.part_id;
            std::string target_name;
            //开始一个事务
            k2::K2TxnOptions options{};
            options.syncFinalize = true;
            k2graph::MyBeginTxnRequest qr{.opts = options,
                                          //.prom = new std::promise<k2::dto::K23SI_MTR>(),
                                          .prom = {},
                                          .startTime = k2::Clock::now()};
            auto BeginTxnResult = qr.prom.get_future();
            pushQ(k2graph::BeginTxnQ, std::move(qr));
            k2::dto::K23SI_MTR mtr; //标识事务，之后的请求要用到
            try
            {
                mtr = BeginTxnResult.get().GetMtr();
            }
            catch (...)
            {
                //构造错误信息,并返回
                cpp2::ResultCode rescode;
                rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                failed_codes.emplace_back(rescode);
                responseCommon.set_failed_codes(failed_codes);
                resp_.set_result(responseCommon);
                promise_.setValue(std::move(resp_));
                return f;
            }
            std::cout << "begintxn succeed\n";
            for(auto iter = req.update_items.begin(); iter != req.update_items.end(); iter++) {
                //获得tagID
                auto tagName = iter->name;
                target_name = boost::any_cast<std::string>(iter->prop);
                k2::dto::Schema TagSchema1;
                TagSchema1.name = std::to_string(0);
                TagSchema1.version = 1;
                TagSchema1.fields = std::vector<k2::dto::SchemaField>{
                    {k2::dto::FieldType::INT64T, "SpaceID", false, false},
                    {k2::dto::FieldType::STRING, "TagName", false, false},
                    {k2::dto::FieldType::INT32T, "TagID", false, false}
                };
                TagSchema1.setPartitionKeyFieldsByName(std::vector<k2::String>{"SpaceID"});
                TagSchema1.setRangeKeyFieldsByName(std::vector<k2::String>{"TagName"});
                k2::dto::SKVRecord tag_record(std::to_string(spaceID), std::make_shared<k2::dto::Schema>(TagSchema1));
                tag_record.serializeNext<int64_t>(spaceID);
                tag_record.serializeNext<k2::String>(boost::any_cast<std::string>(tagName));
                
                //将prop转换成chogori的fieldsForUpdate形式，就是记录位置
                k2graph::MyReadRequest read_request{
                            .mtr = mtr,
                            .record = k2::dto::SKVRecord(),
                            .key = std::move(tag_record.getKey()),
                            .collectionName = std::to_string(spaceID),
                            //  .prom = new std::promise<k2::ReadResult<k2::dto::SKVRecord>>()};
                            .prom = std::promise<k2::ReadResult<k2::dto::SKVRecord>>()
                };
                auto ReadResult = read_request.prom.get_future();
                pushQ(k2graph::readTxQ, std::move(read_request));
                auto MyReadResult = ReadResult.get();
                auto status = MyReadResult.status;
                if (!status.is2xxOK()) {
                    cpp2::ResultCode rescode;
                    std::cout << status << "\n";
                    rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                    failed_codes.emplace_back(rescode);
                    responseCommon.set_failed_codes(failed_codes);
                    resp_.set_result(responseCommon);
                    promise_.setValue(std::move(resp_));
                    return f;
                }
                std::cout << "read TagID succeed\n";
                auto res = MyReadResult.value.deserializeField<int32_t>("TagID");
                auto tagID = *res;
                std::cout << tagID << std::endl;
                //获取Tag
                std::shared_ptr<k2::dto::Schema> schema;

                if (SchemaTable.find(tagID) != SchemaTable.end())
                {
                    schema = SchemaTable[tagID];
                }
                else
                {
                    schema = GetSchemaFromK2(spaceID, tagID);
                }
                //获得原顶点的skvrecord
                k2::dto::SKVRecord vertex_record(std::to_string(spaceID), schema);
                vertex_record.serializeNext<int16_t>(partID);
                vertex_record.serializeNext<int64_t>(vertexID);
                vertex_record.serializeNext<int32_t>(tagID);
                k2graph::MyReadRequest vertex_read_request{
                                .mtr = mtr,
                                .record = k2::dto::SKVRecord(),
                                .key = std::move(vertex_record.getKey()),
                                .collectionName = std::to_string(spaceID),
                                //  .prom = new std::promise<k2::ReadResult<k2::dto::SKVRecord>>()};
                                .prom = std::promise<k2::ReadResult<k2::dto::SKVRecord>>()};
                
                auto vertex_ReadResult = vertex_read_request.prom.get_future();
                pushQ(k2graph::readTxQ, std::move(vertex_read_request));
                std::cout << "l1497 read oldrecord success" << std::endl;
                //先假设正常读取 TODO
                auto origin_skvRecord = std::move(vertex_ReadResult.get().value);
                //将prop转换成chogori的fieldsForUpdate形式，就是记录位置
                //就说查找prop的名字然后查找其在schema中的位置
                //value是要更换的值
                //其余值不变
                //value直接替换record中的部分
                k2::dto::SKVRecord new_vertex_record(std::to_string(spaceID), schema);
                // auto exp = Expression::decode(iter->get_value()).value();
                // if(auto v = Expression::asInt(iter->get_value())){
                //     std::cout << v << std::endl;
                    
                // }
                Getters getters;
                auto value = Expression::decode(iter->get_value()).value()->eval(getters).value();
                // std::cout << myvalue << std::endl;
                // auto value = iter->get_value();
                auto fields = schema->fields;
                std::cout << "l1507" << std::endl;
                int i = 0;
                for (auto field = fields.begin(); field != fields.end(); field++) {
                    auto name = field->name.c_str();
                    std::cout << "target name: "<<target_name << std::endl;
                    std::cout << "name: "<<name << std::endl;
                    std::cout << "type: "<<field->type << std::endl;
                    auto type = field->type;
                    if(name == target_name) {
                        fieldsForUpdate.push_back(i);
                    }
                    switch (type) {
                        case k2::dto::FieldType::STRING:
                                    if(name == target_name) {
                                        new_vertex_record.serializeNext<k2::String>(boost::any_cast<std::string>(value));
                                    }
                                    else{
                                        auto temp = origin_skvRecord.template deserializeField<k2::String>(name);
                                        new_vertex_record.serializeNext<k2::String>(*temp);
                                    }
                        break;
                        case k2::dto::FieldType::INT16T:
                                    if(name == target_name) {
                                        new_vertex_record.serializeNext<int16_t>(boost::any_cast<int16_t>(value));
                                    }
                                    else{
                                        auto temp = origin_skvRecord.deserializeField<int16_t>(name);
                                        new_vertex_record.serializeNext<int16_t>(*temp);
                                    }
                        break;
                        case k2::dto::FieldType::INT32T:
                                    if(name == target_name) {
                                        new_vertex_record.serializeNext<int32_t>(boost::any_cast<int32_t>(value));
                                    }
                                    else{
                                        auto temp = origin_skvRecord.deserializeField<int32_t>(name);
                                        new_vertex_record.serializeNext<int32_t>(*temp);
                                    }
                        break;
                        case k2::dto::FieldType::INT64T:
                                    if(name == target_name) {
                                        new_vertex_record.serializeNext<int64_t>(boost::get<int64_t>(value));
                                    }
                                    else{
                                        auto temp = origin_skvRecord.deserializeField<int64_t>(name);
                                        new_vertex_record.serializeNext<int64_t>(*temp);
                                    }
                        break;
                        case k2::dto::FieldType::FLOAT:
                                    if(name == target_name) {
                                        new_vertex_record.serializeNext<float>(boost::any_cast<float>(value));
                                    }
                                    else{
                                        auto temp = origin_skvRecord.deserializeField<float>(name);
                                        new_vertex_record.serializeNext<float>(*temp);
                                    }
                        break;
                        case k2::dto::FieldType::DOUBLE:
                                    if(name == target_name) {
                                        new_vertex_record.serializeNext<double>(boost::get<double>(value));
                                    }
                                    else{
                                        auto temp = origin_skvRecord.deserializeField<double>(name);
                                        new_vertex_record.serializeNext<double>(*temp);
                                    }
                        break;
                        case k2::dto::FieldType::BOOL:
                                    if(name == target_name) {
                                        new_vertex_record.serializeNext<bool>(boost::get<bool>(value));
                                    }
                                    else{
                                        auto temp = origin_skvRecord.deserializeField<bool>(name);
                                        new_vertex_record.serializeNext<bool>(*temp);
                                    }
                        break;
                        default:
                                    //构造错误信息,并返回
                                    // TODO：考虑多个顶点出错的情况，是直接返回，还是将正确的执行后返回
                                    cpp2::ResultCode rescode;
                                    rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                                    failed_codes.emplace_back(rescode);
                                    responseCommon.set_failed_codes(failed_codes);
                                    resp_.set_result(responseCommon);
                                    promise_.setValue(std::move(resp_));
                                    return f;
                    }
                    i++;
                }
                std::cout << "construct newrecords succeed\n";
                k2graph::MyUpdateRequest update_request{
                                .mtr = mtr,
                                .record = std::move(new_vertex_record),
                                .fieldsForUpdate = std::move(fieldsForUpdate),
                                .key = std::move(new_vertex_record.getKey()),
                                .prom = std::promise<k2::PartialUpdateResult>()
                };
                auto updateResult = update_request.prom.get_future();
                k2graph::pushQ(k2graph::UpdateTxQ, std::move(update_request));
                if(!updateResult.get().status.is2xxOK()){
                    cpp2::ResultCode rescode;
                    rescode.set_code(cpp2::ErrorCode::E_UNKNOWN);
                    failed_codes.emplace_back(rescode);
                    responseCommon.set_failed_codes(failed_codes);
                    resp_.set_result(responseCommon);
                    promise_.setValue(std::move(resp_));
                    return f;
                }
                std::cout << "update succeed\n";
            }
            k2graph::MyEndTxnRequest end_txn_req{
                .mtr = mtr,
                .shouldCommit = true,
                //.prom = new std::promise<k2::EndResult>()};
                .prom = std::promise<k2::EndResult>()};
            auto EndTxnQResult = end_txn_req.prom.get_future();
            pushQ(k2graph::EndTxnQ, std::move(end_txn_req));
            promise_.setValue(std::move(resp_));
            return f;

        }
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
