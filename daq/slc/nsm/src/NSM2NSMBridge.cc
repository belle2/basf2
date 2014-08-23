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
  m_callback[1] = callback2;
  const std::string host[2] = {host1, host2};
  const int port[2] = {port1, port2};
  for (int i = 0; i < 2; i++) {
    if (m_callback[i] != NULL && host[i].size() > 0 && port[i] > 0) {
      m_daemon[i] = new NSMNodeDaemon(m_callback[i], host[i], port[i]);
    } else {
      m_daemon[i] = NULL;
    }
  }
}

void NSM2NSMBridge::run() throw()
{
  try {
    const int timeout = m_callback[0]->getTimeout();
    NSMCommunicator* com[2] = {NULL, NULL};
    if (m_daemon[0] != NULL) com[0] = m_daemon[0]->getCommunicator();
    if (m_daemon[1] != NULL) com[1] = m_daemon[1]->getCommunicator();
    while (true) {
      int i = NSMCommunicator::select(timeout, com, 2);
      if (i >= 0 && m_daemon[i] != NULL) {
        m_daemon[i]->push();
      } else {
        for (int i = 0; i < 2; i++) {
          if (m_callback[i] != NULL) {
            m_callback[i]->timeout();
          }
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
