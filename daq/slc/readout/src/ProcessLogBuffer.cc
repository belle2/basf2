#include "daq/slc/readout/ProcessLogBuffer.h"

#include "daq/slc/base/Debugger.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cstdio>
#include <cstring>

using namespace Belle2;

const int ProcessLogBuffer::MAX_MESSAGE = 20;

size_t ProcessLogBuffer::size() throw()
{
  return _mutex.size() + _cond.size() + sizeof(int) * 2 +
         sizeof(process_log_message) * MAX_MESSAGE;
}

bool ProcessLogBuffer::open(const std::string& path)
{
  _path = path;
  if (!_memory.open(path, size())) {
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
  _windex = (int*)buf;
  buf += sizeof(int);
  _rindex = (int*)buf;
  buf += sizeof(int);
  _msg_v = (process_log_message*)buf;
  _available = true;
  return true;
}

bool ProcessLogBuffer::create(const std::string& path)
{
  _path = path;
  if (open(path)) {
    _mutex.init();
    _cond.init();
    *_windex = *_rindex = 0;
    memset(_msg_v, 0, sizeof(process_log_message) * MAX_MESSAGE);
    return true;
  }
  return false;
}

void ProcessLogBuffer::clear()
{
  *_windex = *_rindex = 0;
  memset(_msg_v, 0, sizeof(process_log_message) * MAX_MESSAGE);
}

void ProcessLogBuffer::close()
{
  _memory.close();
  _available = false;
}

void ProcessLogBuffer::unlink(const std::string& path)
{
  SharedMemory::unlink(path);
  close();
}

std::string ProcessLogBuffer::recieve(SystemLog::Priority& priority, int timeout)
{
  if (!_available) return "";
  _mutex.lock();
  if (*_rindex == MAX_MESSAGE) *_rindex = 0;
  int i = *_rindex;
  if (_msg_v[i].priority == SystemLog::UNKNOWN) {
    if (timeout > 0) {
      _cond.wait(_mutex, timeout);
    } else {
      _cond.wait(_mutex);
    }
  }
  if (_msg_v[i].priority == SystemLog::UNKNOWN) {
    priority = SystemLog::UNKNOWN;
    _mutex.unlock();
    return "";
  }
  (*_rindex)++;
  priority = _msg_v[i].priority;
  _msg_v[i].priority = SystemLog::UNKNOWN;
  std::string message = _msg_v[i].message;
  _cond.signal();
  _mutex.unlock();
  return message;
}

bool ProcessLogBuffer::send(SystemLog::Priority priority,
                            const std::string& message)
{
  if (!_available) return false;
  _mutex.lock();
  if (*_windex == MAX_MESSAGE) *_windex = 0;
  int i = *_windex;
  while (_msg_v[i].priority > SystemLog::UNKNOWN) {
    _cond.wait(_mutex);
  }
  (*_windex)++;
  _msg_v[i].priority = priority;
  strcpy(_msg_v[i].message, message.c_str());
  _cond.signal();
  _mutex.unlock();
  return true;
}
