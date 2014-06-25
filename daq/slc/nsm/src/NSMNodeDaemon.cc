#include "daq/slc/nsm/NSMNodeDaemon.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

NSMNodeDaemon::NSMNodeDaemon(NSMCallback* callback,
                             const std::string host, int port)
{
  m_callback = callback;
  m_host = host;
  m_port = port;
  m_nsm_comm = NULL;
  init();
}

void NSMNodeDaemon::init() throw(NSMHandlerException)
{
  try {
    NSMNode& node(m_callback->getNode());
    m_nsm_comm = new NSMCommunicator();
    if (m_port < 0) {
      m_nsm_comm->init(node);
    } else {
      m_nsm_comm->init(node, m_host, m_port);
    }
    if (m_callback != NULL) {
      m_nsm_comm->setCallback(m_callback);
      m_callback->init();
    }
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
      if (m_nsm_comm->wait(m_callback->getTimeout())) {
        NSMMessage& msg(m_nsm_comm->getMessage());
        m_callback->perform(msg);
      } else {
        m_callback->timeout();
      }
    }
  } catch (const std::exception& e) {
    LogFile::fatal("NSM node daemon : Caught exception (%s:%d %s). "
                   "Terminate process...",
                   m_host.c_str(), m_port, e.what());
    m_callback->setReply(e.what());
  }
  m_callback->term();
}
