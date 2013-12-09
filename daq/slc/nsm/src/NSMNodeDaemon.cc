#include "daq/slc/nsm/NSMNodeDaemon.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <unistd.h>

using namespace Belle2;

void NSMNodeDaemon::run() throw()
{
  NSMNode* node = _callback->getNode();
  NSMCommunicator* nsm_comm = new NSMCommunicator(node);
  nsm_comm->setCallback(_callback);
  while (true) {
    try {
      if (_port < 0)
        nsm_comm->init();
      else
        nsm_comm->init(_host, _port);
      break;
    } catch (const NSMHandlerException& e) {
      Belle2::debug("[DEBUG] Failed to connect NSM network (%s:%d). Re-trying to connect...",
                    _host.c_str(), _port);
      sleep(3);
    }
  }
  _callback->init();
  if (_rdata != NULL) {
    while (!_rdata->isAvailable()) {
      try {
        _rdata->open(nsm_comm);
      } catch (const NSMHandlerException& e) {
        Belle2::debug("NSM node daemon : Failed to allocate NSM node data. Waiting for 5 seconds..");
        sleep(5);
      }
    }
  }
  if (_wdata != NULL) {
    while (!_wdata->isAvailable()) {
      try {
        _wdata->allocate(nsm_comm);
      } catch (const NSMHandlerException& e) {
        sleep(5);
      }
    }
  }
  try {
    while (true) {
      if (nsm_comm->wait(2)) {
        _callback->setMessage(nsm_comm->getMessage());
        nsm_comm->performCallback();
      } else {
        _callback->selfCheck();
      }
    }
  } catch (const std::exception& e) {
    Belle2::debug("NSM node daemon : Caught exception (%s). Terminate process...",
                  e.what());
  }
}
