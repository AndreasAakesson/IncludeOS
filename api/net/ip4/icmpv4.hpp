// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
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

#ifndef NET_IP4_ICMPv4_HPP
#define NET_IP4_ICMPv4_HPP

#include "../inet.hpp"
#include "ip4.hpp"

namespace net {

  void icmp_default_out(ip4::Packet::ptr);

  class ICMPv4 {
  public:
    using Stack = IP4::Stack;

    // Initialize
    ICMPv4(Stack&);

    // Known ICMP types
    enum icmp_types { ICMP_ECHO_REPLY, ICMP_ECHO = 8 };

    struct icmp_header {
      uint8_t  type;
      uint8_t  code;
      uint16_t checksum;
      uint16_t identifier;
      uint16_t sequence;
      uint8_t  payload[0];
    }__attribute__((packed));

    struct full_header {
      LinkLayer::header link_hdr;
      IP4::ip_header    ip_hdr;
      icmp_header       icmp_hdr;
    }__attribute__((packed));

    // Input from network layer
    void receive(ip4::Packet::ptr);

    // Delegate output to network layer
    void set_network_downstream(downstream<ip4::Packet> s)
    { network_downstream_ = s;  };

  private:
    Stack& inet_;
    downstream<ip4::Packet> network_downstream_ {icmp_default_out};

    void ping_reply(full_header* full_hdr, uint16_t size);
  }; //< class ICMPv4
} //< namespace net

#endif //< NET_IP4_ICMPv4_HPP
