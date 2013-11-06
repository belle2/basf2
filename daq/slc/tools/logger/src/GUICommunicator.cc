#include "GUICommunicator.h"

#include "system/TCPServerSocket.h"

#include "base/State.h"
#include "base/Connection.h"
#include "base/Debugger.h"
#include "base/StringUtil.h"

#include <unistd.h>

using namespace Belle2;

GUICommunicator::GUICommunicatorList GUICommunicator::__comm_l;

Mutex GUICommunicator::__mutex;

GUICommunicator::GUICommunicator(const TCPSocket& socket)
  : _socket(socket)
{
  __mutex.lock();
  __comm_l.push_back(this);
  __mutex.unlock();
}

GUICommunicator::~GUICommunicator() throw()
{
  __mutex.lock();
  __comm_l.remove(this);
  __mutex.unlock();
}

void GUICommunicator::push(const LogMessage& msg)
{
  __mutex.lock();
  for (GUICommunicatorList::iterator it = __comm_l.begin();
       it != __comm_l.end(); it++) {
    (*it)->_box.push(msg);
  }
  __mutex.unlock();
}

void GUICommunicator::run()
{
  _is_ready = false;
  _writer = TCPSocketWriter(_socket);
  _reader = TCPSocketReader(_socket);
  _is_ready = true;
  try {
    int npar = 0;
    int pars[3];
    std::string str;
    while (true) {
      LogMessage msg = _box.pop();
      npar = msg.pack(pars, str);
      _writer.writeInt(npar);
      for (int i = 0; i < npar; i++) {
        _writer.writeInt(pars[i]);
      }
      _writer.writeString(str);
    }
  } catch (const IOException& e) {
    Belle2::debug("[DEBUG] %s:%d: Connection broken", __FILE__, __LINE__);
  }
  _is_ready = false;
  _socket.close();
}
