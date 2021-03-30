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


#include <stdlib.h>                                   // strtol
#include <string>                                     // std::string
#include "butil/string_splitter.h"                     // StringSplitter
#include "brpc/log.h"
#include "brpc/policy/pppoker_naming_service.h"
#include "bthread/bthread.h"


namespace brpc {
namespace policy {

int PPPokerNamingService::GetServers(const char *service_name,
                                  std::vector<ServerNode>* servers) {
    servers->clear();
    // Sort/unique the inserted vector is faster, but may have a different order
    // of addresses from the file. To make assertions in tests easier, we use
    // set to de-duplicate and keep the order.

    if (!service_name) {
        LOG(FATAL) << "Param[service_name] is NULL";
        return -1;
    }

    auto service_info = PPPokerService::get_instance()->get_service_info(service_name);
    
    if(service_info.version > m_service.version)
    {
        m_service = service_info;
        servers->assign(m_service.services.begin(), m_service.services.end());
    }
    RPC_VLOG << "Got " << servers->size()
             << (servers->size() > 1 ? " servers" : " server");
    return 0;
}

int PPPokerNamingService::RunNamingService(const char* service_name,
                                        NamingServiceActions* actions) {
    std::vector<ServerNode> servers;
    const int rc = GetServers(service_name, &servers);
    if (rc != 0) {
        servers.clear();
    }
    actions->ResetServers(servers);
    for (;;) {
        servers.clear();
        const int rc = GetServers(service_name, &servers);
        if (rc != 0) {
            return rc;
        }

        if(servers.size() > 0)
        {
            actions->ResetServers(servers);
        }
        
        if (bthread_usleep(100000L) < 0) {
            if (errno == ESTOP) {
                return 0;
            }
            PLOG(ERROR) << "Fail to sleep";
            return -1;
        }

    }
}

void PPPokerNamingService::Describe(
    std::ostream& os, const DescribeOptions&) const {
    os << "list";
    return;
}

NamingService* PPPokerNamingService::New() const {
    return new PPPokerNamingService;
}

void PPPokerNamingService::Destroy() {
    delete this;
}

}  // namespace policy
} // namespace brpc
