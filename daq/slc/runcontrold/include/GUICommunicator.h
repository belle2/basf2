#ifndef _Belle2_GUICommunicator_hh
#define _Belle2_GUICommunicator_hh

#include "xml/NodeLoader.h"

#include "database/DBInterface.h"

#include "system/Mutex.h"
#include "system/TCPSocketWriter.h"
#include "system/TCPSocketReader.h"
#include "system/BufferedWriter.h"

#include "RunControlMessage.h"

namespace Belle2 {

  class GUICommunicator {

  public:
    GUICommunicator(const std::string& host, int port,
                    DBInterface* db, NodeLoader* loader)
      : _host(host), _port(port), _loader(loader), _db(db) {}
    ~GUICommunicator() throw() {}

  public:
    void run();
    void sendMessage(const RunControlMessage& message)
    throw(IOException);
    bool isReady() { return _is_ready; }

  private:
    std::string _host;
    int _port;
    NodeLoader* _loader;
    DBInterface* _db;
    TCPSocket _socket;
    TCPSocketWriter _writer;
    TCPSocketReader _reader;
    BufferedWriter _buf;
    Mutex _mutex;
    bool _is_ready;
  };

}

#endif

