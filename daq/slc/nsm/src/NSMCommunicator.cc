#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/ConfigFile.h"
#include "daq/slc/base/Date.h"

extern "C" {
#include "nsm2/nsmlib2.h"
#include "nsm2/belle2nsm.h"
}

#include <iostream>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

using namespace Belle2;

NSMCommunicator::NSMCommunicator(NSMNode* node, const std::string& host, int port)
throw() : _node(node), _callback(NULL)
{
  _id = -1;
  _nsmc = NULL;
  _host = host;
  _port = port;
  _rc_node = NULL;
}

void NSMCommunicator::init(const std::string& host, int port) throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  if (_node == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "No node for NSM was registered!"));
  }
  if (host.size() > 0) _host = host;
  if (port > 0) _port = port;
  if (_host.size() == 0 || _port <= 0) {
    _nsmc = b2nsm_init(_node->getName().c_str());
  } else {
    _nsmc = b2nsm_init2(_node->getName().c_str(), 0, host.c_str(), port, port);
  }
  if (_nsmc == NULL) {
    _id = -1;
    throw (NSMHandlerException(__FILE__, __LINE__,
                               Belle2::form("Error during init2 (%s=>%s:%d): %s",
                                            _node->getName().c_str(), host.c_str(), _port,
                                            b2nsm_strerror())));
  }
  nsmlib_usesig(_nsmc, 0);
  //b2nsm_logging(stdout);
  _id = _nsmc->nodeid;

  b2nsm_context(_nsmc);
  if (_callback != NULL) {
    _callback->setCommunicator(this);
    NSMCallback::NSMRequestList& req_v(_callback->getRequestList());
    for (NSMCallback::NSMRequestList::iterator it = req_v.begin();
         it != req_v.end(); it++) {
      Command& command(it->cmd);
      if ((it->id = b2nsm_callback(command.getLabel(), NULL)) < 0) {
        it->id = -1;
        _id = -1;
        throw (NSMHandlerException(__FILE__, __LINE__,
                                   Belle2::form("Failed to register callback (%s)",
                                                command.getLabel())));
      }
    }
  }
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
}

void NSMCommunicator::sendRequest(const NSMNode* node, const Command& command,
                                  int npar, int* pars,
                                  int len, const char* datap) throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(_nsmc);
  if (b2nsm_sendany(node->getName().c_str(), command.getLabel(),
                    npar, (int*)pars, len, datap, NULL) < 0) {
    _id = -1;
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to send request"));
  }
#endif
}

void NSMCommunicator::sendRequest(const NSMNode* node, const Command& cmd,
                                  int npar, int* pars, const Serializable& obj)
throw(NSMHandlerException)
{
  _writer.seekTo(0);
  _writer.writeObject(obj);
  sendRequest(node, cmd, npar, pars, (int)_writer.count(), (const char*)_writer.ptr());
}

void NSMCommunicator::sendRequest(const NSMNode* node, const Command& cmd,
                                  int npar, int* pars, const std::string& message)
throw(NSMHandlerException)
{
  sendRequest(node, cmd, npar, pars, message.size(),
              (message.size() == 0) ? NULL : message.c_str());
}

void NSMCommunicator::sendRequest(const NSMNode* node, const Command& cmd,
                                  NSMMessage& message)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  sendRequest(node, cmd, message.getNParams(), message.getParams(),
              message.getLength(),
              (message.getLength() == 0) ? NULL : message.getData());
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
}

void NSMCommunicator::sendRequest(const NSMNode* node, const Command& cmd,
                                  const std::string& message)
throw(NSMHandlerException)
{
  sendRequest(node, cmd, 0, NULL, message.size(), message.c_str());
}

void NSMCommunicator::replyOK(const NSMNode*, const std::string& message)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(_nsmc);
  if (b2nsm_ok(_message.getMsg(), _node->getState().getLabel(), message.c_str()) < 0) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to reply OK"));
  }
#endif
}

void NSMCommunicator::replyError(const std::string& message)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(_nsmc);
  if (b2nsm_error(_message.getMsg(), message.c_str()) < 0) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to reply error"));
  }
#endif
}

bool NSMCommunicator::sendLog(const SystemLog& log)
{
#if NSM_PACKAGE_VERSION >= 1914
  try {
    if (_rc_node != NULL &&
        b2nsm_nodeid(_rc_node->getName().c_str()) >= 0) {
      std::string str;
      int pars[3];
      int npar = log.pack(pars, str);
      sendRequest(_rc_node, Command::LOG, npar, pars, str);
    }
  } catch (const NSMHandlerException& e) {
    return false;
  }
#endif
  return true;
}

bool NSMCommunicator::sendError(const std::string& message)
{
#if NSM_PACKAGE_VERSION >= 1914
  try {
    _node->setState(State::ERROR_ES);
    if (_rc_node != NULL) {
      sendRequest(_rc_node, Command::ERROR, 0, NULL, message);
    }
  } catch (const NSMHandlerException& e) {
    return false;
  }
  return true;
#endif
}

bool NSMCommunicator::sendFatal(const std::string& message)
{
#if NSM_PACKAGE_VERSION >= 1914
  try {
    _node->setState(State::FATAL_ES);
    if (_rc_node != NULL) {
      sendRequest(_rc_node, Command::FATAL, 0, NULL, message);
    }
  } catch (const NSMHandlerException& e) {
    return false;
  }
  return true;
#endif
}

void NSMCommunicator::sendState() throw(NSMHandlerException)
{
  if (_rc_node != NULL) {
    sendRequest(_rc_node, Command::OK, 0, NULL, _node->getState().getLabel());
  }
}

bool NSMCommunicator::wait(int sec) throw(NSMHandlerException)
{
  if (_nsmc == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Not ready for wait"));
  }
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(_nsmc->sock, &fds);
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
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to select"));
  }
  if (FD_ISSET(_nsmc->sock, &fds)) {
    _message.read(_nsmc);
    const char* rcname = _message.getNodeName();
    if (rcname != NULL && _rc_node == NULL) {
      _rc_node = new NSMNode(rcname);
    }
    b2nsm_context(_nsmc);
    return true;
  } else {
    return false;
  }
}

void NSMCommunicator::setContext(NSMcontext* nsmc) throw(NSMHandlerException)
{
  if (_nsmc != NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "NSM is already available"));
  }
  if (nsmc != NULL) {
    _nsmc = nsmc;
    _id = _nsmc->nodeid;
  }
}

bool NSMCommunicator::performCallback() throw(NSMHandlerException)
{
  if (_callback) {
    return _callback->perform(_message);
  }
  return false;
}

int NSMCommunicator::getNodeIdByName(const std::string& name)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(_nsmc);
  return b2nsm_nodeid(name.c_str());
#else
  return -1;
#endif
}

int NSMCommunicator::getNodePidByName(const std::string& name)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  b2nsm_context(_nsmc);
  return b2nsm_nodepid(name.c_str());
#else
  return -1;
#endif
}

bool NSMCommunicator::isConnected(NSMNode* node) throw()
{
  bool is_online = getNodeIdByName(node->getName()) >= 0 &&
                   getNodePidByName(node->getName()) > 0;
  return is_online;
}
