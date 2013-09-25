#include "NSMNodeDaemon.hh"

#include "NSMCommunicator.hh"

#include <util/Debugger.hh>

#include <unistd.h>

using namespace B2DAQ;

void NSMNodeDaemon::run() throw()
{
  NSMCommunicator* nsm_comm = new NSMCommunicator(_node);
  nsm_comm->setCallback(_callback);
  try {
    nsm_comm->init();
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("NSM node daemon : Failed to connect NSM network. Terminate process...");
    return;
  }
  if (_rdata != NULL) {
    while (!_rdata->isAvailable()) {
      try {
        _rdata->open();
      } catch (const NSMHandlerException& e) {
        B2DAQ::debug("NSM node daemon : Failed to allocate NSM node data. Waiting for 5 seconds..");
        sleep(5);
      }
    }
  }
  if (_wdata != NULL) {
    while (!_wdata->isAvailable()) {
      try {
        _wdata->allocate();
      } catch (const NSMHandlerException& e) {
        B2DAQ::debug("NSM node daemon : Failed to allocate NSM node data. Waiting for 5 seconds..");
        sleep(5);
      }
    }
  }
  try {
    while (true) {
      if (nsm_comm->wait(2000)) {
        _node->setConnection(Connection::ONLINE);
        nsm_comm->performCallback();
      } else {
        _callback->selfCheck();
      }
    }
  } catch (const std::exception& e) {
    B2DAQ::debug("NSM node daemon : Caught exception (%s). Terminate process...",
                 e.what());
  }
}
