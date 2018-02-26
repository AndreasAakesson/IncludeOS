#include "balancer.hpp"

#define HTTP_BALANCER_DEBUG 1
#ifdef HTTP_BALANCER_DEBUG
#define PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define PRINT(fmt, ...) /* fmt */
#endif

namespace http_lb {

  Transaction::Transaction(Balancer& bal, http::Response_writer_ptr r, Index idx)
    : index{idx},
      balancer{bal},
      rw{std::move(r)}
  {}

  void Transaction::handle_resp(http::Error err, http::Response_ptr resp, http::Connection&)
  {
    if(not err)
    {
      auto res = balancer.process_output_chain(std::move(resp));
      if(res)
      {
        rw->set_response(std::move(res.resp));
        rw->write();
      }
    }
    else
    {
      rw->write_header(http::Service_Unavailable);
      // Send custom 404/500 body here
      rw->write("500");
    }
    finish();
  }

  void Transaction::finish()
  {
    balancer.remove_trans(*this);
  }

  Balancer::Balancer(Stack& outside, const uint16_t port, Stack& inside)
    : server{outside.tcp(), {this, &Balancer::recv_request}},
      client{inside.tcp()}
  {
    server.listen(port);
  }

  Balancer::Node_iter Balancer::find_node(const std::string& key)
  {
    return std::find_if(nodes.begin(), nodes.end(),
      [&key](const Node& n)->bool
    {
      return n.key == key;
    });
  }

  void Balancer::forward(http::URI url, http::Request_ptr req, http::Response_writer_ptr rw)
  {
    // create a transaction
    auto& trans = create_transaction(std::move(rw));
    client.send(std::move(req), url, {trans, &Transaction::handle_resp});
  }

  void Balancer::redirect(http::status_t code, http::URI url, http::Response_writer_ptr rw)
  {
    rw->header().set_field("Location", url);
    rw->write_header(code);
    rw->end();
  }

  void Balancer::recv_request(http::Request_ptr req, http::Response_writer_ptr rw)
  {
    // run the input chain
    auto res = process_input_chain(std::move(req), std::move(rw));
    // don't do regular balancing due to result
    if(res == false)
      return;

    // get the request and response writer back from the result
    // (cleaner/safer this way)
    req = std::move(res.req);
    rw  = std::move(res.rw);

    // create a transaction
    auto& trans = create_transaction(std::move(rw));

    // get a node according the algorithm
    const auto& node = get_node();
    PRINT("get_node(): %s\n", node.to_string().c_str());

    client.send(std::move(req), node.host, {trans, &Transaction::handle_resp});
  }

  Transaction& Balancer::create_transaction(http::Response_writer_ptr rw)
  {
    // TODO: Throw when limit of transactions is reached

    stats_transactions++;

    if(free_index.size() > 0)
    {
      auto idx = free_index.back();
      Ensures(transactions[idx] == nullptr);
      transactions[idx] =
        std::make_unique<Transaction>(*this, std::move(rw), idx);
      free_index.pop_back();
      return *(transactions[idx]);
    }
    else
    {
      auto idx = transactions.size();
      transactions.emplace_back(
        std::make_unique<Transaction>(*this, std::move(rw), idx));
      return *(transactions.back());
    }
  }

  Input_result Balancer::process_input_chain(http::Request_ptr req, http::Response_writer_ptr rw)
  {
    Input_result res{std::move(req), std::move(rw)};

    for(auto& filter : input_chain)
    {
      res = filter(std::move(res.req), std::move(res.rw), *this);

      if(res == false)
      {
        break;
      }
    }

    return res;
  }

  Output_result Balancer::process_output_chain(http::Response_ptr resp)
  {
    Output_result res{std::move(resp)};

    for(auto& filter : output_chain)
    {
      res = filter(std::move(res.resp));

      if(res == false)
      {
        break;
      }
    }

    return res;
  }

  void Balancer::remove_trans(Transaction& trans)
  {
    PRINT("remove_trans(): %d\n", trans.index);
    const auto idx = trans.index;
    transactions[idx] = nullptr;

    //if(free_index.capacity() < transactions.size())
    //  free_index.reserve(transactions.size());

    free_index.push_back(idx);
  }

  const Node& Balancer::get_node() const
  {
    static unsigned i = 0;
    return nodes.at(i++ % nodes.size());
  }

}
