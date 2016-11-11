
#pragma once
#ifndef NET_FRAME_HPP
#define NET_FRAME_HPP

#include "buffer.hpp"

namespace net {

  /** Anonymous link Frame, used for casting purposes */
  class Frame : public net::Buffer {
  public:
    using ptr = bufptr<Frame>;
  private:
    void upstream() const
    { printf("<net::Frame> Calling upstream on frame does nothing.\n"); }

  }; // < class Frame
}


#endif
