
#pragma once
#ifndef NET_ETHERNET_FRAME_HPP
#define NET_ETHERNET_FRAME_HPP

#include "../frame.hpp"
#include "header.hpp"

namespace net {
  namespace ethernet {

    class Frame : public net::Frame {
    public:
      using ptr = std::unique_ptr<Frame>;

      Header& header() const
      { return *reinterpret_cast<Header*>(payload()); }

      constexpr uint16_t header_len() const
      { return sizeof(Header); /* Ethernet header is constant length */ }

      uint8_t* data() const
      { return payload() + header_len(); }

      void upstream()
      { set_payload(payload() + header_len()); }

      void downstream()
      { set_payload(payload() - header_len()); }

    };

  } // < namespace ethernet
} // < namespace net

#endif
