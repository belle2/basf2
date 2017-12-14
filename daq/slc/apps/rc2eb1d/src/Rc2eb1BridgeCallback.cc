#include "daq/slc/apps/rc2eb1d/Rc2eb1BridgeCallback.h"

#include "daq/slc/apps/rc2eb1d/Rc2eb1Callback.h"


using namespace Belle2;

Rc2eb1BridgeCallback::Rc2eb1BridgeCallback(Rc2eb1Callback* callback,
                                           const std::string& name)
{
  setNode(NSMNode(name));
  m_callback = callback;
  //callback->setCallback(this);
}

bool Rc2eb1BridgeCallback::perform(NSMCommunicator& com) throw()
{
  return m_callback->perform(com);
}
