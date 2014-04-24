#ifndef _Belle2_NSM2SocketBridgeThread_h
#define _Belle2_NSM2SocketBridgeThread_h

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMMessage.h>

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Mutex.h>

#include <daq/slc/base/SystemLog.h>

namespace Belle2 {

  class NSM2SocketBridge;

  class NSM2SocketBridgeThread {

  public:
    NSM2SocketBridgeThread(const TCPSocket& socket,
                           NSM2SocketBridge* bridge,
                           DBInterface* db);
    ~NSM2SocketBridgeThread() throw();

  public:
    void run() throw();
    void sendMessage(NSMMessage& message) throw();

  private:
    NSM2SocketBridge* m_bridge;
    TCPSocket m_socket;
    DBInterface* m_db;
    TCPSocketReader m_reader;
    TCPSocketWriter m_writer;
    Mutex m_mutex;

  };

}

#endif
