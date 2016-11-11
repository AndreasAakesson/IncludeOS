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

#ifndef HW_NIC_HPP
#define HW_NIC_HPP

#include "../net/buffer_store.hpp"
#include "mac_addr.hpp"
#include <net/frame.hpp>
#include <net/inet_common.hpp>

// temporary until full_header on packets (and by that ethernet) is removed
namespace net {
  namespace ip4 {
    class Packet;
  }
}

namespace hw {

  /**
   *  A public interface Network cards
   */
  class Nic {
  public:

    enum class Proto {ETH, IEEE802111};

    virtual Proto proto() const = 0;

    /** Get a readable name. */
    virtual const char* name() const = 0;

    /** A readable name of the type of device @todo: move to a abstract Device? */
    static const char* device_type()
    { return "NIC"; }

    /** The mac address. */
    virtual const MAC_addr& mac() const noexcept = 0;

    virtual uint16_t MTU() const noexcept = 0;

    /** Implemented by the underlying (link) driver */
    virtual net::downstream_spec<net::Frame> create_link_downstream() = 0;
    virtual void set_ip4_upstream(net::upstream_spec<net::ip4::Packet> handler) = 0;
    virtual void set_ip6_upstream(net::upstream handler) = 0;
    virtual void set_arp_upstream(net::upstream handler) = 0;

    /** @note: performance issues with virtual? */
    virtual net::Frame::ptr create_frame() = 0;

    net::BufferStore& bufstore() noexcept
    { return bufstore_; }

    size_t buffers_available()
    { return bufstore_.available(); }

    uint16_t bufsize() const
    { return bufstore_.bufsize(); }

    /** Subscribe to event for when there is more room in the tx queue */
    void on_transmit_queue_available(net::transmit_avail_delg del)
    { transmit_queue_available_event_ = del; }

    virtual size_t transmit_queue_available() = 0;

    virtual size_t receive_queue_waiting() = 0;

    std::string ifname() const {
      return "eth" + std::to_string(N);
    }

    virtual void deactivate() = 0;

  protected:
    /**
     *  Constructor
     *
     *  Constructed by the actual Nic Driver
     */
    Nic(uint32_t bufstore_packets, uint16_t bufsz)
      : bufstore_{ bufstore_packets, bufsz }
    {
      static int id_counter = 0;
      N = id_counter++;
    }

    friend class Devices;

    net::transmit_avail_delg transmit_queue_available_event_ =
      [](auto) { assert(0 && "<NIC> Transmit queue available delegate is not set!"); };

    auto create_buffer(size_t size)
    {
      // get buffer (as buffer + data)
      auto* ptr = (net::Buffer*) bufstore_.get_buffer();
      // place packet at front of buffer
      new (ptr) net::Buffer(bufstore_.bufsize(), size, &bufstore_);
      return net::Buffer::ptr(ptr);
    }

  private:
    net::BufferStore bufstore_;
    int N;

  };

} //< namespace hw

#endif // NIC_HPP
