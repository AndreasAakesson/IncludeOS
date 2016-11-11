

#pragma once
#ifndef NET_LINK_LAYER_HPP
#define NET_LINK_LAYER_HPP

#include <hw/nic.hpp>

namespace net {

template <class T>
class Link_layer : public hw::Nic {
public:
  using Protocol    = T;

public:
  explicit Link_layer(Protocol&& protocol, uint32_t bufstore_sz, uint16_t bufsz);

  downstream_spec<net::Frame> create_link_downstream() override
  { return {link_, &Protocol::transmit}; }

  void set_ip4_upstream(upstream_spec<net::ip4::Packet> handler) override
  { link_.set_ip4_upstream(handler); }

  void set_ip6_upstream(upstream handler) override
  { link_.set_ip6_upstream(handler); }

  void set_arp_upstream(upstream handler) override
  { link_.set_arp_upstream(handler); }

  hw::Nic::Proto proto() const override
  { return Protocol::proto(); }

  net::Frame::ptr create_frame() override
  { return link_.create_frame(create_buffer(0)); }

protected:
  /** Called by the underlying physical driver inheriting the Link_layer */
  void receive(typename Protocol::Frame::ptr frame)
  { link_.receive(std::move(frame)); }

private:
  Protocol link_;
};

template <class Protocol>
Link_layer<Protocol>::Link_layer(Protocol&& protocol, uint32_t bufstore_packets, uint16_t bufsz)
  : hw::Nic(bufstore_packets, bufsz + Protocol::header_size()),
    link_{std::forward<Protocol>(protocol)}
{
}

} // < namespace net

#endif
