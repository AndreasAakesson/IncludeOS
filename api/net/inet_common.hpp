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

/** Common utilities for internetworking */

#ifndef NET_INET_COMMON_HPP
#define NET_INET_COMMON_HPP

#include <delegate>
#include <net/buffer.hpp>

namespace net {
  class Ethernet;

  using LinkLayer = Ethernet;

  // Downstream / upstream delegates
  template<typename T = Buffer>
  using upstream      = delegate<void(std::unique_ptr<T, std::default_delete<Buffer>>)>;

  template<typename T = Buffer>
  using downstream    = upstream<T>;

  // Delegate for signalling available buffers in device transmit queue
  using transmit_avail_delg = delegate<void(size_t)>;

  // Compute the internet checksum for the buffer / buffer part provided
  uint16_t checksum(void* data, size_t len) noexcept;

  // View a packet differently based on context
  template <typename T, typename Packet>
  inline auto view_packet_as(Packet packet) noexcept {
    return std::static_pointer_cast<T>(packet);
  }


  template<typename Derived, typename Base>
  auto static_unique_ptr_cast( std::unique_ptr<Base>&& p )
  {
      auto* d = static_cast<Derived *>(p.release());
      return std::unique_ptr<Derived>(d);
  }

} //< namespace net

#endif //< NET_INET_COMMON_HPP
