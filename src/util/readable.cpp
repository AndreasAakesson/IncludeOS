#include <utility/readable.hpp>
#include <utility/writable.hpp>
#include <cstdio>

using namespace stream;

Readable::Readable() {
  // buffer by default
  on_data_ = on_data_handler::from<Readable, &Readable::buffer>(this);
  on_readable_ = []{
    printf("<Readable::on_readable_> There is data to be read.\n");
  };
}


Chunk Readable::read() {
  printf("<Readable::read> Reading (sync)\n");
  Chunk chunk;

  if(!buffer_.empty()) {
    printf("<Readable::read> Got buffered chunks\n");
    chunk = buffer_.front();
    buffer_.pop_front();
  }
  else {
    printf("<Readable::read> Empty buffer - call _read()\n");
    _read();
  }

  return chunk;
}

bool Readable::push(Chunk chunk) {
  // No more data to be read.
  if(!chunk) {
    printf("<Readable::push> Empty chunk - EOF.\n");
    //end();
    return false;
  }

  if(flowing and buffer_.empty()) {
    printf("<Readable::push> Flow and empty buffer - call on_data_\n");
    on_data_(chunk); // dest.write
  }
  else {
    buffer(chunk);
  }

  return need_more_data();
}

void Readable::buffer(Chunk chunk) {
  printf("<Readable::buffer> Buffering chunk.\n");
  buffer_.push_back(chunk);

  if(!flowing) {
    printf("<Readable::buffer> Is flowing - call on_readable_\n");
    on_readable_();
  }
}

bool Readable::need_more_data() const {
  return true;
}

Readable& Readable::pause() {
  printf("<Readable::pause> Pausing.\n");
  if(flowing) {
    flowing = false;
  }
  return *this;
}

Readable& Readable::resume() {
  printf("<Readable::pause> Resuming.\n");
  if(!flowing) {
    flowing = true;
    read();
  }
  return *this;
}

Writable& Readable::pipe(Writable& dest, bool end) {
  printf("<Readable::pipe> Piping writer\n");

  on_data_ = [&](Chunk chunk) {
    if(!dest.write(chunk)){
      this->pause();
    }
  };

  //dest.emit_pipe(this);

  if(!flowing)
    resume();

  return dest;
}

Readable& Readable::unpipe(Writable& dest) {
  //if(!pipes_) {
  //  return this;
  //}
  flowing = false;
  //dest.on_unpipe(this);

  return *this;
}

