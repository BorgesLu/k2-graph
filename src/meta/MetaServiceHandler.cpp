/* Copyright (c) 2018 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "meta/MetaServiceUtils.h"
#include "meta/MetaServiceHandler.h"
#include "meta/processors/partsMan/CreateSpaceProcessor.h"
#include "meta/processors/partsMan/DropSpaceProcessor.h"
#include "meta/processors/partsMan/ListSpacesProcessor.h"
#include "meta/processors/partsMan/GetSpaceProcessor.h"
#include "meta/processors/partsMan/ListHostsProcessor.h"
#include "meta/processors/partsMan/ListPartsProcessor.h"
#include "meta/processors/partsMan/GetPartsAllocProcessor.h"
#include "meta/processors/schemaMan/CreateTagProcessor.h"
#include "meta/processors/schemaMan/AlterTagProcessor.h"
#include "meta/processors/schemaMan/DropTagProcessor.h"
#include "meta/processors/schemaMan/GetTagProcessor.h"
#include "meta/processors/schemaMan/ListTagsProcessor.h"
#include "meta/processors/schemaMan/CreateEdgeProcessor.h"
#include "meta/processors/schemaMan/AlterEdgeProcessor.h"
#include "meta/processors/schemaMan/DropEdgeProcessor.h"
#include "meta/processors/schemaMan/GetEdgeProcessor.h"
#include "meta/processors/schemaMan/ListEdgesProcessor.h"
#include "meta/processors/indexMan/CreateTagIndexProcessor.h"
#include "meta/processors/indexMan/DropTagIndexProcessor.h"
#include "meta/processors/indexMan/GetTagIndexProcessor.h"
#include "meta/processors/indexMan/ListTagIndexesProcessor.h"
#include "meta/processors/indexMan/RebuildTagIndexProcessor.h"
#include "meta/processors/indexMan/ListTagIndexStatusProcessor.h"
#include "meta/processors/indexMan/CreateEdgeIndexProcessor.h"
#include "meta/processors/indexMan/DropEdgeIndexProcessor.h"
#include "meta/processors/indexMan/GetEdgeIndexProcessor.h"
#include "meta/processors/indexMan/ListEdgeIndexesProcessor.h"
#include "meta/processors/indexMan/RebuildEdgeIndexProcessor.h"
#include "meta/processors/indexMan/ListEdgeIndexStatusProcessor.h"
#include "meta/processors/customKV/MultiPutProcessor.h"
#include "meta/processors/customKV/GetProcessor.h"
#include "meta/processors/customKV/MultiGetProcessor.h"
#include "meta/processors/customKV/ScanProcessor.h"
#include "meta/processors/customKV/RemoveProcessor.h"
#include "meta/processors/customKV/RemoveRangeProcessor.h"
#include "meta/processors/admin/HBProcessor.h"
#include "meta/processors/usersMan/AuthenticationProcessor.h"
#include "meta/processors/admin/BalanceProcessor.h"
#include "meta/processors/admin/LeaderBalanceProcessor.h"
#include "meta/processors/admin/CreateSnapshotProcessor.h"
#include "meta/processors/admin/DropSnapshotProcessor.h"
#include "meta/processors/admin/ListSnapshotsProcessor.h"
#include "meta/processors/configMan/RegConfigProcessor.h"
#include "meta/processors/configMan/GetConfigProcessor.h"
#include "meta/processors/configMan/SetConfigProcessor.h"
#include "meta/processors/configMan/ListConfigsProcessor.h"
#include "meta/processors/jobMan/AdminJobProcessor.h"

#include "k2_includes.h"
#include "k2_queue_defs.h"

#define RETURN_FUTURE(processor)     \
    auto f = processor->getFuture(); \
    processor->process(req);         \
    return f;

namespace nebula
{
    namespace meta
    {
//通用的数据结构；

        std::unordered_map<GraphSpaceID, std::string> spaceIdNameMap_;

        using schemaID = int32_t;
        using tagID = int32_t;
        using edgeID = int32_t;
        //顶点和边的schema name都不允许重复，不同space 之间的名称相互隔离
        std::unordered_map<std::pair<GraphSpaceID, std::string>,schemaID>  SpaceSchemaNameIDMap ;
        
        //维护schema的信息。减少对K2的访问和类型转换。
        //对点和边的schema是分开访问的，所以这里维护了两张表
        //TagItem 内容：TagID, TagName,SchemaerVer,Schema
        std::unordered_map<std::pair<GraphSpaceID, tagID>,cpp2::TagItem> SpaceTagMap;
        std::unordered_map<std::pair<GraphSpaceID, edgeID>,nebula::cpp2::Schema> SpaceEdgeMap;



//通用函数
            
    GraphSpaceID getSpaceID(const std::string& name){         
            for(auto &iter:spaceIdNameMap_){
                if (iter.second == name)
                    return iter.first;
            }
            return -1;
    }


        folly::Future<cpp2::ListSpacesResp>
        MetaServiceHandler::future_listSpaces(const cpp2::ListSpacesReq &req)
        {
           // std::cout << "list space called" << std::endl;
            folly::Promise<cpp2::ListSpacesResp> promise_;
            auto f = promise_.getFuture();
            cpp2::ListSpacesResp resp_;

            std::vector<cpp2::IdName> spaces; //返回sapcename 与 ID 的映射，也就是目前实现的 spaceTable；
           // std::string spaceName = "Test";
           // cpp2::ID id;
           // id.set_space_id(0);
           // cpp2::IdName space;

           // spaceid 0 保留
          GraphSpaceID DefaultSpaceId = 0;
          spaceIdNameMap_[DefaultSpaceId] = "DefaultSapce";
            
            for(auto &iter:spaceIdNameMap_){
                 cpp2::IdName space;
                 cpp2::ID id;
                 id.set_space_id(iter.first);
                 space.set_id(id);
                 std::cout<<"listSpace(): "<<iter.second<<std::endl;
                 space.set_name(iter.second);
                 
                 spaces.emplace_back(std::move(space));

            }
          //  space.set_id(id);
          //  space.set_name(std::move(spaceName));
          //  spaces.emplace_back(std::move(space));

            resp_.set_spaces(std::move(spaces));

//ip 
            nebula::cpp2::HostAddr host_addr;

            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;

            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);

            auto localMeatHost = hostAddrRet.value();
           // std::cout << "ip is: " << localMeatHost.first << std::endl;
           // std::cout << "port is: " << localMeatHost.second << std::endl;

            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);
            //  resp_.set_leader(localhost);
            resp_.code = cpp2::ErrorCode::SUCCEEDED;

            promise_.setValue(std::move(resp_));

          //  std::cout << "list space end\n\n";

            return f;
        }


//without k2
/*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_createSpace(const cpp2::CreateSpaceReq& req) {
           
            std::cout<<"create space called\n";
            folly::Promise<cpp2::ExecResp> promise_;
            auto f = promise_.getFuture();
            cpp2::ExecResp resp_;

            auto properties = req.get_properties();
            std::string mySpaceName =  properties.get_space_name();
            std::cout<<"create space():"<<mySpaceName<<std::endl;
            GraphSpaceID mySpaceID = spaceIdNameMap_.size();
            spaceIdNameMap_[mySpaceID] = mySpaceName;
            
            cpp2::ID thriftID;
            thriftID.set_space_id(mySpaceID);
            resp_.set_id(thriftID);

            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);

            //leader ip
            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;

            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);

            std::cout<<"create space end\n";  

            promise_.setValue(std::move(resp_));
            return f;    
        }
*/

