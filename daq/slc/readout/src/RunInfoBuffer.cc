#include "daq/slc/readout/RunInfoBuffer.h"

#include <daq/slc/base/Debugger.h>
#include <daq/slc/system/LogFile.h>

#include <cstring>

using namespace Belle2;

size_t RunInfoBuffer::size() throw()
{
  return _mutex.size() + _cond.size() +
         sizeof(unsigned int) * (5 + _nreserved);
}

bool RunInfoBuffer::open(const std::string& nodename,
                         int nreserved, bool recreate)
{
  _nreserved = nreserved;
  std::string username = getenv("USER");
  _path = "/run_info_" + username + "_" + nodename;
  //if (recreate) SharedMemory::unlink(_path);
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
  _buf = (unsigned int*)buf;
  if (recreate) init();
  return true;
}

bool RunInfoBuffer::init()
{
  if (_buf == NULL) return false;
  _mutex.init();
  _cond.init();
  memset(_buf, 0, sizeof(unsigned int) * (5 + _nreserved));
  return true;
}

void RunInfoBuffer::clear()
{
  if (_buf == NULL) return;
  _mutex.lock();
  memset(_buf, 0, sizeof(unsigned int) * (5 + _nreserved));
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
  if (_buf == NULL) return false;
  return _mutex.lock();
}

bool RunInfoBuffer::unlock() throw()
{
  if (_buf == NULL) return false;
  return _mutex.unlock();
}

bool RunInfoBuffer::wait() throw()
{
  if (_buf == NULL) return false;
  return _cond.wait(_mutex);
}

bool RunInfoBuffer::wait(int time) throw()
{
  if (_buf == NULL) return false;
  return _cond.wait(_mutex, time, 0);
}

bool RunInfoBuffer::notify() throw()
{
  if (_buf == NULL) return false;
  return _cond.broadcast();
}

bool RunInfoBuffer::waitRunning(int timeout)
{
  if (_buf == NULL) return false;
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
  if (_buf == NULL) return false;
  lock();
  setState(RunInfoBuffer::RUNNING);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportError()
{
  if (_buf == NULL) return false;
  lock();
  setState(RunInfoBuffer::ERROR);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportReady()
{
  if (_buf == NULL) return false;
  lock();
  setState(RunInfoBuffer::READY);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportNotReady()
{
  if (_buf == NULL) return false;
  lock();
  setState(RunInfoBuffer::NOTREADY);
  notify();
  unlock();
  return true;
}

