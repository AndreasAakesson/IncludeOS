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

#pragma once
#ifndef NET_IP4_ARP_HPP
#define NET_IP4_ARP_HPP

#include <kernel/os.hpp> // OS::uptime()
#include <map>
#include <delegate>
#include "ip4.hpp"

namespace net {

  class PacketArp;

  /** ARP manager, including an ARP-Cache. */
  class Arp {
  public:
    using Stack   = IP4::Stack;
    /** Network/Internet protocol */
    using Network = IP4;
    /** Link protocol */
    using Link    = Ethernet;

  private:
    /** ARP cache expires after cache_exp_t_ seconds */
    static constexpr uint16_t cache_exp_t_ {60 * 60 * 12};

    /** Cache entries are just MAC's and timestamps */
    struct cache_entry {
      Link::addr  mac_;
      uint64_t    timestamp_;

      /** Map needs empty constructor (we have no emplace yet) */
      cache_entry() noexcept = default;

      cache_entry(Link::addr mac) noexcept
      : mac_(mac), timestamp_(OS::uptime()) {}

      cache_entry(const cache_entry& cpy) noexcept
      : mac_(cpy.mac_), timestamp_(cpy.timestamp_) {}

      void update() noexcept { timestamp_ = OS::uptime(); }
    }; //< struct cache_entry

    using Cache       = std::map<Network::addr, cache_entry>;
    using PacketQueue = std::map<Network::addr, Network::Packet::ptr>;
  public:
    /**
     *  You can assign your own ARP-resolution delegate
     *
     *  We're doing this to keep the Hårek Haugerud mapping (HH_MAP)
     */
    using Arp_resolver = delegate<void(Network::Packet::ptr packet)>;

    enum Opcode { H_request = 0x100, H_reply = 0x200 };

    /** Arp opcodes (Big-endian) */
    static constexpr uint16_t H_htype_eth {0x0100};
    static constexpr uint16_t H_ptype_ip4 {0x0008};
    static constexpr uint16_t H_hlen_plen {0x0406};

    /** Constructor */
    explicit Arp(Stack&) noexcept;

    struct __attribute__((packed)) header {
      Link::header     ethhdr;    // Ethernet header
      uint16_t         htype;     // Hardware type
      uint16_t         ptype;     // Protocol type
      uint16_t         hlen_plen; // Protocol address length
      uint16_t         opcode;    // Opcode
      Link::addr       shwaddr;   // Source mac
      Network::addr    sipaddr;   // Source ip
      Link::addr       dhwaddr;   // Target mac
      Network::addr    dipaddr;   // Target ip
    };

    /** Handle incoming ARP packet. */
    void receive(net::Frame::ptr);

    /** Roll your own arp-resolution system. */
    void set_resolver(Arp_resolver ar)
    { arp_resolver_ = ar; }

    enum Resolver_name { DEFAULT, HH_MAP };

    void set_resolver(Resolver_name nm) {
      // @TODO: Add HÅREK-mapping here
      switch (nm) {
      case HH_MAP:
        arp_resolver_ = {this, &Arp::hh_map};
        break;
      default:
        arp_resolver_ = {this, &Arp::arp_resolve};
      }
    }

    /** Delegate link-layer output. */
    void set_link_downstream(downstream<net::Frame> link)
    { link_downstream_ = link; }

    /** Downstream transmission. */
    void transmit(ip4::Packet::ptr);

  private:

    /** Stats */
    uint32_t& requests_rx_;
    uint32_t& requests_tx_;
    uint32_t& replies_rx_;
    uint32_t& replies_tx_;

    Stack& inet_;

    /** Needs to know which mac address to put in header->swhaddr */
    Link::addr mac_;

    /** Outbound data goes through here */
    downstream<net::Frame> link_downstream_;

    /** The ARP cache */
    Cache cache_;

    /** Cache IP resolution. */
    void cache(Network::addr, Link::addr);

    /** Check if an IP is cached and not expired */
    bool is_valid_cached(Network::addr);

    /** ARP resolution. */
    Link::addr resolve(Network::addr);

    void arp_respond(header* hdr_in);

    // two different ARP resolvers
    void arp_resolve(Network::Packet::ptr);
    void hh_map(Network::Packet::ptr);

    Arp_resolver arp_resolver_ = {this, &Arp::arp_resolve};

    PacketQueue waiting_packets_;

    /** Add a packet to waiting queue, to be sent when IP is resolved */
    void await_resolution(Network::Packet::ptr, Network::addr);

    /** Create a default initialized ARP-packet */
    Packet_ptr create_packet();
  }; //< class Arp

} //< namespace net

#endif //< NET_ARP_HPP