//create space with K2 

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_createSpace(const cpp2::CreateSpaceReq& req) {
            std::cout << "Create space with k2 called!\n";
            folly::Promise<cpp2::ExecResp> promise_;
            auto f = promise_.getFuture();
            cpp2::ExecResp resp_;
            
            auto properties = req.get_properties();
            std::string mySpaceName =  properties.get_space_name();

            static std::unordered_map<std::string, std::vector<std::string>> name2Eps = {
            {"test1", {"tcp+k2rpc://0.0.0.0:10000"}},{"test2", {"tcp+k2rpc://0.0.0.0:10001"}},{"test3", {"tcp+k2rpc://0.0.0.0:10002"}}
        };
            static std::unordered_map<std::string, int32_t> spaceTable;

             bool isRepeated = false;
            int32_t mySpaceID;
            std::vector<k2::String> endpoints;
            std::vector<std::string> stdEndpoints = name2Eps[mySpaceName];
            for (const std::string& ep : stdEndpoints) {
                    endpoints.emplace_back(ep);
            }

                //rangeEnds
            std::vector<k2::String> rangeEnds;
            rangeEnds.push_back("");

            //判断name是否出现过
            for(auto &iter:spaceIdNameMap_){
                if(iter.second == mySpaceName){
                        std::cout << "space alerady exist!";
                        isRepeated = true;
                        resp_.set_code(cpp2::ErrorCode::E_EXISTED);
                        mySpaceID = -1;
                }               
            }

            if (!isRepeated)
            {
                mySpaceID = spaceIdNameMap_.size();
                spaceIdNameMap_[mySpaceID] = mySpaceName;
            }

            k2graph::MyCollectionCreateRequest myRequest{
                    .req = k2::dto::CollectionCreateRequest{
                        .metadata{
                            .name = std::to_string(mySpaceID),
                            .hashScheme = k2::dto::HashScheme::HashCRC32C,
                            .storageDriver = k2::dto::StorageDriver::K23SI,
                            .capacity{},
                            .retentionPeriod = 24h
                        },
                        .clusterEndpoints = std::move(endpoints),
                        .rangeEnds = std::move(rangeEnds)
                    },
                    .prom = new std::promise<k2::Status>()
                };

                k2graph::pushQ(k2graph::collectionCreateQ, myRequest);

                try
                { //future.get()时可能抛出异常
                    
                    std::future<k2::Status> result = myRequest.prom->get_future();
                    k2::Status  status = result.get();

                    if (!status.is2xxOK())
                    {

                        std::cout<<"fail to create a collection"<<std::endl;
                        std::cout << status << std::endl;
                    }
                    else
                    {
                     std::cout<<"success "<<std::endl;
                     resp_.set_code(cpp2::ErrorCode::SUCCEEDED);
  
                    }
                }
                catch (...)
                {
                  //  _return.code = ErrorCode::E_UNKNOWN;
                  //  return;
                }

            cpp2::ID thriftID;
            thriftID.set_space_id(mySpaceID);
            resp_.set_id(thriftID);      

         //leader ip
            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;

            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);

            promise_.setValue(std::move(resp_));
            return f;

            std::cout<<"create space with k2 end\n";


        }
        

