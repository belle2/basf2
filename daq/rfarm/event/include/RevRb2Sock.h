//+
// File : Rb2Sock.h
// Description : Pick an event from RingBuffer and send it to EvtSocket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 10 - Dec - 2002, Orignal for RFARM
//        26 - Apr - 2012, for Belle II RFARM
//-

#include <string>

#include "framework/pcore/RingBuffer.h"
#include "daq/dataflow/REvtSocket.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#define RBUFSIZE 100000000

namespace Belle2 {

  class RevRb2Sock {
  public:
    /*! Constuctor and Destructor */
    RevRb2Sock(std::string rbuf, int port, std::string shmname, int id);
    ~RevRb2Sock(void);

    /*! Event function */
    int SendEvent(void);

  private:
    RingBuffer* m_rbuf;
    REvtSocketSend* m_sock;
    RFFlowStat* m_flow;
    char* m_evtbuf;

  };

}
