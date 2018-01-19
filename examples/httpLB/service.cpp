// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2017 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "balancer.hpp"
#include <os>
#include <net/super_stack.hpp>

using namespace http_lb;
Input_filter input_filter =
  [](http::Request_ptr req, http::Response_writer_ptr rw, Balancer& bal)
  ->Input_result
{
  printf("input_filter(req): %s %s\n",
    http::method::str(req->method()).to_string().c_str(),
    req->uri().to_string().c_str());


  // custom forward
  if(req->uri() == "/admin")
  {
    auto node = bal.find_node("4");
    Expects(node != bal.pool().end());
    bal.forward(node->host, std::move(req), std::move(rw));
    return {};
  }

  return {std::move(req), std::move(rw)};
};


void Service::start()
{
  auto& eth0 = net::Super_stack::get<net::IP4>(0);
  auto& eth1 = net::Super_stack::get<net::IP4>(1);

  static Balancer balancer{eth0, 8080, eth1};

  auto& pool = balancer.pool();
  pool.emplace_back("1","http://10.0.0.1:6001");
  pool.emplace_back("2","http://10.0.0.1:6002");
  pool.emplace_back("3","http://10.0.0.1:6003");
  pool.emplace_back("4","http://10.0.0.1:6004");
  pool.emplace_back("5","http://10.0.0.1:6005");

  balancer.input_chain.push_back(input_filter);
}
