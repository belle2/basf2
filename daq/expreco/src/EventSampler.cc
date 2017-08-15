//+
// File : EventSampler.cc
// Description : Fetch events from event processors and place them in a RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 17 - Dec - 2013
//-

#include "daq/expreco/EventSampler.h"

using namespace Belle2;
using namespace std;

// constructor/destructor

EventSampler::EventSampler(vector<string> nodes, int port, string rbufname, int interval) : m_port(port), m_interval(interval)
{
  // Attach to output RingBuffer
  m_rbuf = new RingBuffer(rbufname.c_str());

  // Open EvtSocket
  for (vector<string>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
    string& nodename = *it;
    printf("EventSampler : connecting to %s (port %d)\n", nodename.c_str(), port);
    EvtSocketSend* sock = new EvtSocketSend(nodename.c_str(), port);
    if (sock == NULL) {
      printf("EventSampler : error to connect to %s\n",
             nodename.c_str());
    } else {
      m_socklist.push_back(sock);
    }
    fflush(stdout);
  }
  printf("EventSampler : init : socklist = %d\n", m_socklist.size());
  fflush(stdout);
}

EventSampler::~EventSampler()
{
  for (vector<EvtSocketSend*>::iterator it = m_socklist.begin();
       it != m_socklist.end(); ++it) {
    EvtSocketSend* sock = *it;
    delete sock;
    m_socklist.erase(it);
  }
  delete m_rbuf;
}

int EventSampler::server()
{
  //  printf ( "EventSampler : server started\n" );
  fflush(stdout);
  int nsample = 0;
  for (;;) {
    for (vector<EvtSocketSend*>::iterator it = m_socklist.begin();
         it != m_socklist.end(); ++it) {
      // Receive an event from connected socket
      EvtSocketSend* sock = *it;
      //      printf ( "EventSampler : receiving event from sock %s\n",
      //         (sock->sock())->node() );
      EvtMessage* msg = sock->recv();
      //      printf ( "Event Sampler : got event : %d\n", msg->size() );
      //      fflush ( stdout );
      if (msg == NULL) {
        printf("EventSampler : Error to receive data\n");
        return -1;
      }
      // Put the message in ring buffer. If full, just skip the event.
      int stat = m_rbuf->insq((int*)msg->buffer(), msg->paddedSize());
      delete msg;
      nsample++;
      if (nsample % 1000 == 0)
        printf("EventSampler : %d events sampled and queued\n", nsample);
      usleep(m_interval);
    }
  }
}

