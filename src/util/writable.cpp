#include <utility/writable.hpp>
#include <utility/readable.hpp>

using namespace stream;

bool Writable::write(Chunk) {
  return true;
}
