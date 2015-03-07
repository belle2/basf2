#include "daq/slc/apps/runcontrold/RunControlMasterCallback.h"

#include "daq/slc/apps/runcontrold/RunControlCallback.h"


using namespace Belle2;

RunControlMasterCallback::RunControlMasterCallback(RunControlCallback* callback)
{
  m_callback = callback;
  callback->setCallback(this);
}

bool RunControlMasterCallback::perform(NSMCommunicator& com) throw()
{
  return m_callback->perform(com);
}
