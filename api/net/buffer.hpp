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

#ifndef NET_BUFFER_HPP
#define NET_BUFFER_HPP

#include "buffer_store.hpp"
#include "ip4/addr.hpp"
#include <delegate>
#include <cassert>

namespace net
{
  /* Temp, remove */
  class Buffer;
  using Packet_ptr = std::unique_ptr<Buffer>;

  class Buffer
  {
  public:
    template <typename T>
    using bufptr  = std::unique_ptr<T, std::default_delete<Buffer>>;
    using ptr     = bufptr<Buffer>;
    /**
     *  Construct, using existing buffer.
     *
     *  @param capacity: Size of the buffer
     *  @param len: Length of data in the buffer
     *
     *  @WARNING: There are two adjacent parameters of the same type, violating CG I.24.
     */
    Buffer(
        uint16_t cap,
        uint16_t len,
        BufferStore* bs) noexcept
    : capacity_ (cap),
      size_     (len),
      bufstore  (bs)
    {}

    virtual ~Buffer()
    {
      if (bufstore)
          bufstore->release(this);
      else
          delete[] (uint8_t*) this;
    }

    /** Get the buffer */
    BufferStore::buffer_t buffer() const noexcept
    { return (BufferStore::buffer_t) buf_; }

    /** Get the network buffer length - i.e. the number of populated bytes  */
    uint16_t size() const noexcept
    { return size_; }

    /** Get the size of the buffer. This is >= len(), usually MTU-size */
    uint16_t capacity() const noexcept
    { return capacity_; }

    void set_size(uint16_t new_size) noexcept {
      assert((size_ = new_size) <= capacity_);
    }

    /** next-hop ipv4 address for IP routing */
    void next_hop(ip4::Addr ip) noexcept {
      next_hop4_ = ip;
    }
    auto next_hop() const noexcept {
      return next_hop4_;
    }

    /* Add a packet to this packet chain.  */
    void chain(Buffer::ptr p) noexcept {
      if (!chain_) {
        chain_ = std::move(p);
        last_ = chain_.get();
      } else {
        auto* ptr = p.get();
        last_->chain(std::move(p));
        last_ = ptr->last_in_chain() ? ptr->last_in_chain() : ptr;
        assert(last_);
      }
    }

    /* Get the last packet in the chain */
    Buffer* last_in_chain() noexcept
    { return last_; }

    /* Get the tail, i.e. chain minus the first element */
    Buffer* tail() noexcept
    { return chain_.get(); }

    /* Get the tail, and detach it from the head (for FIFO) */
    Buffer::ptr detach_tail() noexcept
    { return std::move(chain_); }

    // override delete to do nothing
    static void operator delete (void*) {}

    /**
     * @brief      Moves a network unit upstream, moving the payload and converts to another type of unit.
     *
     * @param[in]  base  A unique_ptr to unit to be converted from
     *
     * @tparam     Derived    What Base should convert to
     * @tparam     Base       Unit to be downcasted
     * @tparam     Parent     If sibling, first upcast before downcast (defaults to Base)
     *
     * @return     A unique_ptr of Derived type
     */
    template <class Derived, class Base, class Parent = Base>
    static auto static_move_upstream(bufptr<Base>&& base)
    {
      static_assert(std::is_base_of<Buffer, Base>::value, "Moving something else than a Buffer.");
      static_assert(std::is_base_of<Parent, Derived>::value, "Derived network packet does not inherit base.");
      // Move payload upstream
      base->upstream();
      auto* d = static_cast<Derived*>((Parent*)(base.release()));
      return bufptr<Derived>(d);
    }

  protected:
    /**
     *  For a UDPv6 packet, the payload location is the start of
     *  the UDPv6 header, and so on
     */
    void set_payload(BufferStore::buffer_t location) noexcept
    { payload_ = location; }

    /** Get the payload of the packet */
    BufferStore::buffer_t payload() const noexcept
    { return payload_; }


  private:
    Buffer::ptr chain_ {nullptr};
    Buffer*    last_  {nullptr};

    /** Default constructor Deleted. See Buffer(Buffer&). */
    Buffer() = delete;

    /**
     *  Delete copy and move because we want Buffers and buffers to be 1 to 1
     *
     *  (Well, we really deleted this to avoid accidental copying)
     *
     *  The idea is to use Buffer_ptr (i.e. shared_ptr<Buffer>) for passing packets.
     *
     *  @todo Add an explicit way to copy packets.
     */
    Buffer(Buffer&) = delete;
    Buffer(Buffer&&) = delete;


    /** Delete copy and move assignment operators. See Buffer(Buffer&). */
    Buffer& operator=(Buffer) = delete;
    Buffer operator=(Buffer&&) = delete;

    uint16_t              capacity_;
    uint16_t              size_;
    BufferStore*          bufstore;
    ip4::Addr             next_hop4_;
    BufferStore::buffer_t payload_ {nullptr};
    BufferStore::buffer_t buf_[0];

    /**
     * @brief      Moves the payload "upstream" (forward ->)
     * In this case, the buffer starts where the link header begins.
     */
    void upstream()
    { payload_ = buffer(); }
  }; //< class Buffer

} //< namespace net

#endif
