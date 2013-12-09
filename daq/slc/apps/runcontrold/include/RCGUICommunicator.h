#ifndef _Belle2_RCGUICommunicator_h
#define _Belle2_RCGUICommunicator_h

#include "daq/slc/apps/runcontrold/RCCommunicator.h"

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>

namespace Belle2 {

  class RCMasterCallback;

  class RCGUICommunicator : public RCCommunicator {

  public:
    RCGUICommunicator(const TCPSocket& socket,
                      RCMasterCallback* callback);
    virtual ~RCGUICommunicator() throw();

  public:
    virtual bool sendMessage(const RunControlMessage& msg) throw();
    virtual bool sendState(NSMNode* node) throw();
    virtual bool sendDataObject(const std::string& name,
                                DataObject* data) throw();
    virtual bool isOnline(NSMNode*) throw() { return true; }

  public:
    void run();

  private:
    TCPSocket _socket;
    TCPSocketWriter _writer;
    TCPSocketReader _reader;
    Mutex _socket_mutex;
    RCMasterCallback* _callback;

  };

}

#endif
