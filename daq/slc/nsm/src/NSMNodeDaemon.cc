#include "daq/slc/nsm/NSMNodeDaemon.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

NSMNodeDaemon::NSMNodeDaemon(NSMCallback* callback,
                             const std::string host, int port)
{
  m_callback = callback;
  try {
    NSMNode& node(m_callback->getNode());
    m_com = new NSMCommunicator(host, port);
    m_com->init(node, host, port);
    if (m_callback != NULL) {
      m_com->setCallback(m_callback);
      m_callback->init();
    }
  } catch (const NSMHandlerException& e) {
    LogFile::fatal("Failed to connect NSM network (%s:%d). "
                   "Terminating process : %s ",
                   host.c_str(), port, e.what());
    exit(1);
  }
  LogFile::debug("Connected to NSM2 daemon (%s:%d)",
                 host.c_str(), port);
}

void NSMNodeDaemon::run() throw()
{
  try {
    while (true) {
      if (m_com->wait(m_callback->getTimeout())) {
        NSMMessage& msg(m_com->getMessage());
        m_callback->setMessage(msg);
        m_callback->perform(msg);
      } else {
        m_callback->timeout();
      }
    }
  } catch (const std::exception& e) {
    LogFile::fatal("NSM node daemon : Caught exception : %s"
                   "Terminate process...", e.what());
    m_callback->setReply(e.what());
  }
  m_callback->term();
}
