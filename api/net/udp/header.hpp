// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2016 Oslo and Akershus University College of Applied Sciences
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

#include <net/ip4/addr.hpp> // ip4::Addr
#include <net/ip4/ip4.hpp> // temp, IP4::full_header

namespace net {
  namespace udp {

    using addr_t = ip4::Addr;
    using port_t = uint16_t;

    /** UDP header */
    struct Header {
      port_t   sport;
      port_t   dport;
      uint16_t length;
      uint16_t checksum;
    };

    /** Full UDP Header with all sub-headers */
    // temp
    struct full_header {
      IP4::full_header full_hdr;
      Header           udp_hdr;
    }__attribute__((packed));

  }
}
