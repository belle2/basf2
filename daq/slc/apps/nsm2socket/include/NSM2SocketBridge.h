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
#include <daq/slc/system/Cond.h>

#include <daq/slc/base/ERRORNo.h>

#include <list>

namespace Belle2 {

  class NSM2SocketCallback;

  class NSM2SocketBridge {

  public:
    NSM2SocketBridge(const TCPSocket& socket);
    ~NSM2SocketBridge() throw();

  public:
    void run() throw();
    bool send(const NSMMessage& msg) throw();
    bool send(const NSMMessage& msg,
              const Serializable& obj) throw();

  protected:
    void dbget(const std::string& table, const std::string& config,
               const NSMNode& node, bool isfull);
    void dblistget(const std::string& table,
                   const NSMNode& node, const std::string& grep);
    void dbset(const std::string& table, const DBObject& obj);
    void nsmdataget(int revision, const std::string& name,
                    const std::string& format);
    void loglist(const std::string& table,
                 const NSMNode& node, int max);

    DBInterface& getDB() { return *m_db; }

  private:
    NSM2SocketCallback* m_callback;
    DBInterface* m_db;
    TCPSocket m_socket;
    TCPSocketWriter m_writer;
    TCPSocketReader m_reader;
    Mutex m_mutex;
    Cond m_cond;
    std::string m_table;

  };

}

#endif
