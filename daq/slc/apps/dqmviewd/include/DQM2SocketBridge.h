#ifndef _Belle2_DQM2SocketBridge_h
#define _Belle2_DQM2SocketBridge_h

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMMessage.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Mutex.h>

namespace Belle2 {

  class DQMViewCallback;

  class DQM2SocketBridge {

  public:
    DQM2SocketBridge(const TCPSocket& socket,
                     DQMViewCallback* callback,
                     DBInterface* db);
    ~DQM2SocketBridge() throw();

  public:
    void run() throw();
    bool sendMessage(const NSMMessage& msg) throw();
    bool recieveMessage(NSMMessage& msg) throw();

  private:
    DQMViewCallback* m_callback;
    DBInterface* m_db;
    TCPSocket m_socket;
    TCPSocketWriter m_writer;
    TCPSocketReader m_reader;
    Mutex m_mutex;

  };

}

#endif
