#pragma once
#ifndef UTILITY_STREAM_HPP
#define UTILITY_STREAM_HPP

#include <memory>
#include <deque>

class Readable;
class Writable;

namespace stream {
  struct Chunk;
  using buffer_t = std::shared_ptr<uint8_t>;
  using Buffer = std::deque<Chunk>;


  struct Chunk {
    buffer_t data;
    uint32_t length;

    Chunk(buffer_t buf = nullptr, size_t len = 0)
      : data(buf), length(len) {}

    operator bool () const
    { return length and data; }

  };
}

#endif