/*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_dropSpace(const cpp2::DropSpaceReq& req) {
            auto* processor = DropSpaceProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }



        folly::Future<cpp2::AdminJobResp>
        MetaServiceHandler::future_runAdminJob(const cpp2::AdminJobReq& req) {
            auto* processor = AdminJobProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

*/
        folly::Future<cpp2::GetSpaceResp>
        MetaServiceHandler::future_getSpace(const cpp2::GetSpaceReq& req) {
            folly::Promise<cpp2::GetSpaceResp> promise_ ;
            auto f = promise_.getFuture();
            cpp2::GetSpaceResp resp_;

              auto spaceName = req.get_space_name();
              auto spaceID = getSpaceID(spaceName);
              if(spaceID == -1){
                  std::cout<<"space not exist!\n";
                  //return;
              }

            cpp2::SpaceProperties properties;
            properties.set_space_name(spaceName);
            properties.set_partition_num(1); //暂时设为1, raft分区。在K2不在需要
            properties.set_replica_factor(1);  //暂时设为1,多副本，容错，K2由远端的plog保证

            cpp2::SpaceItem item;
            item.set_space_id(spaceID);
            item.set_properties(properties);
            resp_.set_item(item);

            //error-code  始终都是成功
            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);

            //leader ip
            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;

            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);  

            promise_.setValue(std::move(resp_));
            return f;  

        }
