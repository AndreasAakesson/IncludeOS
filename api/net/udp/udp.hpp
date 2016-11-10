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

#ifndef NET_UDP_HPP
#define NET_UDP_HPP

#include "datagram.hpp"
#include "socket.hpp"
#include <deque>
#include <map>
#include <net/ip4/ip4.hpp>
#include <cstring>

namespace net {

  /** Basic UDP support. @todo Implement UDP sockets.  */
  class UDP {
  public:
    using addr_t      = udp::addr_t;
    using port_t      = udp::port_t;

    using Stack       = IP4::Stack;

    using downstream  = IP4::downstream;
    using upstream    = IP4::upstream;

    using full_header = udp::full_header; // temp

    using sendto_handler = udp::Socket::sendto_handler;

    // write buffer for sendq
    struct WriteBuffer
    {
      WriteBuffer(
                  const uint8_t* data, size_t length, sendto_handler cb,
                  UDP& udp, addr_t LA, port_t LP, addr_t DA, port_t DP);

      int remaining() const {
        return len - offset;
      }
      bool done() const {
        return offset == len;
      }

      size_t packets_needed() const;
      void write();

      // buffer, total length and current write offset
      std::shared_ptr<uint8_t> buf;
      size_t len;
      size_t offset;
      // the callback for when this buffer is written
      sendto_handler callback;
      // the UDP stack
      UDP& udp;

      // port and addr this was being sent from
      addr_t l_addr;
      port_t l_port;
      // destination address and port
      port_t d_port;
      addr_t d_addr;
    };

    ////////////////////////////////////////////

    addr_t local_ip() const
    { return stack_.ip_addr(); }

    /** Input from network layer */
    void receive(ip4::Packet::ptr);

    /** Delegate output to network layer */
    void set_network_downstream(downstream del)
    { network_downstream_ = del; }

    /** Send UDP datagram from source ip/port to destination ip/port.

        @param sip   Local IP-address
        @param sport Local port
        @param dip   Remote IP-address
        @param dport Remote port   */
    void transmit(udp::Datagram::ptr udp);

    //! @param port local port
    udp::Socket& bind(port_t port);

    //! returns a new UDP socket bound to a random port
    udp::Socket& bind();

    bool is_bound(port_t port);

    /** Close a port **/
    void close(port_t port);

    //! construct this UDP module with @inet
    UDP(Stack& inet);

    Stack& stack()
    {
      return stack_;
    }

    // send as much as possible from sendq
    void flush();

    // create and transmit @num packets from sendq
    void process_sendq(size_t num);

    inline constexpr uint16_t max_datagram_size() noexcept {
      return stack().ip_obj().MDDS() - sizeof(udp::Header);
    }

    class Port_in_use_exception : public std::exception {
    public:
      Port_in_use_exception(UDP::port_t p)
        : port_(p) {}
      virtual const char* what() const noexcept {
        return "UDP port allready in use";
      }

      UDP::port_t port(){
        return port_;
      }

    private:
      UDP::port_t port_;
    };

  private:
    Stack&      stack_;
    downstream  network_downstream_;
    std::map<port_t, udp::Socket> ports_;
    port_t      current_port_ {1024};

    // the async send queue
    std::deque<WriteBuffer> sendq;
    friend class udp::Socket;
  }; //< class UDP

} //< namespace net

#endif
