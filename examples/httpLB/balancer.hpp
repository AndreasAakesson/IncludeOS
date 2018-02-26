
#pragma once
#ifndef HTTP_LB_BALANCER_HPP
#define HTTP_LB_BALANCER_HPP

#include <net/inet>
#include <net/ip4/ip4.hpp>
#include <http>
#include <list>

namespace http_lb {

  class Balancer;

  struct Transaction {
    using Index = size_t;
    Index     index;
    Balancer& balancer;
    http::Response_writer_ptr rw;

    Transaction(Balancer& bal, http::Response_writer_ptr r, Index idx);

    void handle_resp(http::Error, http::Response_ptr, http::Connection&);
    void finish();
  };

  struct Node {
    std::string     key;
    const uri::URI  host;

    Node(std::string name, const std::string& uri)
      : key{std::move(name)}, host{uri}
    {
      Ensures(host.is_valid());
    }

    std::string to_string() const
    { return "[" + key + "]: " + host.to_string(); }
  };

  struct Input_result {
    http::Request_ptr         req;
    http::Response_writer_ptr rw;

    Input_result(http::Request_ptr req, http::Response_writer_ptr rw)
      : req{std::move(req)}, rw{std::move(rw)}
    {}

    Input_result()
      : Input_result{nullptr, nullptr}
    {}

    bool not_null() const
    { return req != nullptr and rw != nullptr; }

    operator bool () const
    { return not_null(); }
  };

  struct Output_result {
    http::Response_ptr resp;

    Output_result(http::Response_ptr res)
      : resp{std::move(res)}
    {}

    Output_result()
      : Output_result{nullptr}
    {}

    bool not_null() const
    { return resp != nullptr; }

    operator bool () const
    { return not_null(); }
  };

  using Input_filter  = delegate<Input_result(http::Request_ptr, http::Response_writer_ptr, Balancer&)>;
  using Input_chain   = std::list<Input_filter>;

  using Output_filter = delegate<Output_result(http::Response_ptr)>;
  using Output_chain  = std::list<Output_filter>;

  // Balance in all things
  class Balancer {
  public:
    using Stack         = net::Inet<net::IP4>;
    using Transactions  = std::vector<std::unique_ptr<Transaction>>;
    using Nodes         = std::vector<Node>;
    using Node_iter     = Nodes::iterator;
    using Indexes       = std::vector<size_t>;
    friend struct Transaction;

  public:
    Input_chain   input_chain;
    Output_chain  output_chain;

    Balancer(Stack& outside, const uint16_t port, Stack& inside);

    Node_iter find_node(const std::string& key);

    void forward(http::URI url, http::Request_ptr req, http::Response_writer_ptr rw);

    void redirect(http::status_t code, http::URI url, http::Response_writer_ptr rw);

    Nodes& pool()
    { return nodes; }

  private:
    http::Server server;
    http::Client client;

    Transactions transactions;
    Indexes      free_index;

    Nodes        nodes;

    uint64_t     stats_transactions = 0;

    void recv_request(http::Request_ptr req, http::Response_writer_ptr rw);

    Input_result process_input_chain(http::Request_ptr req, http::Response_writer_ptr rw);
    Output_result process_output_chain(http::Response_ptr resp);

    Transaction& create_transaction(http::Response_writer_ptr rw);
    void remove_trans(Transaction&);

    const Node& get_node() const;

  }; // < class Balancer

} // < http_lb

#endif
