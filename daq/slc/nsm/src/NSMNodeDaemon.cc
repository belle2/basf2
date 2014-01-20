#include "daq/slc/nsm/NSMNodeDaemon.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include "daq/slc/base/Date.h"

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

NSMNodeDaemon::NSMNodeDaemon(NSMCallback* callback,
                             const std::string host, int port,
                             NSMData* rdata, NSMData* wdata)
  : _callback(callback), _rdata(rdata), _wdata(wdata),
    _host(host), _port(port), _nsm_comm(NULL)
{
  init();
}

void NSMNodeDaemon::init() throw(NSMHandlerException)
{
  try {
    NSMNode* node = _callback->getNode();
    _nsm_comm = new NSMCommunicator(node);
    _nsm_comm->setCallback(_callback);
    if (_port < 0) {
      _nsm_comm->init();
    } else {
      _nsm_comm->init(_host, _port);
    }
    if (_rdata != NULL) _rdata->open(_nsm_comm);
    if (_wdata != NULL) _wdata->allocate(_nsm_comm);
    _callback->init();
  } catch (const NSMHandlerException& e) {
    LogFile::fatal("Failed to connect NSM network (%s:%d). Terminating process ",
                   _host.c_str(), _port);
    delete _nsm_comm;
    _nsm_comm = NULL;
    exit(1);
  }
  LogFile::debug("Connected to NSM2 daemon (%s:%d)", _host.c_str(), _port);
}

void NSMNodeDaemon::run() throw()
{
  try {
    while (true) {
      if (_nsm_comm->wait(2)) {
        _callback->setMessage(_nsm_comm->getMessage());
        _nsm_comm->performCallback();
      } else {
        _callback->selfCheck();
      }
    }
  } catch (const std::exception& e) {
    LogFile::fatal("NSM node daemon : Caught exception (%s:%d %s). Terminate process...",
                   _host.c_str(), _port, e.what());
    exit(1);
  }
}
