#pragma once
#ifndef FS_FILESTREAM
#define FS_FILESTREAM

#include <utility/stream>
#include "disk.hpp"

namespace fs {

class FileStream : public Readable {
private:
  const uint64_t CHUNK_SIZE;
public:

  FileStream(Disk_ptr disk, const Dirent& ent, uint64_t chunk_size = 64000)
    : disk_(disk), entry_(ent), CHUNK_SIZE(chunk_size)
  {
    _read(CHUNK_SIZE);
  }

  FileStream(Disk_ptr disk, const std::string& fpath, uint64_t chunk_size = 64000)
    : disk_(disk), CHUNK_SIZE(chunk_size)
  {
    resolve(fpath);
  }

  const Dirent& entry() const
  { return entry_; }

private:
  Disk_ptr disk_;
  Dirent entry_;

  virtual void _read(size_t n) override {
    disk_->fs().read(entry_, pos(), n,
      [this](error_t err, buffer_t buf, uint64_t len)
    {
      if(err) {

      }
      push({buf, static_cast<uint32_t>(len)});
    });
  }

  void resolve(const std::string& fpath) {
    disk_->fs().stat(fpath, [this](error_t err, const Dirent& ent) {
      entry_ = ent;
      _read(CHUNK_SIZE);
    });
  }

  uint64_t pos() const
  { return 0; }

};

}; // < namespace fs

#endif
