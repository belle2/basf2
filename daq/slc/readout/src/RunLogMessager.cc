#include "daq/slc/readout/RunLogMessanger.h"

#include "daq/slc/base/Debugger.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cstdio>
#include <cstring>

using namespace Belle2;

const int RunLogMessanger::DEBUG = 1;
const int RunLogMessanger::NOTICE = 2;
const int RunLogMessanger::ERROR = 3;
const int RunLogMessanger::FATAL = 4;

const int RunLogMessanger::MAX_MESSAGE = 20;

size_t RunLogMessanger::size() throw()
{
  return _mutex.size() + _cond.size() + sizeof(int) * 2 +
         sizeof(run_log_message) * MAX_MESSAGE;
}

bool RunLogMessanger::open(const std::string& path)
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
  _msg_v = (run_log_message*)buf;
  return true;
}

bool RunLogMessanger::create(const std::string& path)
{
  _path = path;
  if (open(path)) {
    _mutex.init();
    _cond.init();
    *_windex = *_rindex = 0;
    memset(_msg_v, 0, sizeof(run_log_message) * MAX_MESSAGE);
    return true;
  }
  return false;
}

void RunLogMessanger::close()
{
  _memory.close();
}

void RunLogMessanger::unlink(const std::string& path)
{
  SharedMemory::unlink(path);
  close();
}

std::string RunLogMessanger::recieve(int& priority, int timeout)
{
  _mutex.lock();
  if (*_rindex == MAX_MESSAGE) *_rindex = 0;
  int i = *_rindex;
  if (_msg_v[i].priority == 0) {
    _cond.wait(_mutex, timeout);
  }
  if (_msg_v[i].priority == 0) {
    priority = -1;
    _mutex.unlock();
    return "";
  }
  (*_rindex)++;
  priority = _msg_v[i].priority;
  _msg_v[i].priority = 0;
  std::string message = _msg_v[i].message;
  _cond.signal();
  _mutex.unlock();
  return message;
}

bool RunLogMessanger::send(int priority, const std::string& message)
{
  _mutex.lock();
  if (*_windex == MAX_MESSAGE) *_windex = 0;
  int i = *_windex;
  while (_msg_v[i].priority > 0) {
    _cond.wait(_mutex);
  }
  (*_windex)++;
  _msg_v[i].priority = priority;
  strcpy(_msg_v[i].message, message.c_str());
  _cond.signal();
  _mutex.unlock();
  return true;
}
