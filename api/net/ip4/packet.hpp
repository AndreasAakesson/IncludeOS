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

#ifndef NET_IP4_PACKET_HPP
#define NET_IP4_PACKET_HPP

#include <cassert>

#include <net/util.hpp>
#include <net/frame.hpp>

#include "header.hpp"

namespace net {
  namespace ip4 {

    class Packet : public net::Frame
    {
    public:
      using ptr = bufptr<Packet>;

      static constexpr size_t DEFAULT_TTL {64};

      const ip4::Addr& src() const noexcept
      { return ip_header().saddr; }

      void set_src(const ip4::Addr& addr) noexcept
      { ip_header().saddr = addr; }

      const ip4::Addr& dst() const noexcept
      { return ip_header().daddr; }

      void set_dst(const ip4::Addr& addr) noexcept
      { ip_header().daddr = addr; }

      void set_protocol(const Proto p) noexcept
      { ip_header().protocol = static_cast<uint8_t>(p); }

      uint8_t protocol() const noexcept
      { return ip_header().protocol; }

      uint16_t ip_segment_length() const noexcept
      { return ntohs(ip_header().tot_len); }

      uint8_t ip_header_length() const noexcept
      { return (ip_header().version_ihl & 0xf) * 4; }

      uint8_t ip_full_header_length() const noexcept
      { return sizeof(Full_header) - sizeof(Header) + ip_header_length(); }

      uint16_t ip_data_length() const noexcept
      { return ip_segment_length() - ip_header_length(); }

      void set_ip_data_length(uint16_t length) {
        set_size(ip_full_header_length() + length);
        set_segment_length();
      }

      /** Last modifications before transmission */
      void make_flight_ready() noexcept {
        assert( ip_header().protocol );
        set_segment_length();
        set_ip4_checksum();
      }

      void init() noexcept {
        ip_header().version_ihl    = 0x45;
        ip_header().tos            = 0;
        ip_header().id             = 0;
        ip_header().frag_off_flags = 0;
        ip_header().ttl            = DEFAULT_TTL;
        set_size(ip_full_header_length());
      }

      const Header& ip_header() const noexcept
      { return (reinterpret_cast<Full_header*>(buffer()))->ip_hdr; }

      Header& ip_header() noexcept
      { return (reinterpret_cast<Full_header*>(buffer()))->ip_hdr; }

      void set_ip4_checksum() noexcept {
        auto& hdr = ip_header();
        hdr.check = 0;
        hdr.check = net::checksum(&hdr, sizeof(Header));
      }

    protected:
      char* ip_data() const
      { return (char*) (buffer() + ip_full_header_length()); }

      /**
       *  Set IP4 header length
       *
       *  Inferred from packet size and linklayer header size
       */
      void set_segment_length() noexcept
      { ip_header().tot_len = htons(size() - sizeof(LinkLayer::header)); }

    private:
      friend net::Buffer;

      void upstream()
      { set_payload(payload() + ip_header_length()); /*printf("<ip4::Packet> Payload += %u\n", ip_header_length());*/ }

    }; //< class Packet

  } // < namespace ip4
} //< namespace net

#endif //< IP4_PACKET_IP4_HPP
