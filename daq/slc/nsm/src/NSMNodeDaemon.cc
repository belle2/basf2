#include "daq/slc/nsm/NSMNodeDaemon.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

NSMNodeDaemon::NSMNodeDaemon(NSMCallback* callback,
                             const std::string host, int port)
  : m_host(host), m_port(port), m_nsm_comm(NULL)
{
  m_callback[0] = callback;
  m_callback[1] = NULL;
  init();
}

NSMNodeDaemon::NSMNodeDaemon(NSMCallback* callback1,
                             NSMCallback* callback2,
                             const std::string host, int port)
  : m_host(host), m_port(port), m_nsm_comm(NULL)
{
  m_callback[0] = callback1;
  m_callback[1] = callback2;
  init();
}

void NSMNodeDaemon::init() throw(NSMHandlerException)
{
  try {
    NSMNode& node(m_callback[0]->getNode());
    m_nsm_comm = new NSMCommunicator();
    if (m_port < 0) {
      m_nsm_comm->init(node);
    } else {
      m_nsm_comm->init(node, m_host, m_port);
    }
    m_nsm_comm->setCallback(m_callback[0]);
    m_nsm_comm->setCallback(m_callback[1]);
    m_callback[0]->init();
    if (m_callback[1] != NULL) m_callback[1]->init();
  } catch (const NSMHandlerException& e) {
    LogFile::fatal("Failed to connect NSM network (%s:%d). "
                   "Terminating process ",
                   m_host.c_str(), m_port);
    delete m_nsm_comm;
    m_nsm_comm = NULL;
    exit(1);
  }
  LogFile::debug("Connected to NSM2 daemon (%s:%d)",
                 m_host.c_str(), m_port);
}

void NSMNodeDaemon::run() throw()
{
  try {
    while (true) {
      if (m_nsm_comm->wait(m_callback[0]->getTimeout())) {
        NSMMessage& msg(m_nsm_comm->getMessage());
        if (!m_callback[0]->perform(msg)) {
          if (m_callback[1] != NULL) {
            m_callback[1]->perform(msg);
          }
        }
      } else {
        m_callback[0]->timeout();
        if (m_callback[1] != NULL) {
          m_callback[1]->timeout();
        }
      }
    }
  } catch (const std::exception& e) {
    LogFile::fatal("NSM node daemon : Caught exception (%s:%d %s). "
                   "Terminate process...",
                   m_host.c_str(), m_port, e.what());
    m_callback[0]->setReply(e.what());
  }
  m_callback[0]->term();
}
