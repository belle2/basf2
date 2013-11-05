#include "MessageBox.h"

using namespace Belle2;

MessageBox MessageBox::__box;

RunControlMessage MessageBox::pop() throw()
{
  _mutex.lock();
  while (_message_v.empty()) {
    _cond.wait(_mutex);
  }
  RunControlMessage message = _message_v.front();
  _message_v.pop();
  _mutex.unlock();
  return message;
}

void MessageBox::push(const RunControlMessage& message) throw()
{
  _mutex.lock();
  _message_v.push(message);
  _cond.signal();
  _mutex.unlock();
}
