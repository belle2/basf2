#include "daq/storage/SharedEventBuffer.h"

#include <daq/slc/system/LogFile.h>

#include <cstring>
#include <cstdio>
#include <unistd.h>

using namespace Belle2;

size_t SharedEventBuffer::size() throw()
{
  return _mutex.size() + _cond.size() +
         sizeof(Header) + sizeof(int) * (_nword);
}

bool SharedEventBuffer::open(const std::string& nodename,
                             size_t nword, bool recreate)
{
  _nword = nword;
  std::string username = getenv("USER");
  _path = "/storage_info_" + username + "_" + nodename;
  LogFile::debug("%s", _path.c_str());
  if (recreate) SharedMemory::unlink(_path);
  if (!_memory.open(_path, size())) {
    perror("shm_open");
    LogFile::fatal("Failed to open %s", _path.c_str());
    return false;
  }
  char* buf = (char*) _memory.map(0, size());
  if (buf == NULL) {
    return false;
  }
  _mutex = MMutex(buf);
  buf += _mutex.size();
  _cond = MCond(buf);
  buf += _cond.size();
  _header = (Header*)buf;
  buf += sizeof(Header);
  _buf = (int*)buf;
  if (recreate) init();
  return true;
}

bool SharedEventBuffer::init()
{
  if (_buf == NULL) return false;
  _mutex.init();
  _cond.init();
  memset(_header, 0, sizeof(Header));
  memset(_buf, 0, (_nword) * sizeof(int));
  return true;
}

void SharedEventBuffer::clear()
{
  if (_buf == NULL) return;
  _mutex.lock();
  memset(_header, 0, sizeof(Header));
  memset(_buf, 0, (_nword) * sizeof(int));
  _mutex.unlock();
}

bool SharedEventBuffer::close()
{
  _memory.close();
  return true;
}

bool SharedEventBuffer::unlink()
{
  _memory.unlink();
  _memory.close();
  return true;
}

bool SharedEventBuffer::lock() throw()
{
  if (_buf == NULL) return false;
  return _mutex.lock();
}

bool SharedEventBuffer::unlock() throw()
{
  if (_buf == NULL) return false;
  return _mutex.unlock();
}

bool SharedEventBuffer::wait() throw()
{
  if (_buf == NULL) return false;
  return _cond.wait(_mutex);
}

bool SharedEventBuffer::wait(int time) throw()
{
  if (_buf == NULL) return false;
  return _cond.wait(_mutex, time, 0);
}

bool SharedEventBuffer::notify() throw()
{
  if (_buf == NULL) return false;
  return _cond.broadcast();
}

bool SharedEventBuffer::isWritable(int nword) throw()
{
  if (_buf == NULL) return false;
  _mutex.lock();
  bool writable = _header->nword_in - _header->nword_out < _nword - (nword + 1);
  _mutex.unlock();
  return writable;
}

bool SharedEventBuffer::isReadable(int nword) throw()
{
  if (_buf == NULL) return false;
  _mutex.lock();
  bool readable = _header->nword_in - _header->nword_out >= _nword - (nword + 1);
  _mutex.unlock();
  return readable;

}

unsigned int SharedEventBuffer::write(const int* buf, unsigned int nword,
                                      unsigned int serial)
{
  if (_buf == NULL) return 0;
  if (nword == 0) return 0;
  if (nword > _nword) return -1;
  _mutex.lock();
  unsigned int i_w = 0;
  unsigned int i_r = 0;
  while (true) {
    i_w = _header->nword_in % _nword;
    i_r = _header->nword_out % _nword;
    if ((serial == 0 || serial - 1 == _header->count_in) &&
        _header->nword_in - _header->nword_out < _nword - (nword + 1)) {
      if (i_w >= i_r) {
        unsigned int count = _nword - i_w;
        if (nword + 1 < count) {
          _buf[i_w] = nword;
          memcpy((_buf + i_w + 1), buf, sizeof(int) * nword);
        } else {
          _buf[i_w] = nword;
          memcpy((_buf + i_w + 1), buf, sizeof(int) * count);
          if (nword >= count)
            memcpy(_buf, buf + count, sizeof(int) * (nword - count));
        }
      } else {
        _buf[i_w] = nword;
        memcpy((_buf + i_w + 1), buf, sizeof(int) * nword);
      }
      break;
    }
    _cond.wait(_mutex);
  }
  _header->nword_in += nword + 1;
  unsigned int count = ++_header->count_in;
  _cond.broadcast();
  _mutex.unlock();
  return count;
}

unsigned int SharedEventBuffer::read(int* buf)
{
  if (_buf == NULL) return 0;
  _mutex.lock();
  unsigned int i_w = 0;
  unsigned int i_r = 0;
  unsigned int nword = 0;
  while (true) {
    i_w = _header->nword_in % _nword;
    i_r = _header->nword_out % _nword;
    nword = _buf[i_r];
    if (nword > 0) {
      if (_header->nword_in - _header->nword_out >= (nword + 1)) {
        if (i_w > i_r) {
          memcpy(buf, (_buf + i_r + 1), sizeof(int) * nword);
          break;
        } else if (i_w < i_r) {
          if (_nword - i_r > nword) {
            memcpy(buf, (_buf + i_r + 1), sizeof(int) * nword);
            break;
          } else {
            unsigned int count = _nword - i_r;
            memcpy(buf, (_buf + i_r + 1), sizeof(int) * count);
            if (nword > count)
              memcpy(buf + count, _buf, sizeof(int) * (nword - count));
            break;
          }
        }
      }
    }
    _cond.wait(_mutex);
  }
  _header->nword_out += nword + 1;
  unsigned int count = ++_header->count_out;
  _cond.broadcast();
  _mutex.unlock();
  return count;
}
