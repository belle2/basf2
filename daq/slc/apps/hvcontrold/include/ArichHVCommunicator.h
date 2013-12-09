#ifndef Belle2_ArichHVCommunicator_h
#define Belle2_ArichHVCommunicator_h

#include "daq/slc/apps/hvcontrold/ArichHVMessage.h"
#include "daq/slc/apps/hvcontrold/ArichHVCallback.h"

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>

namespace Belle2 {

  class ArichHVCommunicator {

  public:
    ArichHVCommunicator(ArichHVCallback* callback,
                        const std::string& host,
                        int port, NSMData* data)
      : _crate(callback->getCrate()), _host(host),
        _port(port), _data(data) {
      callback->setHVCommunicator(this);
      _callback = callback;
    }
    ~ArichHVCommunicator() throw() {}

  public:
    std::string sendRequest(ArichHVMessage& msg)
    throw(IOException);
    void run();

  private:
    HVCrateInfo* _crate;
    std::string _host;
    int _port;
    NSMData* _data;
    bool _available;
    Mutex _mutex;
    TCPSocket _socket;
    TCPSocketWriter _writer;
    TCPSocketReader _reader;
    ArichHVCallback* _callback;

  };

}

#endif
