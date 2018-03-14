#include "daq/slc/apps/hvmasterd/HVMasterMasterCallback.h"

#include "daq/slc/apps/hvmasterd/HVMasterCallback.h"

using namespace Belle2;

HVMasterMasterCallback::HVMasterMasterCallback(HVMasterCallback* callback)
{
  m_callback = callback;
  //callback->setCallback(this);
}

bool HVMasterMasterCallback::perform(NSMCommunicator& com) throw()
{
  return m_callback->perform(com);
}
