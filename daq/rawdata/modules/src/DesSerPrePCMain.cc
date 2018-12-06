//+
// File : DAQmain.cc
// Description : Base class for Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/DesSerPrePCMain.h>
//#include <daq/rawdata/modules/DesSerPrePC.h>
#include <daq/rawdata/DesSerPrePC.h>

//#define MAXEVTSIZE 400000000

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DesSerPrePCMain)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------



DesSerPrePCMainModule::DesSerPrePCMainModule() : Module()
{
  addParam("PortFrom", m_port_recv, "port number", 30000);
  addParam("HostNameFrom", m_host_recv, "hostname from", string("cpr001"));

  addParam("PortTo", m_port_send, "port number", 31001);
  addParam("HostNameTo", m_host_send, "hostname from", string("0.0.0.0."));

  addParam("NodeID", m_nodeid, "Node(subsystem) ID", 0);
  addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));
  addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);

  m_compressionLevel = 0;
}


DesSerPrePCMainModule::~DesSerPrePCMainModule()
{
}


void DesSerPrePCMainModule::initialize()
{
  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();
  // Open message handler



}

void DesSerPrePCMainModule::event()
{
  DesSerPrePC des_ser_prepc(m_host_recv, m_port_recv,
                            m_host_send, m_port_send, m_shmflag,
                            m_nodename, m_nodeid);
  des_ser_prepc.DataAcquisition();


  //
  // Update EventMetaData
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(1);

}

