
#pragma once
#include <util/timer.hpp>

namespace microLB
{
  class Probe
  {
  public:
    using Probe_result_handler = delegate<void(bool active)>;
    using Interval = std::chrono::seconds;

    void activate(Probe_result_handler handler)
    {
      res_handler = handler;
      Expects(res_handler);
      timer.start(interval);
    }

    virtual ~Probe() {}

  protected:
    Probe(const Interval interval)
      : timer{{this, &Probe::do_probe}},
        interval{interval}
    {}

    void result(bool active)
    {
      res_handler(active);
      timer.start(interval);
    }

    virtual void do_probe() = 0;

  private:
    Probe_result_handler res_handler;
    Timer timer;
    const Interval interval;
  };

}
