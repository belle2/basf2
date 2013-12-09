#ifndef _Belle2_LogUICommunicator_hh
#define _Belle2_LogUICommunicator_hh

#include "daq/slc/apps/logger/LogMessageBox.h"
#include "daq/slc/apps/logger/LogDBManager.h"

#include "daq/slc/system/Mutex.h"
#include "daq/slc/system/TCPSocketWriter.h"
#include "daq/slc/system/TCPSocketReader.h"

#include <list>

namespace Belle2 {

  class LogUICommunicator {

  public:
    typedef std::list<LogUICommunicator*> LogUICommunicatorList;
    static void push(const SystemLog& msg);

  private:
    static LogUICommunicatorList __comm_l;
    static Mutex __mutex;

  public:
    LogUICommunicator(const TCPSocket& socket, LogDBManager* dbman);
    ~LogUICommunicator() throw();

  public:
    void run();
    bool isReady() { return _is_ready; }

  private:
    TCPSocket _socket;
    LogDBManager* _dbman;
    TCPSocketWriter _writer;
    TCPSocketReader _reader;
    Mutex _mutex;
    LogMessageBox _box;
    bool _is_ready;
  };

}

#endif

