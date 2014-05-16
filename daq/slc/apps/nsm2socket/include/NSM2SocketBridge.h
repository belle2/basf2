#ifndef _Belle2_NSM2SocketBridge_h
#define _Belle2_NSM2SocketBridge_h

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMMessage.h>

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Mutex.h>

#include <list>

namespace Belle2 {

  class NSM2SocketCallback;

  class NSM2SocketBridge {

  public:
    NSM2SocketBridge(const std::string& host, int port,
                     NSM2SocketCallback* callback,
                     DBInterface* db);
    ~NSM2SocketBridge() throw();

  public:
    void run() throw();
    bool sendMessage(const NSMMessage& msg) throw();
    bool recieveMessage(NSMMessage& msg) throw();
    NSM2SocketCallback* getCallback() { return m_callback; }

  private:
    NSM2SocketCallback* m_callback;
    DBInterface* m_db;
    TCPServerSocket m_server_socket;
    TCPSocketWriter m_writer;
    TCPSocketReader m_reader;
    int m_timeout;
    Mutex m_mutex;

  };

}

#endif