/*
        folly::Future<cpp2::ListHostsResp>
        MetaServiceHandler::future_listHosts(const cpp2::ListHostsReq& req) {
            auto* processor = ListHostsProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ListPartsResp>
        MetaServiceHandler::future_listParts(const cpp2::ListPartsReq& req) {
            auto* processor = ListPartsProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
*/
        folly::Future<cpp2::GetPartsAllocResp>
        MetaServiceHandler::future_getPartsAlloc(const cpp2::GetPartsAllocReq& req) {
         //   auto* processor = GetPartsAllocProcessor::instance(kvstore_);
         //   RETURN_FUTURE(processor);
       //  std::cout<<"get partsAlloc called\n\n";
        folly::Promise<cpp2::GetPartsAllocResp> promise_ ;
        auto f = promise_.getFuture();
        cpp2::GetPartsAllocResp resp_;

         auto spaceID = req.get_space_id();
        // std::cout<<"spaceID is:"<< spaceID<<std::endl;

        nebula::cpp2::HostAddr host_addr;
        std::string MetaIp = "127.0.0.1";
        int32_t MetaPort = 9777;
        auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
        auto localMeatHost = hostAddrRet.value();
        host_addr.set_ip(localMeatHost.first);
        host_addr.set_port(localMeatHost.second);
        resp_.set_leader(host_addr);
        
        decltype(resp_.parts) parts; 
        nebula::PartitionID partId = 0; 
        std::vector<nebula::cpp2::HostAddr> partHosts{host_addr};
       
        parts.emplace(partId, std::move(partHosts));
        resp_.set_parts(std::move(parts));

        
         promise_.setValue(std::move(resp_));
       //   std::cout<<"end of get partsAlloc \n\n";
         return f; 

        }
/*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_multiPut(const cpp2::MultiPutReq& req) {
            auto* processor = MultiPutProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::GetResp>
        MetaServiceHandler::future_get(const cpp2::GetReq& req) {
            auto* processor = GetProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::MultiGetResp>
        MetaServiceHandler::future_multiGet(const cpp2::MultiGetReq& req) {
            auto* processor = MultiGetProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ScanResp>
        MetaServiceHandler::future_scan(const cpp2::ScanReq& req) {
            auto* processor = ScanProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_remove(const cpp2::RemoveReq& req) {
            auto* processor = RemoveProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_removeRange(const cpp2::RemoveRangeReq& req) {
            auto* processor = RemoveRangeProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
*/
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_createTag(const cpp2::CreateTagReq& req) {
            std::cout<<"create tag called\n";
            folly::Promise<cpp2::ExecResp> promise_ ;
            auto f = promise_.getFuture();
            cpp2::ExecResp resp_;

            
            bool isRepeated = false;
            int32_t tagID;

            auto spaceID = req.get_space_id();  //todo : 检查spaceID 的合法性
            auto tagName = req.get_tag_name();
            auto iter =  SpaceSchemaNameIDMap.find(std::make_pair(spaceID,tagName));
            if(iter != SpaceSchemaNameIDMap.end()){
                resp_.set_code(cpp2::ErrorCode::E_EXISTED);
                tagID = -1;
                isRepeated = true;
            }

            if(!isRepeated){
                tagID = SpaceSchemaNameIDMap.size();
                auto pair = std::make_pair(spaceID,tagName);
                SpaceSchemaNameIDMap[pair] = tagID;
            }

            k2::dto::Schema TagSchema;
            TagSchema.name = std::to_string(tagID);
            TagSchema.version = 1;

//在这里维护SpaceTagMap的信息
          
            tagID = SpaceSchemaNameIDMap.size();
            auto pair = std::make_pair(spaceID,tagID);
            cpp2::TagItem item;
            int64_t version = 1;
            item.set_tag_id(tagID);
            item.set_tag_name(tagName);
            item.set_version(version);
            item.set_schema(req.schema);

            SpaceTagMap[pair] = item;


            TagSchema.fields = std::vector<k2::dto::SchemaField>{
            {k2::dto::FieldType::INT16T, "PartID", false, false},
            {k2::dto::FieldType::INT64T, "VertexID", false, false},
            {k2::dto::FieldType::INT32T, "TagID", false, false}};

            TagSchema.setPartitionKeyFieldsByName(std::vector<k2::String>{"PartID"});
            TagSchema.setRangeKeyFieldsByName(std::vector<k2::String>{"VertexID", "TagID"});

        //自定义的schema字段
           auto columns = req.get_schema().get_columns();
          if (!columns.empty()){
              k2::dto::SchemaField Graphfield;
              for(auto& column : columns){
                  auto name = column.get_name();
                  switch(column.get_type().get_type()){
                       case nebula::cpp2::SupportedType::BOOL:                          
                            Graphfield.type = k2::dto::FieldType::BOOL;
                            Graphfield.name = name;
                          //  Graphfield.descending = false;
                          //  Graphfield.nullLast = false;
                            TagSchema.fields.push_back(Graphfield);
                            break;
                        case nebula::cpp2::SupportedType::INT:
                           Graphfield.type = k2::dto::FieldType::INT32T;
                            Graphfield.name = name;
                            TagSchema.fields.push_back(Graphfield);
                            break;
                        case nebula::cpp2::SupportedType::DOUBLE:
                            Graphfield.type = k2::dto::FieldType::DOUBLE;
                            Graphfield.name = name;
                            TagSchema.fields.push_back(Graphfield);
                            break;
                        case nebula::cpp2::SupportedType::STRING:
                            Graphfield.type = k2::dto::FieldType::STRING;
                            Graphfield.name = name;
                            TagSchema.fields.push_back(Graphfield);
                            break;                       
                        default:
                        //todo : 处理不支持的数据类型
                         LOG(ERROR) << "Unknown type " << static_cast<int>(column.get_type().get_type());
                         break;
                  }
              }
          }

        k2graph::MySchemaCreateRequest myRequest{
            .req = k2::dto::CreateSchemaRequest{
                .collectionName = std::to_string(spaceID),
                .schema = std::move(TagSchema)
            },
            .prom = new std::promise<k2::CreateSchemaResult>()
        };

        k2graph::pushQ(k2graph::SchemaCreateQ, myRequest);

         try
        { //future.get()时可能抛出异常           
            auto result = myRequest.prom->get_future();
            auto CreateSchemaResult  = result.get();
            auto status = CreateSchemaResult.status;
            if (!status.is2xxOK())
            {

                std::cout<<"fail to create tag"<<std::endl;
                std::cout << status << std::endl;
            }
            else
            {
                std::cout<<"success "<<std::endl;
                resp_.set_code(cpp2::ErrorCode::SUCCEEDED);

            }
        }
        catch (...)
        {
            //  _return.code = ErrorCode::E_UNKNOWN;
            //  return;
        }


            cpp2::ID thriftID;
            thriftID.set_tag_id(tagID);
            resp_.set_id(thriftID);      

         //leader ip
            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;

            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);

            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);

            promise_.setValue(std::move(resp_));
            return f;

            std::cout<<"create tag with k2 end\n";



        }
