#include "daq/slc/nsm/NSMCommunicator.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/runcontrol/RCCommand.h>

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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
  m_nsmc = b2nsm_init2(node.getName().c_str(), 0, host.c_str(), port, port);
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

void NSMCommunicator::sendRequest(const NSMMessage& msg)
throw(NSMHandlerException)
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
  for (NSMNodeList::iterator it = m_masters.begin();
       it != m_masters.end(); it++) {
    //if (b2nsm_nodeid(it->first.c_str()) >= 0) {
    try {
      LogFile::debug("OK >> %s (state = %s)", it->second.getName().c_str(),
                     node.getState().getLabel());
      sendRequest(NSMMessage(it->second, NSMCommand::OK,
                             node.getState().getLabel()));
    } catch (const NSMHandlerException& e) {
      //}
      //} else {
      LogFile::debug("%s is not online. ", it->first.c_str());
    }
  }
}

void NSMCommunicator::sendExclude(const NSMNode& node)
throw(NSMHandlerException)
{
  for (NSMNodeList::iterator it = m_masters.begin();
       it != m_masters.end(); it++) {
    //if (b2nsm_nodeid(it->first.c_str()) >= 0) {
    try {
      LogFile::debug("EXCLUDE >> %s", it->second.getName().c_str());
      sendRequest(NSMMessage(it->second, NSMCommand::EXCLUDE,
                             node.getName()));
    } catch (const NSMHandlerException& e) {
      //}
      //} else {
      LogFile::debug("%s is not online. ", it->first.c_str());
    }
  }
}

void NSMCommunicator::sendInclude(const NSMNode& node)
throw(NSMHandlerException)
{
  for (NSMNodeList::iterator it = m_masters.begin();
       it != m_masters.end(); it++) {
    //if (b2nsm_nodeid(it->first.c_str()) >= 0) {
    try {
      LogFile::debug("INCLUDE >> %s", it->second.getName().c_str());
      sendRequest(NSMMessage(it->second, NSMCommand::INCLUDE,
                             node.getName()));
    } catch (const NSMHandlerException& e) {
      //}
      //} else {
      LogFile::debug("%s is not online. ", it->first.c_str());
    }
  }
}

void NSMCommunicator::replyError(int error, const std::string& message)
throw(NSMHandlerException)
{
  for (NSMNodeList::iterator it = m_masters.begin();
       it != m_masters.end(); it++) {
    //if (b2nsm_nodeid(it->first.c_str()) >= 0) {
    try {
      sendRequest(NSMMessage(it->second, NSMCommand::ERROR,
                             error, message));
    } catch (const NSMHandlerException& e) {
      LogFile::debug("%s is not online. ", it->first.c_str());
      //}
    }
  }
}

bool NSMCommunicator::sendLog(const DAQLogMessage& log, bool recored)
{
  for (NSMNodeList::iterator it = m_masters.begin();
       it != m_masters.end(); it++) {
    //if (b2nsm_nodeid(it->first.c_str()) >= 0) {
    try {
      sendLog(it->second, log, recored);
    } catch (const NSMHandlerException& e) {
      LogFile::debug("%s is not online. ", it->first.c_str());
      //}
    }
  }
  return true;
}

bool NSMCommunicator::sendLog(const NSMNode& node,
                              const DAQLogMessage& log,
                              bool recoreded)
{
#if NSM_PACKAGE_VERSION >= 1914
  try {
    if (node.getName().size() > 0
        /*&& b2nsm_nodeid(node.getName().c_str()) >= 0*/) {
      NSMMessage msg(node, NSMCommand::LOG);
      msg.setNParams(3);
      msg.setParam(0, (int)log.getPriority());
      msg.setParam(1, log.getDateInt());
      msg.setParam(2, (recoreded) ? 1 : 0);
      msg.setData(log.getNodeName() + "\n" + log.getMessage());
      sendRequest(msg);
    }
  } catch (const NSMHandlerException& e) {
    LogFile::debug("%s is not online. ", node.getName().c_str());
    return false;
  }
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
  return true;
}

bool NSMCommunicator::sendError(const std::string& message)
{
  int error = 255;
  return sendError(error, message);
}

bool NSMCommunicator::sendError(int error,
                                const std::string& message)
{
  for (NSMNodeList::iterator it = m_masters.begin();
       it != m_masters.end(); it++) {
    //if (b2nsm_nodeid(it->first.c_str()) >= 0) {
    try {
      sendRequest(NSMMessage(it->second, NSMCommand::ERROR, error, message));
    } catch (const NSMHandlerException& e) {
      LogFile::debug("%s is not online. ", it->first.c_str());
      //}
    }
  }
  return true;
}

