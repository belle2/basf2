#include "daq/slc/apps/rocontrold/EB1TXController.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

EB1TXController::~EB1TXController() throw()
{
}

void EB1TXController::initArguments(const DBObject& obj)
{
  const DBObject& o_eb1tx(obj("eb1tx"));
  const DBObjectList& o_rxs(o_eb1tx.getObjects("rx"));
  int nrxs = o_rxs.size();
  m_callback->add(new NSMVHandlerInt("eb1tx.nrxs", true, false, nrxs));
}

bool EB1TXController::loadArguments(const DBObject& obj)
{
  const DBObject& o_eb1tx(obj("eb1tx"));
  if (o_eb1tx.getBool("xinetd")) {
    setUsed(false);
    return false;
  }
  std::string executable = o_eb1tx.getText("executable");
  std::string host = o_eb1tx.getText("host");
  int port = o_eb1tx.getInt("port");
  const DBObjectList& o_rxs(o_eb1tx.getObjects("rx"));
  setUsed(true);
  m_con.setExecutable(executable);
  m_con.addArgument("-e");
  m_con.addArgument("%s:%d", host.c_str(), port);
  m_con.addArgument("-i");
  m_con.addArgument(o_rxs.size());
  m_con.addArgument("-l");
  for (DBObjectList::const_iterator i = o_rxs.begin();
       i != o_rxs.end(); i++) {
    std::string host = i->getText("host");
    int port = i->getInt("port");
    //m_con.addArgument("%s:%d", host.c_str(), port);
    m_con.addArgument(port);
    //const std::string nodename = StringUtil::tolower(m_callback->getNode().getName());
    //m_con.addArgument(nodename+"_"+m_name);
  }
  return true;
}