/*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_alterTag(const cpp2::AlterTagReq& req) {
            auto* processor = AlterTagProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_dropTag(const cpp2::DropTagReq& req) {
            auto* processor = DropTagProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
*/
        folly::Future<cpp2::GetTagResp>
        MetaServiceHandler::future_getTag(const cpp2::GetTagReq &req) {
           std::cout<<"get tag called\n";
           std::cout<<"get tag end\n";
        }


        folly::Future<cpp2::ListTagsResp>
        MetaServiceHandler::future_listTags(const cpp2::ListTagsReq& req) {
            std::cout << "list tags called\n" ;
            folly::Promise<cpp2::ListTagsResp> promise_ ;
            auto f = promise_.getFuture();
            cpp2::ListTagsResp resp_;

            auto spaceId = req.get_space_id();

            decltype(resp_.tags) tags;

            for(auto &iter:SpaceTagMap){
                if(iter.first.first == spaceId)
                {   
                    std::cout<<iter.first.first;
                    tags.emplace_back(iter.second);

                }
            
            }

            resp_.set_tags(std::move(tags));
          
            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);



            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;
            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);

            promise_.setValue(std::move(resp_));
            std::cout << "list tags end\n" ;
            return f;

        }

/*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_createEdge(const cpp2::CreateEdgeReq& req) {
            auto* processor = CreateEdgeProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_alterEdge(const cpp2::AlterEdgeReq& req) {
            auto* processor = AlterEdgeProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_dropEdge(const cpp2::DropEdgeReq& req) {
            auto* processor = DropEdgeProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::GetEdgeResp>
        MetaServiceHandler::future_getEdge(const cpp2::GetEdgeReq& req) {
            auto* processor = GetEdgeProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
*/
        folly::Future<cpp2::ListEdgesResp>
        MetaServiceHandler::future_listEdges(const cpp2::ListEdgesReq& req) {
           // auto* processor = ListEdgesProcessor::instance(kvstore_);
           // RETURN_FUTURE(processor);
          // std::cout<<"list edge called\n\n";
            folly::Promise<cpp2::ListEdgesResp> promise_ ;
            auto f = promise_.getFuture();
            cpp2::ListEdgesResp resp_;

            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);

            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;
            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);

            promise_.setValue(std::move(resp_));
            return f;
        }
