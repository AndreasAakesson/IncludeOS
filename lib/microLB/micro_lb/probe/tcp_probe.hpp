
#pragma once
#include "probe.hpp"
#include <net/tcp/tcp.hpp>

namespace microLB
{
  class TCP_probe : public Probe
  {
  public:
    TCP_probe(net::TCP& tcp, net::Socket endpoint, Interval interval)
      : Probe{interval}, tcp{tcp}, endpoint{std::move(endpoint)}
    {}

  private:
    net::TCP& tcp;
    const net::Socket endpoint;

    void do_probe() override
    {
      tcp.connect(endpoint, {this, &TCP_probe::handle_connect});
    }

    void handle_connect(net::tcp::Connection_ptr conn)
    {
      if(conn != nullptr)
      {
        result(true);
        conn->abort();
      }
      else
      {
        result(false);
      }
    }
  };
}