bool NSMCommunicator::sendFatal(const std::string& message)
{
  for (NSMNodeList::iterator it = m_masters.begin();
       it != m_masters.end(); it++) {
    //if (b2nsm_nodeid(it->first.c_str()) >= 0) {
    try {
      sendRequest(NSMMessage(it->second, NSMCommand::FATAL, message));
    } catch (const NSMHandlerException& e) {
      LogFile::debug("%s is not online. ", it->first.c_str());
      //}
    }
  }
  return true;
}

void NSMCommunicator::sendState(const NSMNode& node) throw(NSMHandlerException)
{
  std::string text = StringUtil::form("%s %s", node.getName().c_str(),
                                      node.getState().getLabel());
  for (NSMNodeList::iterator it = m_masters.begin();
       it != m_masters.end(); it++) {
    //if (b2nsm_nodeid(it->first.c_str()) >= 0) {
    try {
      LogFile::debug("STATE >> %s", it->second.getName().c_str());
      sendRequest(NSMMessage(it->second, NSMCommand::STATE, text));
    } catch (const NSMHandlerException& e) {
      LogFile::debug("%s is not online. ", it->first.c_str());
      //}
    }
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
    readContext(m_nsmc);
    b2nsm_context(m_nsmc);
    return true;
  } else {
    return false;
  }
}

int NSMCommunicator::select(int sec, NSMCommunicator** com, int ncoms)
throw(NSMHandlerException)
{
  fd_set fds;
  int ret;
  FD_ZERO(&fds);
  for (int i = 0; i < ncoms; i++) {
    if (com[i] != NULL && com[i]->m_nsmc != NULL) {
      FD_SET(com[i]->m_nsmc->sock, &fds);
    }
  }
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

  for (int i = 0; i < ncoms; i++) {
    if (com[i] != NULL && com[i]->m_nsmc != NULL) {
      if (FD_ISSET(com[i]->m_nsmc->sock, &fds)) {
        com[i]->readContext(com[i]->m_nsmc);
        b2nsm_context(com[i]->m_nsmc);
        return i;
      }
    }
  }
  return -1;
}

void NSMCommunicator::readContext(NSMcontext* nsmc) throw()
{
  if (nsmc == NULL || nsmc == m_nsmc) {
    m_message.read(m_nsmc);
    m_message.setRequestName();
    const char* nodename = m_message.getNodeName();
    NSMCommand cmd(m_message.getRequestName());
    if (cmd != NSMCommand::OK && cmd != NSMCommand::ERROR &&
        cmd != NSMCommand::LOG && cmd != RCCommand::STATE &&
        strlen(nodename) > 0 && m_masters.find(nodename) == m_masters.end()) {
      m_masters.insert(NSMNodeList::value_type(nodename, NSMNode(nodename)));
    }
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

bool NSMCommunicator::isConnected(const std::string& node) throw()
{
  bool is_online = getNodeIdByName(node) >= 0 &&
                   getNodePidByName(node) > 0;
  return is_online;
}

const std::string NSMCommunicator::getNodeHost(const std::string& nodename)
throw()
{
#if NSM_PACKAGE_VERSION >= 1914
  NSMsys* sys = m_nsmc->sysp;
  for (int inod = 0; inod < NSMSYS_MAX_NOD; inod++) {
    NSMnod& nod(sys->nod[inod]);
    if (! nod.name[0]) continue;
    if (nodename == nod.name) {
      sockaddr_in addr;
      addr.sin_addr.s_addr = nod.ipaddr;
      return inet_ntoa(addr.sin_addr);
    }
  }
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
  return "";
}

const std::string NSMCommunicator::getNodeHost() throw()
{
#if NSM_PACKAGE_VERSION >= 1914
  if (!getMessage().getMsg()) return "";
  unsigned int nodeid = getMessage().getMsg()->node;
  NSMsys* sys = m_nsmc->sysp;
  if (nodeid < NSMSYS_MAX_NOD) {
    NSMnod& nod(sys->nod[nodeid]);
    if (!nod.name[0]) return "";
    sockaddr_in addr;
    addr.sin_addr.s_addr = nod.ipaddr;
    return inet_ntoa(addr.sin_addr);
  }
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
  return "";
}

//const std::string NSMCommunicator::getNodeHost() throw()
//{
//  return getNodeHost(m_master_node.getName());
//}
