#ifndef _Belle2_NSM2SocketBridge_h
#define _Belle2_NSM2SocketBridge_h

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMMessage.h>

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/Mutex.h>

#include <daq/slc/base/SystemLog.h>

#include <list>

namespace Belle2 {

  class NSM2SocketCallback;
  class NSM2SocketBridgeThread;

  class NSM2SocketBridge {

  public:
    NSM2SocketBridge(const std::string& host, int port,
                     NSM2SocketCallback* callback,
                     DBInterface* db);
    ~NSM2SocketBridge() throw();

  public:
    void run() throw();
    void sendMessage(NSMMessage& message) throw();
    void add(NSM2SocketBridgeThread* thread);
    void remove(NSM2SocketBridgeThread* thread);
    NSM2SocketCallback* getCallback() { return m_callback; }

  private:
    NSM2SocketCallback* m_callback;
    DBInterface* m_db;
    TCPServerSocket m_server_socket;
    int m_timeout;
    std::list<NSM2SocketBridgeThread*> m_thread_l;
    Mutex m_mutex;

  };

}

#endif
