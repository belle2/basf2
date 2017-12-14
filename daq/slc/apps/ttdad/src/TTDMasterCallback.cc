#include "daq/slc/apps/ttdad/TTDMasterCallback.h"

#include "daq/slc/apps/ttdad/TTDACallback.h"


using namespace Belle2;

TTDMasterCallback::TTDMasterCallback(TTDACallback* callback)
{
  m_callback = callback;
}

bool TTDMasterCallback::perform(NSMCommunicator& com) throw()
{
  return m_callback->perform(com);
}
