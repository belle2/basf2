#include "daq/slc/nsm/NSMCommunicator.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

extern "C" {
#include <nsm2/nsmlib2.h>
#include <nsm2/belle2nsm.h>
}

#include <iostream>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#define NSM_DEBUGMODE 1

using namespace Belle2;

NSMCommunicator::NSMCommunicator(const std::string& host, int port)
throw()
{
  m_id = -1;
  m_nsmc = NULL;
  m_host = host;
  m_port = port;
}

void NSMCommunicator::init(const NSMNode& node,
                           const std::string& host, int port)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  if (host.size() > 0) m_host = host;
  if (port > 0) m_port = port;
  if (m_host.size() == 0 || m_port <= 0) {
    m_nsmc = b2nsm_init(node.getName().c_str());
  } else {
    m_nsmc = b2nsm_init2(node.getName().c_str(), 0, host.c_str(), port, port);
  }
  if (m_nsmc == NULL) {
    m_id = -1;
    throw (NSMHandlerException("Error during init2 (%s=>%s:%d): %s",
                               node.getName().c_str(), host.c_str(),
                               m_port, b2nsm_strerror()));
  }
  nsmlib_usesig(m_nsmc, 0);
  //b2nsm_logging(stdout);
  m_id = m_nsmc->nodeid;
  m_node = node;

#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
}

void NSMCommunicator::setCallback(NSMCallback* callback) throw(NSMHandlerException)
{
  b2nsm_context(m_nsmc);
  if (callback != NULL) {
    callback->setCommunicator(this);
    NSMCallback::NSMRequestList& req_v(callback->getRequestList());
    for (NSMCallback::NSMRequestList::iterator it = req_v.begin();
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

void NSMCommunicator::sendRequest(const NSMMessage& msg) throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  const char* nodename = msg.getNodeName();
  const char* reqname = msg.getRequestName();
  if (nodename != NULL && reqname != NULL &&
      strlen(nodename) > 0 && strlen(reqname) > 0) {
    sendRequest(nodename, reqname, msg.getNParams(), msg.getParams(),
                msg.getLength(), msg.getData());
  }
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
}

void NSMCommunicator::sendRequest(const std::string& node, const std::string& cmd,
                                  int npar, const int* pars, int len, const char* data)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(m_nsmc);
  if (b2nsm_sendany(node.c_str(), cmd.c_str(),
                    npar, (int*) pars, len, data, NULL) < 0) {
    m_id = -1;
    throw (NSMHandlerException("Failed to send request: %s", b2nsm_strerror()));
  }
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
}

void NSMCommunicator::replyOK(const NSMNode& node)
throw(NSMHandlerException)
{
  sendRequest(NSMMessage(m_master_node, NSMCommand::OK,
                         node.getState().getLabel()));
}

void NSMCommunicator::replyError(const std::string& message)
throw(NSMHandlerException)
{
  sendRequest(NSMMessage(m_master_node, NSMCommand::ERROR, message));
}

bool NSMCommunicator::sendLog(const SystemLog& log)
{
  return sendLog(m_master_node, log);
}

bool NSMCommunicator::sendLog(const NSMNode& node, const SystemLog& log)
{
#if NSM_PACKAGE_VERSION >= 1914
  try {
    if (node.getName().size() > 0 &&
        b2nsm_nodeid(node.getName().c_str()) >= 0) {
      sendRequest(NSMMessage(node, NSMCommand::LOG, log));
    }
  } catch (const NSMHandlerException& e) {
    return false;
  }
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
  return true;
}

bool NSMCommunicator::sendError(const std::string& message)
{
  try {
    if (m_master_node.getName().size() > 0) {
      sendRequest(NSMMessage(m_master_node, NSMCommand::ERROR, message));
    }
  } catch (const NSMHandlerException& e) {
    return false;
  }
  return true;
}

bool NSMCommunicator::sendFatal(const std::string& message)
{
  try {
    if (m_master_node.getName().size() > 0) {
      sendRequest(NSMMessage(m_master_node, NSMCommand::FATAL, message));
    }
  } catch (const NSMHandlerException& e) {
    return false;
  }
  return true;
}

void NSMCommunicator::sendState(const NSMNode& node) throw(NSMHandlerException)
{
  if (m_master_node.getName().size() > 0) {
    sendRequest(NSMMessage(m_master_node, NSMCommand::STATE,
                           node.getState().getLabel()));
  }
}

bool NSMCommunicator::wait(int sec) throw(NSMHandlerException)
{
  if (m_nsmc == NULL) {
    throw (NSMHandlerException("Not ready for wait"));
  }
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(m_nsmc->sock, &fds);
  int ret;
  while (true) {
    if (sec >= 0) {
      timeval t = {sec, 0};
      ret = ::select(FD_SETSIZE, &fds, NULL, NULL, &t);
    } else {
      ret = ::select(FD_SETSIZE, &fds, NULL, NULL, NULL);
    }
    if (ret != -1 || (errno != EINTR && errno != EAGAIN)) break;
  }
  if (ret < 0) {
    perror("select");
    throw (NSMHandlerException("Failed to select"));
  }
  if (FD_ISSET(m_nsmc->sock, &fds)) {
    m_message.read(m_nsmc);
    m_message.setRequestName();
    const char* master_name = m_message.getNodeName();
    NSMCommand cmd(m_message.getRequestName());
    if (m_master_node.getName().size() == 0 &&
        cmd != NSMCommand::OK && cmd != NSMCommand::ERROR &&
        strlen(master_name) > 0) {
      m_master_node = NSMNode(master_name);
    }
    b2nsm_context(m_nsmc);
    return true;
  } else {
    return false;
  }
}

void NSMCommunicator::setContext(NSMcontext* nsmc) throw(NSMHandlerException)
{
  if (m_nsmc != NULL) {
    throw (NSMHandlerException("NSM is already available"));
  }
  if (nsmc != NULL) {
    m_nsmc = nsmc;
    m_id = m_nsmc->nodeid;
  }
}

int NSMCommunicator::getNodeIdByName(const std::string& name)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(m_nsmc);
  return b2nsm_nodeid(name.c_str());
#else
  return -1;
#endif
}

int NSMCommunicator::getNodePidByName(const std::string& name)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(m_nsmc);
  return b2nsm_nodepid(name.c_str());
#else
  return -1;
#endif
}

bool NSMCommunicator::isConnected(const NSMNode& node) throw()
{
  bool is_online = getNodeIdByName(node.getName()) >= 0 &&
                   getNodePidByName(node.getName()) > 0;
  return is_online;
}
