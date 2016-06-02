#pragma once
#ifndef UTILITY_READABLE_HPP
#define UTILITY_READABLE_HPP

#include <delegate>
#include "stream.hpp"

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param t [description]
 * @param t [description]
 *
 * @return [description]
 */
class Readable {
private:
  /** Data read() (async) */
  using on_data_handler = delegate<void(stream::Chunk)>;

  using on_close_handler = delegate<void()>;

  using on_end_handler = delegate<void()>;

  using on_error_handler = delegate<void()>; // TODO: Add error object

  /** Indicates that there is data to be read synchronus */
  using on_readable_handler = delegate<void()>;

public:
  Readable();

  // Starts listening on data
  void on_data(on_data_handler handler)
  { on_data_ = handler; }

  // When stream and any of its underlying resources have been closed.
  void on_close(on_close_handler handler)
  { on_close_ = handler; }

  // When there is no more data to be read (EOF)
  void on_end(on_end_handler handler)
  { on_end_ = handler; }

  // If there was an error receiving data
  void on_error(on_error_handler handler)
  { on_error_ = handler; }

  void on_readable(on_readable_handler handler)
  { on_readable_ = handler; }

  /**
   * @brief Read sync.
   * @details Read from buffer. If buffer empty, start _read() (async)
   *
   * @param  [description]
   * @return [description]
   */
  stream::Chunk read();

  bool is_paused() const
  { return !flowing; }

  const stream::Buffer& buffer()
  { return buffer_; }

  Writable& pipe(Writable&, bool end = true);

  Readable& unpipe(Writable&);

  Readable& resume();
  Readable& pause();

protected:
  /** Read async. Need to push */
  virtual void _read(size_t = 0) = 0;

  /**
   * @brief Push content downwards.
   * @details Push content to destination.
   * If no destination, or destination don't wanna receive more, buffer.
   *
   * @param  [description]
   */
  bool push(stream::Chunk);

private:
  on_data_handler on_data_;
  on_close_handler on_close_;
  on_end_handler on_end_;
  on_error_handler on_error_;
  on_readable_handler on_readable_;
  stream::Buffer buffer_;

  // flowing state
  bool flowing = false;

  void buffer(stream::Chunk);

  bool need_more_data() const;

};



#endif
