// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.


#ifndef  BRPC_POLICY_PPPOKER_NAMING_SERVICE
#define  BRPC_POLICY_PPPOKER_NAMING_SERVICE

#include "brpc/naming_service.h"
#include <map>
#include <vector>

using namespace std;

typedef struct _ServiceInfo
{   
       _ServiceInfo ()
       {   
           version = 0;
       }
       vector<brpc::ServerNode>  services;
       int                 version;
}ServiceInfo;

class  PPPokerService
{
       public:
       static PPPokerService* get_instance()
       {
           static PPPokerService m_instance;
           return &m_instance;
       }

       ServiceInfo  get_service_info(string service_name)
       {
           ServiceInfo info;
           //lock
           if(m_service_nodes.find(service_name) != m_service_nodes.end())
           {
               info = m_service_nodes[service_name];
           }
           //unlock
           return info;
       }

       void set_service_info(string service_name, vector<brpc::ServerNode>&  services)
       {
           //lock
           if(m_service_nodes.find(service_name) != m_service_nodes.end())
           {
               m_service_nodes[service_name].services = services;
               m_service_nodes[service_name].version++;
           }
           else
           {
               ServiceInfo info;
               info.version = 1;
               info.services = services;
               m_service_nodes[service_name] = info;
           }

           //unlock
       }

       private:
       PPPokerService(){}
       static PPPokerService *m_service;
       map<string, ServiceInfo>  m_service_nodes;
};

namespace brpc {
namespace policy {

class PPPokerNamingService : public NamingService {
private:
    int RunNamingService(const char* service_name,
                         NamingServiceActions* actions) override;

    int GetServers(const char *service_name,
                   std::vector<ServerNode>* servers);

    void Describe(std::ostream& os, const DescribeOptions& options) const override;

    NamingService* New() const override;
    
    void Destroy() override;
    
    ServiceInfo  m_service;
};

}  // namespace policy
} // namespace brpc


#endif  //BRPC_POLICY_LIST_NAMING_SERVICE
