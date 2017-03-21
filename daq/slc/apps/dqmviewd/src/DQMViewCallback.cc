#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"

#include <daq/slc/apps/dqmviewd/DQMSocketReader.h>
#include <daq/slc/apps/dqmviewd/SocketAcceptor.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

DQMViewCallback::DQMViewCallback(const NSMNode& node, ConfigFile& config)
  : NSMCallback(1)
{
  setNode(node);
  const std::string host = config.get("dqm.host");
  const int port = config.getInt("dqm.port");
  const int hport = config.getInt("hist.port");
  PThread(new DQMSocketReader(hport, this));
  PThread(new SocketAcceptor(host, port, this));
}

void DQMViewCallback::init(NSMCommunicator&) throw()
{
}

void DQMViewCallback::timeout(NSMCommunicator&) throw()
{
}