/*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_createTagIndex(const cpp2::CreateTagIndexReq& req) {
            auto* processor = CreateTagIndexProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_dropTagIndex(const cpp2::DropTagIndexReq& req) {
            auto* processor = DropTagIndexProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::GetTagIndexResp>
        MetaServiceHandler::future_getTagIndex(const cpp2::GetTagIndexReq &req) {
            auto* processor = GetTagIndexProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
*/
        folly::Future<cpp2::ListTagIndexesResp>
        MetaServiceHandler::future_listTagIndexes(const cpp2::ListTagIndexesReq& req) {
                      //  std::cout << "list tagIndexes called\n" ;
            folly::Promise<cpp2::ListTagIndexesResp> promise_ ;
            auto f = promise_.getFuture();
            cpp2::ListTagIndexesResp resp_;

            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);

            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;
            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);

            promise_.setValue(std::move(resp_));
            //std::cout << "list tagIndexes end\n" ;
            return f;

        }
/*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_rebuildTagIndex(const cpp2::RebuildIndexReq& req) {
            auto* processor = RebuildTagIndexProcessor::instance(kvstore_, adminClient_.get());
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ListIndexStatusResp>
        MetaServiceHandler::future_listTagIndexStatus(const cpp2::ListIndexStatusReq& req) {
            auto* processor = ListTagIndexStatusProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_createEdgeIndex(const cpp2::CreateEdgeIndexReq& req) {
            auto* processor = CreateEdgeIndexProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_dropEdgeIndex(const cpp2::DropEdgeIndexReq& req) {
            auto* processor = DropEdgeIndexProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::GetEdgeIndexResp>
        MetaServiceHandler::future_getEdgeIndex(const cpp2::GetEdgeIndexReq& req) {
            auto* processor = GetEdgeIndexProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

*/
        folly::Future<cpp2::ListEdgeIndexesResp>
        MetaServiceHandler::future_listEdgeIndexes(const cpp2::ListEdgeIndexesReq& req) {
           // auto* processor = ListEdgeIndexesProcessor::instance(kvstore_);
           // RETURN_FUTURE(processor);
            //std::cout << "list EdgeIndexes called\n" ;
            folly::Promise<cpp2::ListEdgeIndexesResp> promise_ ;
            auto f = promise_.getFuture();
            cpp2::ListEdgeIndexesResp resp_;

            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);

            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;
            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);

            promise_.setValue(std::move(resp_));
          //  std::cout << "list edgeIndexes end\n" ;
            return f;
           
        }
