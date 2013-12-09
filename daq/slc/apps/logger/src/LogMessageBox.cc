#include "daq/slc/apps/logger/LogMessageBox.h"

using namespace Belle2;

SystemLog LogMessageBox::pop() throw()
{
  _mutex.lock();
  while (_message_v.empty()) {
    _cond.wait(_mutex);
  }
  SystemLog message = _message_v.front();
  _message_v.pop();
  _mutex.unlock();
  return message;
}

void LogMessageBox::push(const SystemLog& message) throw()
{
  _mutex.lock();
  _message_v.push(message);
  _cond.signal();
  _mutex.unlock();
}
