#ifndef _B2DAQ_GUICommunicator_hh
#define _B2DAQ_GUICommunicator_hh

#include "HostCommunicator.hh"
#include "NSMDataManager.hh"

#include <xml/NodeLoader.hh>

#include <db/DBInterface.hh>

#include <system/TCPServerSocket.hh>
#include <system/Mutex.hh>
#include <system/TCPSocketWriter.hh>
#include <system/TCPSocketReader.hh>
#include <system/BufferedWriter.hh>

namespace B2DAQ {

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

