//+
// File : DAQmain.cc
// Description : Base class for Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
//#include <daq/rawdata/modules/DAQConsts.h>
//#include <daq/rawdata/des_ser_COPPER_main.h>
#include "des_ser_COPPER_main.h"
//#include <daq/rawdata/modules/DesSerPrePC.h>
//#include <daq/rawdata/DesSerPrePC.h>
#include <daq/rawdata/DesSerCOPPER.h>
#include <unistd.h>
//#define MAXEVTSIZE 400000000

using namespace std;
using namespace Belle2;

//RunInfoBuffer DesSerCOPPER::g_status;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


// DesSerPrePCMainModule::DesSerPrePCMainModule() : Module()
// {
//   addParam("PortFrom", m_port_recv, "port number", 33000);
//   addParam("HostNameFrom", m_host_recv, "hostname from", string("cpr001"));

//   addParam("PortTo", m_port_send, "port number", 34001);
//   addParam("HostNameTo", m_host_send, "hostname from", string("0.0.0.0."));

//   addParam("NodeID", m_nodeid, "Node(subsystem) ID", 0);
//   addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));
//   addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);

//   m_compressionLevel = 0;
//   m_msghandler = new MsgHandler(0);

// }


// DesSerPrePCMainModule::~DesSerPrePCMainModule()
// {
// }


// void DesSerPrePCMainModule::initialize()
// {
//   // Initialize EvtMetaData
//   m_eventMetaDataPtr.registerInDataStore();
//   // Open message handler



// }


//namespace Belle2 {

//void DesSerPrePCMainModule::event()
int main(int argc, char** argv)
{

  if (argc != 6) {
    printf("[FATAL] Usage : %s <COPPER hostname> <COPPER node ID> <bit flag of FINNESEs> <Use NSM(Network Shared Memory)? yes=1/no=0> <NSM nodename>",
           argv[0]);
    //      printf("Usage : %s <COPPER hostname> <Use NSM(Network Shared Memory)? yes=1/no=0> <port # to eb0> <NSM nodename>", argv[0]);;
    exit(1);
  }

  string host_recv = argv[1];     //! hostname
  int shmflag = atoi(argv[4]);      //! Use shared memory -> 1; Without shm -> 0
  int port_send = 33000;      //! port number
  //    int port_send = atoi(argv[3]);      //! port number
  string nodename = argv[5];    //! Node name
  string host_send = "0.0.0.0";    //! hostname
  int port_recv = 33000;    //! port number
  int nodeid = atoi(argv[2]);
  int finesse_bitflag = atoi(argv[3]);
  DesSerCOPPER des_ser_copper(host_recv, port_recv,
                              host_send, port_send, shmflag,
                              nodename, nodeid, finesse_bitflag);
  des_ser_copper.DataAcquisition();


  //
  // Update EventMetaData
  //
//   m_eventMetaDataPtr.create();
//   m_eventMetaDataPtr->setExperiment(1);
//   m_eventMetaDataPtr->setRun(1);
//   m_eventMetaDataPtr->setEvent(1);

}
//}

