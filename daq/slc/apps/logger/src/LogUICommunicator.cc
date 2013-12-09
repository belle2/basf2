#include "daq/slc/apps/logger/LogUICommunicator.h"

#include "daq/slc/system/TCPServerSocket.h"

#include "daq/slc/base/State.h"
#include "daq/slc/base/Connection.h"
#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <unistd.h>

using namespace Belle2;

LogUICommunicator::LogUICommunicatorList LogUICommunicator::__comm_l;

Mutex LogUICommunicator::__mutex;

LogUICommunicator::LogUICommunicator(const TCPSocket& socket, LogDBManager* dbman)
  : _socket(socket), _dbman(dbman)
{
  __mutex.lock();
  __comm_l.push_back(this);
  __mutex.unlock();
}

LogUICommunicator::~LogUICommunicator() throw()
{
  __mutex.lock();
  __comm_l.remove(this);
  __mutex.unlock();
}

void LogUICommunicator::push(const SystemLog& msg)
{
  __mutex.lock();
  for (LogUICommunicatorList::iterator it = __comm_l.begin();
       it != __comm_l.end(); it++) {
    (*it)->_box.push(msg);
  }
  __mutex.unlock();
}

void LogUICommunicator::run()
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
      SystemLog msg = _box.pop();
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
