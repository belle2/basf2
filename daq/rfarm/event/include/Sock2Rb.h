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
#include "daq/dataflow/EvtSocket.h"

#define RBUFSIZE 100000000

class Sock2Rb {
public:
  /*! Constuctor and Destructor */
  Sock2Rb(std::string& rbuf, int port);
  ~Sock2Rb(void);

  /*! Event function */
  int ReceiveEvent(void);

private:
  RingBuffer* m_rbuf;
  EvtSocketRecv* m_sock;
  char* m_evtbuf;

};

