// revisions
//
// 20191119 nakao - following currently unused functions are removed
// void NSMCommunicator::callContext()
//   handling nsmlib_recv inside a user program is against the usage of NSM2
// const std::string NSMCommunicator::getNodeHost(const std::string& nodename)
//   making hostname visible to application is against the philosophy of NSM2
// const std::string NSMCommunicator::getNodeHost()
//   accessing NSM2 internal structure is against the philosophy of NSM2

#include "daq/slc/nsm/NSMCommunicator.h"

#include <daq/slc/base/TimeoutException.h>
#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMHandlerException.h>
#include <daq/slc/nsm/NSMNotConnectedException.h>

extern "C" {
#include <nsm2/nsm2.h>
#include <nsm2/nsmlib2.h>
#include <nsm2/belle2nsm.h>
}

#include <cmath>
#include <cstdio>
#include <cstring>

#include <sys/select.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <daq/slc/system/LockGuard.h>

#define NSM_DEBUGMODE 1

using namespace Belle2;

NSMCommunicatorList NSMCommunicator::g_comm;
Mutex NSMCommunicator::g_mutex;
Mutex NSMCommunicator::g_mutex_select;

NSMCommunicator& NSMCommunicator::select(double usec)
{
  fd_set fds;
  int ret;
  FD_ZERO(&fds);
  int highest = 0;
  LockGuard lockGuard(g_mutex_select);
  for (NSMCommunicatorList::iterator it = g_comm.begin();
       it != g_comm.end(); it++) {
    NSMCommunicator& com(*(*it));
    if (com.m_nsmc != NULL) {
      FD_SET(com.m_nsmc->sock, &fds);
      if (highest < com.m_nsmc->sock)
        highest = com.m_nsmc->sock;
    }
  }
  while (true) {
    errno = 0;
    if (usec >= 0) {
      double s = 0;
      double us = modf(usec, &s);
      timeval t = {(long)s, (long)(us * 1000000)};
      ret = ::select(highest + 1, &fds, NULL, NULL, &t);
    } else {
      ret = ::select(highest + 1, &fds, NULL, NULL, NULL);
    }
    if (ret != -1 || (errno != EINTR && errno != EAGAIN)) break;
  }
  if (ret < 0) {
    throw (NSMHandlerException("Failed to select"));
  }
  for (NSMCommunicatorList::iterator it = g_comm.begin();
       it != g_comm.end(); it++) {
    NSMCommunicator& com(*(*it));
    if (com.m_nsmc != NULL) {
      if (FD_ISSET(com.m_nsmc->sock, &fds)) {
        com.m_message.read(com.m_nsmc);
        com.m_message.setRequestName();
        b2nsm_context(com.m_nsmc);
        return com;
      }
    }
  }
  throw (TimeoutException("NSMCommunicator::select was timed out"));
}

NSMCommunicator& NSMCommunicator::connected(const std::string& node)
{
  for (NSMCommunicatorList::iterator it = g_comm.begin();
       it != g_comm.end(); it++) {
    NSMCommunicator& com(*(*it));
    if (com.isConnected(node)) return com;
  }
  throw (NSMNotConnectedException("No connection for " + node));
}

bool NSMCommunicator::send(const NSMMessage& msg)
{
  bool sent = false;
  bool alive = false;
#if NSM_PACKAGE_VERSION >= 1914
  LockGuard lockGuard(g_mutex);
  std::string emsg;
  for (NSMCommunicatorList::iterator it = g_comm.begin();
       it != g_comm.end(); it++) {
    NSMCommunicator& com(*(*it));
    const char* node = msg.getNodeName();
    if (com.isConnected(node)) {
      alive = true;
      const char* req = msg.getRequestName();
      if (node != NULL && req != NULL &&
          strlen(node) > 0 && strlen(req) > 0) {
        b2nsm_context(com.m_nsmc);
        if (b2nsm_sendany(node, req, msg.getNParams(), (int*)msg.getParams(),
                          msg.getLength(), msg.getData(), NULL) < 0) {
          emsg = b2nsm_strerror();
        }
        sent = true;
      }
    }
  }
  if (alive && !sent) {
    throw (NSMHandlerException("Failed to send request: " + emsg));
  }
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
  return sent;
}

NSMCommunicator::NSMCommunicator(const std::string& host, int port)
{
  m_id = -1;
  m_nsmc = NULL;
  m_callback = NULL;
  m_host = host;
  m_port = port;
}

NSMCommunicator::NSMCommunicator(NSMcontext* nsmc)
{
  m_nsmc = nsmc;
  m_id = m_nsmc->nodeid;
  m_callback = NULL;
}

void NSMCommunicator::init(const NSMNode& node,
                           const std::string& host, int port)
{
#if NSM_PACKAGE_VERSION >= 1914
  LockGuard lockGuard(g_mutex);
  if (host.size() > 0) m_host = host;
  if (port > 0) m_port = port;
  if (node.getName().size() == 0) {
    throw (NSMHandlerException("Error during init2 (nodename is empty)"));
  }
  m_nsmc = b2nsm_init2(node.getName().c_str(), 0, host.c_str(), port, port);
  if (m_nsmc == NULL) {
    m_id = -1;
    throw (NSMHandlerException("Error during init2 (%s=>%s:%d): %s",
                               node.getName().c_str(), host.c_str(),
                               m_port, b2nsm_strerror()));
  }
  g_comm.push_back(this);
  nsmlib_usesig(m_nsmc, 0);
  m_id = m_nsmc->nodeid;
  m_node = node;
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
}

NSMCallback& NSMCommunicator::getCallback()
{
  if (m_callback) {
    return *m_callback;
  }
  throw (std::out_of_range("No callback was registered"));
}

void NSMCommunicator::setCallback(NSMCallback* callback)
{
  b2nsm_context(m_nsmc);
  if (callback != NULL) {
    m_callback = callback;
    NSMCallback::NSMCommandList& req_v(callback->getCommandList());
    for (NSMCallback::NSMCommandList::iterator it = req_v.begin();
         it != req_v.end(); it++) {
      NSMCommand& command(*it);
      if (b2nsm_callback(command.getLabel(), NULL) < 0) {
        m_id = -1;
        throw (NSMHandlerException("Failed to register callback (%s)",
                                   command.getLabel()));
      }
    }
  }
}

int NSMCommunicator::getNodeIdByName(const std::string& name)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(m_nsmc);
  return b2nsm_nodeid(name.c_str());
#else
  return -1;
#endif
}

const std::string NSMCommunicator::getNodeNameById(int id)
{
  const char* name = nsmlib_nodename(m_nsmc, id);
  if (name == NULL) return "";
  return name;
}

int NSMCommunicator::getNodePidByName(const std::string& name)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(m_nsmc);
  return b2nsm_nodepid(name.c_str());
#else
  return -1;
#endif
}

bool NSMCommunicator::isConnected(const std::string& node)
{
  bool is_online = getNodeIdByName(node) >= 0 &&
                   getNodePidByName(node) > 0;
  return is_online;
}

NSMMessage NSMCommunicator::popQueue()
{
  NSMMessage msg = m_msg_q.front();
  m_msg_q.pop();
  return msg;
}

void NSMCommunicator::setMessage(const NSMMessage& msg)
{
  m_message = msg;
}
