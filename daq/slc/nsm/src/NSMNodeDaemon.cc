#include "daq/slc/nsm/NSMNodeDaemon.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

NSMNodeDaemon::NSMNodeDaemon(NSMCallback* callback,
                             const std::string host, int port)
  : _host(host), _port(port), _nsm_comm(NULL)
{
  _callback[0] = callback;
  _callback[1] = NULL;
  init();
}

NSMNodeDaemon::NSMNodeDaemon(NSMCallback* callback1,
                             NSMCallback* callback2,
                             const std::string host, int port)
  : _host(host), _port(port), _nsm_comm(NULL)
{
  _callback[0] = callback1;
  _callback[1] = callback2;
  init();
}

void NSMNodeDaemon::init() throw(NSMHandlerException)
{
  try {
    NSMNode& node(_callback[0]->getNode());
    _nsm_comm = new NSMCommunicator();
    if (_port < 0) {
      _nsm_comm->init(node);
    } else {
      _nsm_comm->init(node, _host, _port);
    }
    _nsm_comm->setCallback(_callback[0]);
    _nsm_comm->setCallback(_callback[1]);
    _callback[0]->init();
    if (_callback[1] != NULL) _callback[1]->init();
  } catch (const NSMHandlerException& e) {
    LogFile::fatal("Failed to connect NSM network (%s:%d). "
                   "Terminating process ",
                   _host.c_str(), _port);
    delete _nsm_comm;
    _nsm_comm = NULL;
    exit(1);
  }
  LogFile::debug("Connected to NSM2 daemon (%s:%d)",
                 _host.c_str(), _port);
}

void NSMNodeDaemon::run() throw()
{
  try {
    while (true) {
      if (_nsm_comm->wait(_callback[0]->getTimeout())) {
        NSMMessage& msg(_nsm_comm->getMessage());
        if (!_callback[0]->perform(msg)) {
          if (_callback[1] != NULL) {
            _callback[1]->perform(msg);
          }
        }
      } else {
        _callback[0]->timeout();
        if (_callback[1] != NULL) {
          _callback[1]->timeout();
        }
      }
    }
  } catch (const std::exception& e) {
    LogFile::fatal("NSM node daemon : Caught exception (%s:%d %s). "
                   "Terminate process...",
                   _host.c_str(), _port, e.what());
    exit(1);
  }
}
