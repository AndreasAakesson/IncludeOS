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

#pragma once

#ifndef NET_UDP_DATAGRAM_HPP
#define NET_UDP_DATAGRAM_HPP

#include <net/ip4/packet.hpp>
#include "header.hpp"

namespace net {
 namespace udp {

  class Datagram : public ip4::Packet
  {
  public:
    using ptr = bufptr<Datagram>;

    Header& header() const
    {
      return ((full_header*) buffer())->udp_hdr;
    }

    static const size_t HEADERS_SIZE = sizeof(full_header);

    //! initializes to a default, empty UDP packet, given
    //! a valid MTU-sized buffer
    void init()
    {
      ip4::Packet::init();
      // source and destination ports
      header().sport = 0;
      header().dport = 0;
      // set zero length
      set_length(0);
      // zero the optional checksum
      header().checksum = 0;
      // set UDP payload location (!?)
      set_payload(buffer() + sizeof(full_header));
      set_protocol(ip4::Proto::IP4_UDP);
    }

    port_t src_port() const
    {
      return htons(header().sport);
    }
    port_t dst_port() const
    {
      return htons(header().dport);
    }

    uint16_t length() const
    {
      return ntohs(header().length);
    }
    uint16_t data_length() const
    {
      return length() - sizeof(Header);
    }
    inline char* data()
    {
      return (char*) (buffer() + sizeof(full_header));
    }

    // sets the correct length for all the protocols up to IP4
    void set_length(uint16_t newlen)
    {
      // new total UDPv6 payload length
      header().length = htons(sizeof(Header) + newlen);

      // new total packet length
      set_size( sizeof(full_header) + newlen );
    }

    // generates a new checksum and sets it for this UDP packet
    uint16_t gen_checksum();

    //! assuming the packet has been properly initialized,
    //! this will fill bytes from @buffer into this packets buffer,
    //! then return the number of bytes written. buffer is unmodified
    uint32_t fill(const std::string& buffer)
    {
      uint32_t rem = capacity();
      uint32_t total = (buffer.size() < rem) ? buffer.size() : rem;
      // copy from buffer to packet buffer
      memcpy(data() + data_length(), buffer.data(), total);
      // set new packet length
      set_length(data_length() + total);
      return total;
    }
  };
 }

}

#endif
