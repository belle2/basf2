#include "daq/slc/apps/nsmbridged/NsmbridgeMasterCallback.h"

#include "daq/slc/apps/nsmbridged/NsmbridgeCallback.h"

#include "daq/slc/system/LogFile.h"


using namespace Belle2;

NsmbridgeMasterCallback::NsmbridgeMasterCallback(NsmbridgeCallback* callback)
{
  m_callback = callback;
  LogFile::debug(callback->getNode().getName());
  setNode(callback->getNode());
  callback->setCallback(this);
}

bool NsmbridgeMasterCallback::perform(NSMCommunicator& com) throw()
{
  return m_callback->perform(com);
}
