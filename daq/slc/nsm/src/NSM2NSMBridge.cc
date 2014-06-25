#include "daq/slc/nsm/NSM2NSMBridge.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

NSM2NSMBridge::NSM2NSMBridge(NSMCallback* callback1,
                             const std::string host1, int port1,
                             NSMCallback* callback2,
                             const std::string host2, int port2)
{
  m_callback[0] = callback1;
  m_host[0] = host1;
  m_port[0] = port1;
  m_nsm_comm[0] = NULL;
  m_callback[1] = callback2;
  m_host[1] = host2;
  m_port[1] = port2;
  m_nsm_comm[1] = NULL;
  init();
}

void NSM2NSMBridge::init() throw(NSMHandlerException)
{
  for (int i = 0; i < 2; i++) {
    try {
      NSMNode& node(m_callback[i]->getNode());
      m_nsm_comm[i] = new NSMCommunicator();
      if (m_port[i] > 0 && m_callback != NULL) {
        m_nsm_comm[i]->init(node, m_host[i], m_port[i]);
        m_nsm_comm[i]->setCallback(m_callback[i]);
        m_callback[i]->init();
      }
    } catch (const NSMHandlerException& e) {
      LogFile::fatal("Failed to connect NSM network (%s:%d). "
                     "Terminating process ",
                     m_host[i].c_str(), m_port[i]);
      delete m_nsm_comm[i];
      m_nsm_comm[i] = NULL;
      exit(1);
    }
    LogFile::debug("Connected to NSM2 daemon (%s:%d)",
                   m_host[i].c_str(), m_port[i]);
  }
}

void NSM2NSMBridge::run() throw()
{
  try {
    while (true) {
      int timeout = m_callback[0]->getTimeout();
      int i = NSMCommunicator::select(timeout, m_nsm_comm, 2);
      if (i >= 0) {
        NSMMessage& msg(m_nsm_comm[i]->getMessage());
        m_callback[i]->perform(msg);
      } else {
        for (int i = 0; i < 2; i++) {
          m_callback[i]->timeout();
        }
      }
    }
  } catch (const std::exception& e) {
    LogFile::fatal("NSM node brdge : Caught exception %s\n"
                   "Terminate process...", e.what());
  }
  for (int i = 0; i < 2; i++) {
    if (m_callback[i] != NULL) m_callback[i]->term();
  }
}
