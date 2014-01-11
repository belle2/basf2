#include "daq/slc/readout/RunInfoBuffer.h"

#include <daq/slc/base/Debugger.h>
#include <daq/slc/system/LogFile.h>

#include <cstring>

using namespace Belle2;

size_t RunInfoBuffer::size() throw()
{
  return _mutex.size() + _cond.size() + sizeof(unsigned int) * 10;
}

bool RunInfoBuffer::open(const std::string& nodename, bool recreate)
{
  std::string username = getenv("USER");
  _path = "/run_info_buf_" + username + "_" + nodename;
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
  _info = (unsigned int*)buf;
  buf += sizeof(unsigned int) * 10;
  _mutex = MMutex(buf);
  buf += _mutex.size();
  _cond = MCond(buf);
  return true;
}

bool RunInfoBuffer::init()
{
  if (_info == NULL) return false;
  _mutex.init();
  _cond.init();
  memset(_info, 0, sizeof(unsigned int) * 10);
  return true;
}

void RunInfoBuffer::clear()
{
  _mutex.lock();
  memset(_info, 0, sizeof(unsigned int) * 10);
  _mutex.unlock();
}

bool RunInfoBuffer::close()
{
  _memory.close();
  return true;
}

bool RunInfoBuffer::unlink()
{
  _memory.unlink();
  _memory.close();
  return true;
}

bool RunInfoBuffer::lock() throw()
{
  return _mutex.lock();
}

bool RunInfoBuffer::unlock() throw()
{
  return _mutex.unlock();
}

bool RunInfoBuffer::wait() throw()
{
  return _cond.wait(_mutex);
}

bool RunInfoBuffer::wait(int time) throw()
{
  return _cond.wait(_mutex, time, 0);
}

bool RunInfoBuffer::notify() throw()
{
  return _cond.broadcast();
}

bool RunInfoBuffer::waitRunning(int timeout)
{
  lock();
  if (getState() != RunInfoBuffer::RUNNING) {
    if (!wait(timeout)) {
      unlock();
      return false;
    }
  }
  unlock();
  return true;
}

bool RunInfoBuffer::reportRunning()
{
  lock();
  setState(RunInfoBuffer::RUNNING);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportError()
{
  lock();
  setState(RunInfoBuffer::ERROR);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportReady()
{
  lock();
  setState(RunInfoBuffer::READY);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportNotReady()
{
  lock();
  setState(RunInfoBuffer::NOTREADY);
  notify();
  unlock();
  return true;
}

