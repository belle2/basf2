//+
// File : Sock2Rb.h
// Description : Receive an event from EvtSocket and place it in Rbuf.
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 10 - Dec - 2002, Orignal for RFARM
//        26 - Apr - 2012, for Belle II RFARM
//-

#include <string>

#include "framework/pcore/RingBuffer.h"
#include "daq/dataflow/RSocketLib.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#define MAXBUFSIZE 20000000
#define RBUFSIZE 100000000

class RawRevSock2Rb {
public:
  /*! Constuctor and Destructor */
  RawRevSock2Rb(std::string rbuf, std::string src, int port, std::string shmname, int id);
  ~RawRevSock2Rb(void);

  /*! Event function */
  int ReceiveEvent(void);

  /*! Reconnect */
  int Reconnect(int ntry);

private:
  Belle2::RingBuffer* m_rbuf;
  Belle2::RSocketRecv* m_sock;
  Belle2::RFFlowStat* m_flow;
  char* m_evtbuf;
  int* m_buf;

};