/*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_rebuildEdgeIndex(const cpp2::RebuildIndexReq& req) {
            auto* processor = RebuildEdgeIndexProcessor::instance(kvstore_, adminClient_.get());
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ListIndexStatusResp>
        MetaServiceHandler::future_listEdgeIndexStatus(const cpp2::ListIndexStatusReq& req) {
            auto* processor = ListEdgeIndexStatusProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
        */
        folly::Future<cpp2::HBResp>
        MetaServiceHandler::future_heartBeat(const cpp2::HBReq &req)
        {
            // auto* processor = HBProcessor::instance(kvstore_, clusterId_, &heartBeatStat_);
            // RETURN_FUTURE(processor);
        //    std::cout << "heartbeat called" << "\n\n\n";
            folly::Promise<cpp2::HBResp> promise_;
            auto f = promise_.getFuture();
            cpp2::HBResp resp_;

            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;

            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
          //  std::cout << "ip is: " << localMeatHost.first << std::endl;
          //  std::cout << "port is: " << localMeatHost.second << std::endl;
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);

            int64_t lastUpdateTime = nebula::time::WallClock::fastNowInMilliSec();

            resp_.set_last_update_time_in_ms(lastUpdateTime);
            resp_.set_cluster_id(clusterId_);
            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);
            // return resp;
            promise_.setValue(std::move(resp_));
           // std::cout << "heart beat end\n\n";

            return f;
        }

        /*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_createUser(const cpp2::CreateUserReq& req) {
            auto* processor = CreateUserProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_dropUser(const cpp2::DropUserReq& req) {
            auto* processor = DropUserProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_alterUser(const cpp2::AlterUserReq& req) {
            auto* processor = AlterUserProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_grantRole(const cpp2::GrantRoleReq& req) {
            auto* processor = GrantProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_revokeRole(const cpp2::RevokeRoleReq& req) {
            auto* processor = RevokeProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
        */

        folly::Future<cpp2::ListUsersResp>
        MetaServiceHandler::future_listUsers(const cpp2::ListUsersReq &req)
        {
            // auto* processor = ListUsersProcessor::instance(kvstore_);
            // RETURN_FUTURE(processor);
           // std::cout << "ListUsers called\n\n";
            folly::Promise<cpp2::ListUsersResp> promise_;
            auto f = promise_.getFuture();
            cpp2::ListUsersResp resp_;

            decltype(resp_.users) users;
            std::string account = "GOD";
            std::string password = "nebula";
            users.emplace(std::pair<std::string, std::string>(std::move(account), std::move(password)));
            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);
            promise_.setValue(std::move(resp_));

            return f;
        }

        /*
        folly::Future<cpp2::ListRolesResp>
        MetaServiceHandler::future_listRoles(const cpp2::ListRolesReq& req) {
            auto* processor = ListRolesProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_changePassword(const cpp2::ChangePasswordReq& req) {
            auto* processor = ChangePasswordProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ListRolesResp>
        MetaServiceHandler::future_getUserRoles(const cpp2::GetUserRolesReq& req) {
            auto* processor = GetUserRolesProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::BalanceResp>
        MetaServiceHandler::future_balance(const cpp2::BalanceReq& req) {
            auto* processor = BalanceProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_leaderBalance(const cpp2::LeaderBalanceReq& req) {
            auto* processor = LeaderBalanceProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
*/
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_regConfig(const cpp2::RegConfigReq &req) {
        //    auto* processor = RegConfigProcessor::instance(kvstore_);
        //    RETURN_FUTURE(processor);
        //      std::cout<<"regCongig called\n";

            folly::Promise<cpp2::ExecResp> promise_ ;
            auto f = promise_.getFuture();
            cpp2::ExecResp resp_;

              //error-code  始终都是成功
            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);

            //leader ip
            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;

            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);  

            
            promise_.setValue(std::move(resp_));
            return f;  

         //   std::cout<<"regCongig end\n";
             
        }
/*
        folly::Future<cpp2::GetConfigResp>
        MetaServiceHandler::future_getConfig(const cpp2::GetConfigReq &req) {
            auto* processor = GetConfigProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_setConfig(const cpp2::SetConfigReq &req) {
            auto* processor = SetConfigProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
*/
        folly::Future<cpp2::ListConfigsResp>
        MetaServiceHandler::future_listConfigs(const cpp2::ListConfigsReq &req) {
           // auto* processor = ListConfigsProcessor::instance(kvstore_);
           // RETURN_FUTURE(processor);
         //  std::cout<<"ListConfig called\n";
            folly::Promise<cpp2::ListConfigsResp> promise_ ;
            auto f = promise_.getFuture();
            cpp2::ListConfigsResp resp_;


            //error-code  始终都是成功
            resp_.set_code(cpp2::ErrorCode::SUCCEEDED);

            //leader ip
            nebula::cpp2::HostAddr host_addr;
            std::string MetaIp = "127.0.0.1";
            int32_t MetaPort = 9777;

            auto hostAddrRet = nebula::network::NetworkUtils::toHostAddr(MetaIp, MetaPort);
            auto localMeatHost = hostAddrRet.value();
            host_addr.set_ip(localMeatHost.first);
            host_addr.set_port(localMeatHost.second);
            resp_.set_leader(host_addr);  

          // std::cout<<"ListConfig end\n";
           promise_.setValue(std::move(resp_));
           return f;

        }
/*
        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_createSnapshot(const cpp2::CreateSnapshotReq& req) {
            auto* processor = CreateSnapshotProcessor::instance(kvstore_, adminClient_.get());
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ExecResp>
        MetaServiceHandler::future_dropSnapshot(const cpp2::DropSnapshotReq& req) {
            auto* processor = DropSnapshotProcessor::instance(kvstore_, adminClient_.get());
            RETURN_FUTURE(processor);
        }

        folly::Future<cpp2::ListSnapshotsResp>
        MetaServiceHandler::future_listSnapshots(const cpp2::ListSnapshotsReq& req) {
            auto* processor = ListSnapshotsProcessor::instance(kvstore_);
            RETURN_FUTURE(processor);
        }
        */

    } // namespace meta
} // namespace nebula
