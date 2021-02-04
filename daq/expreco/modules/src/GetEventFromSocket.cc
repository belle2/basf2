//+
// File : GetEventFromSocket.cc
// Description : Module to receive event from expreco
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 17 - Dec - 2013
//-

#include <daq/expreco/modules/GetEventFromSocket.h>

#include <TSystem.h>

#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

//#define MAXEVTSIZE 400000000

#define TIME_WAIT 10000

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GetEventFromSocket)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GetEventFromSocketModule::GetEventFromSocketModule() : Module()
{
  //Set module properties
  setDescription("Get Event from EvtSocket");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  vector<string> emptyhosts;
  addParam("Hosts", m_hosts, "GetEventFromSocket hosts", emptyhosts);
  vector<int> emptysocks;
  addParam("Ports", m_ports, "GetEventFromSocket Ports", emptysocks);
  m_nrecv = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


GetEventFromSocketModule::~GetEventFromSocketModule()
{
}

void GetEventFromSocketModule::initialize()
{
  // Check event sources are not empty
  if (m_hosts.size() == 0) {
    B2FATAL("No hosts specified");
  }

  // Load data objects definitions
  gSystem->Load("libdataobjects");

  // Open receiving socekt
  for (int i = 0; i < (int)m_hosts.size(); i++) {
    //    printf("Connecting to %s (port %d)\n", m_hosts[i].c_str(), m_ports[i]);
    B2INFO("Connecting to " << m_hosts[i] << "(port " << m_ports[i] << ")");
    EvtSocketSend* evtsock = new EvtSocketSend(m_hosts[i].c_str(), m_ports[i]);
    m_socks.push_back(evtsock);
  }
  printf("All hosts connected\n");

  // Initialize DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  // Prefetch first record in GetEventFromSocket
  EvtMessage* msg = receive();
  if (msg == NULL) {
    B2FATAL("Did not receive any data, stopping initialization.");
    return;
  }
  m_streamer->restoreDataStore(msg);

  // Delete buffers
  delete msg;

  m_nrecv = -1;

  B2INFO("Rx initialized.");
}

EvtMessage* GetEventFromSocketModule::receive()
{
  //  printf("GetEventFromSocket:receive here!\n");
  B2DEBUG(1001, "GetEventFromSocket:receive here!");

  fd_set fds;
  fd_set readfds;
  FD_ZERO(&readfds);
  int maxfd = 0;
  for (int i = 0; i < (int)m_socks.size(); i++) {
    int sockfd = (m_socks[i]->sock())->sock();
    FD_SET(sockfd, &readfds);
    if (sockfd > maxfd) maxfd = sockfd;
  }

  while (1) {
    // Look at socket
    memcpy(&fds, &readfds, sizeof(fd_set));
    // select() with blocking until data is ready on socket
    int selstat = select(maxfd + 1, &fds, NULL, NULL, NULL);
    // Check data on the socket or not
    for (int i = 0; i < (int)m_socks.size(); i++) {
      int sockfd = (m_socks[i]->sock())->sock();
      if (FD_ISSET(sockfd, &fds)) {
        EvtMessage* msg = m_socks[i]->recv();
        return msg;
      }
    }
    //    printf("Select loop : should not come here. selstat = %d\n", selstat);
    B2ERROR("Select loop : should not come here. selstat = " << selstat);
    usleep(TIME_WAIT);
  }
}

void GetEventFromSocketModule::beginRun()
{
  B2INFO("beginRun called.");
}


void GetEventFromSocketModule::event()
{
  m_nrecv++;
  // First event is already loaded
  if (m_nrecv == 0) return;

  // Get a record from socket
  EvtMessage* msg = receive();
  B2INFO("Rx: got an event from Socket, size=" << msg->size());
  // Check for termination record
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Rx: got termination message. Exitting....");
    return;
    // Flag End Of File !!!!!
    //    return msg->type(); // EOF
  }

  // Restore DataStore
  m_streamer->restoreDataStore(msg);
  B2INFO("Rx: DataStore Restored!!");

  // Delete EvtMessage
  delete msg;

  return;
}

void GetEventFromSocketModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void GetEventFromSocketModule::terminate()
{
  delete m_streamer;
  B2INFO("terminate called");
}

