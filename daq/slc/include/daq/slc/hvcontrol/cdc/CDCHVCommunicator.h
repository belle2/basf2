#ifndef _Belle2_CDCHVCommunicator_h
#define _Belle2_CDCHVCommunicator_h

#include "daq/slc/hvcontrol/cdc/CDCHVMessage.h"

#include <daq/slc/hvcontrol/HVControlCallback.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>

namespace Belle2 {

  class CDCHVCommunicator {

  public:
    CDCHVCommunicator(HVControlCallback* callback, unsigned int crateid,
                      const std::string& host, int port);
    ~CDCHVCommunicator() throw() {}

  public:
    bool connect() throw();
    bool configure() throw();
    bool turnon() throw();
    bool turnoff() throw();
    bool standby() throw();
    bool shoulder() throw();
    bool peak() throw();

  public:
    CDCHVMessage readParams(int slot, int channel) throw(IOException);
    HVControlCallback* getCallback() { return m_callback; }
    int getId() const { return m_crateid; }

  public:
    std::string send(CDCHVMessage& msg) throw(IOException);
    bool perform(CDCHVMessage& msg, HVState satate) throw();

  private:
    HVControlCallback* m_callback;
    int m_crateid;
    TCPSocket m_socket;
    bool m_available;
    Mutex m_mutex;
    TCPSocketWriter m_writer;
    TCPSocketReader m_reader;

  };

}

#endif
