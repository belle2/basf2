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

  addParam("ProcName", m_name, "Name for basf2 process", string("basf2"));
  addParam("RunInfoBufPath", m_buf_path, "File path to run info buffer", string(""));
  addParam("RunLogMessangerPath", m_msg_path, "File path to run log messanger", string(""));

}


RCTestModuleModule::~RCTestModuleModule()
{
}


void RCTestModuleModule::initialize()
{
  m_buf.open(m_buf_path);
  m_msg.open(m_msg_path);
  m_msg.sendLog(SystemLog(m_name, SystemLog::NOTICE, "Ready for start"));
}

void RCTestModuleModule::terminate()
{
  m_msg.sendLog(SystemLog(m_name, SystemLog::NOTICE, "Run terminating"));
}


void RCTestModuleModule::event()
{
}
