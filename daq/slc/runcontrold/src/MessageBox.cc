#include "MessageBox.hh"

#include <iostream>

using namespace B2DAQ;

MessageBox MessageBox::__box;

RunControlMessage MessageBox::pop() throw()
{
  _mutex.lock();
  while (_message_v.empty()) {
    _cond.wait(_mutex);
  }
  //std::cerr << __FILE__ << ":" << __LINE__ << " size = " << _message_v.size() << std::endl;
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
