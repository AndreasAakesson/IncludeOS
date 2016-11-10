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

#include <net/udp/socket.hpp>
#include <net/udp/udp.hpp>
#include <memory>

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

namespace net
{
namespace udp
{
  Socket::Socket(UDP& udp_instance, port_t port)
    : udp_(udp_instance), l_port(port)
  {}

  addr_t Socket::local_addr() const
  {
    return udp_.local_ip();
  }

  void Socket::packet_init(
      Datagram& d,
      addr_t srcIP,
      addr_t destIP,
      port_t port,
      uint16_t length)
  {
    d.init();
    d.header().sport = htons(this->l_port);
    d.header().dport = htons(port);
    d.set_src(srcIP);
    d.set_dst(destIP);
    d.set_length(length);

    assert(d.data_length() == length);
  }

  void Socket::internal_read(Datagram::ptr udp)
  {
    on_read_handler(
        udp->src(), udp->src_port(), udp->data(), udp->data_length());
  }

  void Socket::sendto(
     addr_t destIP,
     port_t port,
     const void* buffer,
     size_t len,
     sendto_handler cb)
  {
    if (unlikely(len == 0)) return;
    udp_.sendq.emplace_back(
       (const uint8_t*) buffer, len, cb, this->udp_,
       local_addr(), this->l_port, destIP, port);

    // UDP packets are meant to be sent immediately, so try flushing
    udp_.flush();
  }
  void Socket::bcast(
      addr_t srcIP,
      port_t port,
      const void* buffer,
      size_t len,
      sendto_handler cb)
  {
    if (unlikely(len == 0)) return;
    udp_.sendq.emplace_back(
         (const uint8_t*) buffer, len, cb, this->udp_,
         srcIP, this->l_port, IP4::ADDR_BCAST, port);

    // UDP packets are meant to be sent immediately, so try flushing
    udp_.flush();
  }

}
}
