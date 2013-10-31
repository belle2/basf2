#include "NSMCommunicator.h"

#include "NSMCallback.h"

#include "base/StringUtil.h"
#include "base/Debugger.h"

extern "C" {
#include <nsmlib2.h>
#include <belle2nsm.h>
}

#include <iostream>
#include <cstdio>
#include <cstring>

using namespace Belle2;

NSMCommunicator::NSMCommunicator(NSMNode* node) throw()
  : _id(-1), _node(node), _callback(NULL) {}

void NSMCommunicator::init(bool usesig) throw(NSMHandlerException)
{
  if (_node == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "No node for NSM was registered!"));
  }
  _nsmc = NULL;
  if ((_nsmc = b2nsm_init(_node->getName().c_str())) <= 0) {
    _id = -1;
    throw (NSMHandlerException(__FILE__, __LINE__, "Error during init2"));
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
}

void NSMCommunicator::sendRequest(NSMNode* node, const Command& command,
                                  int npar, unsigned int* pars,
                                  int len, const char* datap) throw(NSMHandlerException)
{
  if (b2nsm_sendreq_data(node->getName().c_str(), command.getLabel(),
                         npar, (int*)pars, len, datap) < 0) {
    _id = -1;
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to send request"));
  }
}

void NSMCommunicator::replyOK(NSMNode* node, const std::string& message)
throw(NSMHandlerException)
{
  if (b2nsm_ok(_message.getMsg(), _node->getState().getLabel(), message.c_str()) < 0) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to reply OK"));
  }
}

void NSMCommunicator::replyError(const std::string& message)
throw(NSMHandlerException)
{
  if (b2nsm_error(_message.getMsg(), message.c_str()) < 0) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to reply error"));
  }
}

bool NSMCommunicator::wait(int sec) throw(NSMHandlerException)
{
  if (_nsmc == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Not ready for wait"));
  }
  b2nsm_context(_nsmc);
  NSMcontext* nsmc = nsmlib_selectc(0, sec);
  if (nsmc == (NSMcontext*) - 1) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Error during wait"));
  } else if (nsmc == _nsmc) {
    _message.read(nsmc);
    return true;
  }
  return false;
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
  return b2nsm_nodeid(name.c_str());
}

int NSMCommunicator::getNodePidByName(const std::string& name)
throw(NSMHandlerException)
{
  return b2nsm_nodepid(name.c_str());
}
