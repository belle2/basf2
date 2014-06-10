#ifndef _Belle2_NSM2SocketBridge_h
#define _Belle2_NSM2SocketBridge_h

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMMessage.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Mutex.h>

#include <daq/slc/base/ERRORNo.h>

#include <list>

namespace Belle2 {

  class NSM2SocketCallback;

  class NSM2SocketBridge {

  public:
    NSM2SocketBridge(const TCPSocket& socket,
                     NSM2SocketCallback* callback,
                     DBInterface* db);
    ~NSM2SocketBridge() throw();

  public:
    void run() throw();
    bool sendMessage(const NSMMessage& msg) throw();
    bool recieveMessage(NSMMessage& msg) throw();
    bool sendLog(const DAQLogMessage& log) throw();
    bool sendError(const ERRORNo& eno,
                   const std::string& nodename,
                   const std::string& message) throw();

  private:
    NSM2SocketCallback* m_callback;
    DBInterface* m_db;
    TCPSocket m_socket;
    TCPSocketWriter m_writer;
    TCPSocketReader m_reader;
    Mutex m_mutex;

  };

}

#endif
