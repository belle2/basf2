#ifndef _Belle2_ArichHVCommunicator_h
#define _Belle2_ArichHVCommunicator_h

#include "daq/slc/hvcontrol/arich/ArichHVMessage.h"

#include <daq/slc/hvcontrol/HVControlCallback.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>

namespace Belle2 {

  class ArichHVCommunicator {

  public:
    ArichHVCommunicator(HVControlCallback* callback, unsigned int crateid,
                        const std::string& host, int port);
    ~ArichHVCommunicator() throw() {}

  public:
    bool connect() throw();
    bool configure() throw();
    bool turnon() throw();
    bool turnoff() throw();
    bool standby() throw();
    bool shoulder() throw();
    bool peak() throw();

  public:
    ArichHVMessage readParams(int slot, int channel) throw(IOException);
    HVControlCallback* getCallback() { return m_callback; }
    int getId() const { return m_crateid; }

  private:
    std::string send(ArichHVMessage& msg) throw(IOException);
    bool perform(ArichHVMessage& msg, HVState satate) throw();

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
