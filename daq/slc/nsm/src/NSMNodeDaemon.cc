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
                   "Terminating process ",
                   host.c_str(), port);
    delete m_com;
    m_com = NULL;
    exit(1);
  }
  PThread(new Handler(this, callback));
  LogFile::debug("Connected to NSM2 daemon (%s:%d)",
                 host.c_str(), port);
}

void NSMNodeDaemon::run() throw()
{
  try {
    while (true) {
      if (m_com->wait(m_callback->getTimeout())) {
        push();
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


void NSMNodeDaemon::push()
{
  push(m_com->getMessage());
}

void NSMNodeDaemon::push(const NSMMessage& msg)
{
  m_mutex.lock();
  m_msg_q.push(msg);
  m_cond.signal();
  m_mutex.unlock();
}

void NSMNodeDaemon::pop(NSMMessage& msg)
{
  m_mutex.lock();
  if (m_msg_q.empty()) {
    m_cond.wait(m_mutex);
  }
  msg = m_msg_q.front();
  m_msg_q.pop();
  m_mutex.unlock();
}

void NSMNodeDaemon::Handler::run()
{
  NSMMessage msg;
  while (true) {
    m_daemon->pop(msg);
    m_callback->perform(msg);
  }
}
