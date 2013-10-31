#ifndef _Belle2_GUICommunicator_hh
#define _Belle2_GUICommunicator_hh

#include "HostCommunicator.h"
#include "NSMDataManager.h"

#include <xml/NodeLoader.h>

#include <database/DBInterface.h>

#include <system/TCPServerSocket.h>
#include <system/Mutex.h>
#include <system/TCPSocketWriter.h>
#include <system/TCPSocketReader.h>
#include <system/BufferedWriter.h>

namespace Belle2 {

  class GUICommunicator : public HostCommunicator {

  public:
    GUICommunicator(const TCPServerSocket& server_socket,
                    DBInterface* db, NodeLoader* loader)
      : _server_socket(server_socket), _loader(loader), _db(db) {}
    virtual ~GUICommunicator() throw() {}

  public:
    virtual bool init() throw(IOException);
    virtual bool reset() throw();
    virtual RunControlMessage waitMessage() throw(IOException);
    virtual void sendMessage(const RunControlMessage& message)
    throw(IOException);

  private:
    TCPServerSocket _server_socket;
    NodeLoader* _loader;
    DBInterface* _db;
    TCPSocket _socket;
    TCPSocketWriter _writer;
    TCPSocketReader _reader;
    BufferedWriter _buf;
    Mutex _mutex;

  };

}

#endif

