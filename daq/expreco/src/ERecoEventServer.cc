//+
// File : ERecoEventServer.cc
// Description : Event server to send sampled events to outside
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 17 - Dec - 2013
//-

#include "daq/expreco/ERecoEventServer.h"

using namespace Belle2;
using namespace std;

// constructor/destructor

ERecoEventServer::ERecoEventServer(string rbufname, int port) : m_port(port)
{
  // Attach to RingBuffer
  m_rbuf = new RingBuffer(rbufname.c_str());

  // Open EvtSocket
  m_sock = new EvtSocketRecv(m_port, false);
  m_man = new EvtSocketManager(m_sock);
}

ERecoEventServer::~ERecoEventServer()
{
  delete m_rbuf;
  delete m_sock;
  delete m_man;
}

int ERecoEventServer::server()
{
  SocketIO sio;
  MsgHandler msghdl(0);
  char* evtbuffer = new char[MAXEVTSIZE];

  //  vector<int> recvsock;
  int loop_counter = 0;
  while (m_force_exit == 0) {
    // Pick up a event from RingBuffer (non blocking)
    int size = m_rbuf->remq((int*)evtbuffer);

    // Check connection request
    int exam_stat = m_man->examine();
    if (exam_stat == 0) {
      printf("Initial connection request detected!\n");
      //      int fd = recvsock[;
    } else if (exam_stat == 1 && size > 0) { //
      // printf ( "Event data data ready on socket\n" );
      vector<int>& recvsock = m_man->connected_socket_list();
      for (vector<int>::iterator it = recvsock.begin();
           it != recvsock.end(); ++it) {
        int fd = *it;
        if (m_man->connected(fd)) {
          int is = sio.put(fd, evtbuffer, size * 4);
          if (is <= 0) {
            printf("EventServer: fd %d disconnected\n", fd);
            m_man->remove(fd);
            break;
          }
        }
      }
    }
    usleep(1000);
    loop_counter++;
  }
  return 0;
}

