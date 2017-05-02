#include <daq/slc/pyb2daq/PyNSMCallback.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/system/Daemon.h>
#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

void PyNSMCallback::run(const char* confname)
{
  ConfigFile config("slowcontrol", confname);
  std::string host = config.get("nsm.host");
  int port = config.getInt("nsm.port");
  std::string name = config.get("nsm.nodename");
  int timeout = config.getInt("timeout");
  if (timeout <= 0) timeout = 5;
  m_callback = new PyNSMCallbackImp(this);
  m_callback->setTimeout(timeout);
  m_callback->setNode(NSMNode(name));
  NSMNodeDaemon(m_callback, host, port).run();
  delete m_callback;
}

void PyNSMCallbackWrapper::init()
{
  try {
    if (boost::python::override func = get_override("init")) {
      func();
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

void PyNSMCallbackWrapper::term()
{
  try {
    if (boost::python::override func = get_override("term")) {
      func();
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

void PyNSMCallbackWrapper::timeout()
{
  try {
    if (boost::python::override func = get_override("timeout")) {
      func();
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

