#ifndef _B2DAQ_GUICommunicator_hh
#define _B2DAQ_GUICommunicator_hh

#include "HostCommunicator.hh"
#include "NSMDataManager.hh"

#include <xml/NodeLoader.hh>

#include <system/TCPServerSocket.hh>
#include <system/Mutex.hh>
#include <system/TCPSocketWriter.hh>
#include <system/TCPSocketReader.hh>

namespace B2DAQ {

  class GUICommunicator : public HostCommunicator {

  public:
    GUICommunicator(const TCPServerSocket& server_socket, 
		    NodeLoader* loader, NSMDataManager* data) 
      : _server_socket(server_socket), _loader(loader), 
	_system(&(loader->getSystem())), _data(data) {}
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
    NodeSystem* _system;
    NSMDataManager* _data;
    TCPSocket _socket;
    TCPSocketWriter _writer;
    TCPSocketReader _reader;
    Mutex _mutex;

  };

}

#endif

