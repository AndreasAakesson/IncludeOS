
#pragma once
#include <net/stream.hpp>

namespace liu {
  struct Storage;
}
namespace microLB
{
  struct Nodes;
  struct Session {
    Session(Nodes&, int idx, net::Stream_ptr in, net::Stream_ptr out);
    bool is_alive() const;
    void serialize(liu::Storage&);

    Nodes&     parent;
    const int  self;
    net::Stream_ptr incoming;
    net::Stream_ptr outgoing;

    void flush_incoming();
    void flush_outgoing();
  };
}
