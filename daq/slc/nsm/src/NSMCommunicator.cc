#include "daq/slc/nsm/NSMCommunicator.h"

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

std::vector<NSMCommunicator*> NSMCommunicator::__com_v;

NSMCommunicator* NSMCommunicator::select(int timeout) throw(NSMHandlerException)
{
  NSMcontext* nsmc = nsmlib_selectc(0, timeout);
  for (size_t i = 0; i < __com_v.size(); i++) {
    NSMCommunicator* com = __com_v[i];
    if (com == NULL) continue;
    if (nsmc == (NSMcontext*) - 1) {
      throw (NSMHandlerException(__FILE__, __LINE__, "Error during select"));
    } else if (nsmc == com->_nsmc) {
      com->_message.read(nsmc);
      b2nsm_context(nsmc);
      return com;
    }
  }
  return NULL;
}


NSMCommunicator::NSMCommunicator(NSMNode* node, const std::string& host,
                                 int port, const std::string& config_name) throw()
  : _node(node), _callback(NULL)
{
  _id = -1;
  _nsmc = NULL;
  _host = host;
  _port = port;
  _logger_node = NULL;
  _rc_node = NULL;
  _config_name = config_name;
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
    //_nsmc = nsmlib_init(_node->getName().c_str(), host.c_str(), port, port);
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
  ConfigFile config(_config_name, false);
  config.read("slowcontrol", false);
  std::string rc_name = config.get("RC_NSM_NAME");
  if (rc_name.size() > 0) {
    _rc_node = new NSMNode(rc_name);
  }
  std::string logger_name = config.get("LOG_NSM_NAME");
  if (logger_name.size() > 0) {
    _logger_node = new NSMNode(logger_name);
  }
  __com_v.push_back(this);
#else
#warning "Wrong version of nsm2. try source daq/slc/extra/nsm2/export.sh"
#endif
}

void NSMCommunicator::sendRequest(const NSMNode* node, const Command& command,
                                  int npar, unsigned int* pars,
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
                                  int npar, unsigned int* pars,
                                  const std::string& message)
throw(NSMHandlerException)
{
  sendRequest(node, cmd, npar, pars, message.size(),
              (message.size() == 0) ? NULL : message.c_str());
}

void NSMCommunicator::sendRequest(const NSMNode* node, const Command& cmd,
                                  NSMMessage& message)
throw(NSMHandlerException)
{
  sendRequest(node, cmd, message.getNParams(), message.getParams(),
              message.getData().size(),
              (message.getData().size() == 0) ? NULL : message.getData().c_str());
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

void NSMCommunicator::sendLog(const SystemLog& log) throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  if (_logger_node != NULL &&
      b2nsm_nodeid(_logger_node->getName().c_str()) >= 0) {
    std::string str;
    unsigned int pars[3];
    int npar = log.pack((int*)pars, str);
    sendRequest(_logger_node, Command::LOG, npar, pars, str);
  }
#endif
}

void NSMCommunicator::sendError(const std::string& message) throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  if (_rc_node != NULL) {
    sendRequest(_rc_node, Command::ERROR, 0, NULL, message);
  }
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
    b2nsm_context(_nsmc);
    return true;
  } else {
    return false;
  }
}

bool NSMCommunicator::performCallback() throw(NSMHandlerException)
{
  if (_callback) {
    Command command = _message.getRequestName();
    return _callback->perform(command, _message);
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
