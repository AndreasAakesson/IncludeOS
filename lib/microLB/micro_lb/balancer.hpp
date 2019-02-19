#pragma once
#include "nodes.hpp"

namespace net {
  class Inet;
}
namespace microLB
{
  typedef net::Inet netstack_t;

  struct Waiting {
    Waiting(net::Stream_ptr);
    Waiting(liu::Restore&, DeserializationHelper&);
    void serialize(liu::Storage&);

    net::Stream_ptr conn;
    int total = 0;
  };

  struct Balancer {
    Balancer(bool active_check);
    ~Balancer();

    static Balancer* from_config();

    // Frontend/Client-side of the load balancer
    void open_for_tcp(netstack_t& interface, uint16_t port);
    void open_for_s2n(netstack_t& interface, uint16_t port, const std::string& cert, const std::string& key);
    void open_for_ossl(netstack_t& interface, uint16_t port, const std::string& cert, const std::string& key);
    // Backend/Application side of the load balancer
    static node_connect_function_t connect_with_tcp(netstack_t& interface, net::Socket);
    // Setup and automatic resume (if applicable)
    // NOTE: Be sure to have configured it properly BEFORE calling this
    void init_liveupdate();

    int  wait_queue() const;
    int  connect_throws() const;

    // add a client stream to the load balancer
    // NOTE: the stream must be connected prior to calling this function
    void incoming(net::Stream_ptr);

    void serialize(liu::Storage&, const liu::buffer_t*);
    void resume_callback(liu::Restore&);

    Nodes nodes;
    pool_signal_t get_pool_signal();
    DeserializationHelper de_helper;

  private:
    void handle_connections();
    void handle_queue();
    void deserialize(liu::Restore&);
    std::vector<net::Socket> parse_node_confg();

    std::deque<Waiting> queue;
    int throw_retry_timer = -1;
    int throw_counter = 0;
    // TLS stuff (when enabled)
    void* tls_context = nullptr;
    delegate<void()> tls_free = nullptr;
  };

}
