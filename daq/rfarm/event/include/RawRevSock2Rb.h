//+
// File : Sock2Rb.h
// Description : Receive an event from EvtSocket and place it in Rbuf.
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 10 - Dec - 2002, Orignal for RFARM
//        26 - Apr - 2012, for Belle II RFARM
//-

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#include <vector>
#include <string>

#include "framework/pcore/RingBuffer.h"
#include "daq/dataflow/REvtSocket.h"
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
  RingBuffer* m_rbuf;
  RSocketRecv* m_sock;
  RFFlowStat* m_flow;
  char* m_evtbuf;
  int* m_buf;

};

