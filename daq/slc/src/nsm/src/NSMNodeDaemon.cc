#include "NSMNodeDaemon.h"

#include "NSMCommunicator.h"

#include "base/Debugger.h"

#include <unistd.h>

using namespace Belle2;

void NSMNodeDaemon::run() throw()
{
  NSMCommunicator* nsm_comm = new NSMCommunicator(_node);
  nsm_comm->setCallback(_callback);
  while (true) {
    try {
      nsm_comm->init();
      break;
    } catch (const NSMHandlerException& e) {
      Belle2::debug("[DEBUG] Failed to connect NSM network. Re-trying to connect...");
      sleep(3);
    }
  }
  if (_rdata != NULL) {
    while (!_rdata->isAvailable()) {
      try {
        _rdata->open();
      } catch (const NSMHandlerException& e) {
        Belle2::debug("NSM node daemon : Failed to allocate NSM node data. Waiting for 5 seconds..");
        sleep(5);
      }
    }
  }
  if (_wdata != NULL) {
    while (!_wdata->isAvailable()) {
      try {
        _wdata->allocate();
      } catch (const NSMHandlerException& e) {
        Belle2::debug("NSM node daemon : Failed to allocate NSM node data. Waiting for 5 seconds..");
        sleep(5);
      }
    }
  }
  try {
    while (true) {
      if (nsm_comm->wait(2000)) {
        NSMMessage msg = nsm_comm->getMessage();
        Command command = msg.getRequestName();
        int npar = msg.getNParams();
        const unsigned int* pars = msg.getParams();
        _node->setParams(command, npar, pars, msg.getData());
        _node->setConnection(Connection::ONLINE);
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
