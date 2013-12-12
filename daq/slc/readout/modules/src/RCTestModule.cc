//+
// File : RCTestModule.cc
// Description : Base class for Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/slc/readout/modules/RCTestModule.h>

#include <daq/rawdata/modules/DAQConsts.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RCTestModule)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RCTestModuleModule::RCTestModuleModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");

  addParam("NodeName", m_name, "Name for basf2 process", string("basf2"));
  addParam("NodeId", m_id, "ID for basf2 process", 0);
}


RCTestModuleModule::~RCTestModuleModule()
{
}


void RCTestModuleModule::initialize()
{
  m_msg.setNode(m_name, m_id);
  m_msg.open();
  m_msg.reportReady();
  m_running = false;
}

void RCTestModuleModule::terminate()
{
  m_msg.reportReady();
}


void RCTestModuleModule::event()
{
  if (!m_running) {
    m_msg.reportRunning();
    m_running = true;
  }
  if (false) {  // if error
    m_msg.reportError("some trouble");
  }
  if (false) {  // if critical error
    m_msg.reportFatal("some critical trouble");
  }
}
