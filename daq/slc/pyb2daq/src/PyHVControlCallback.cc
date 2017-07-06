#include <daq/slc/pyb2daq/PyHVControlCallback.h>
#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/system/Daemon.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void PyHVControlCallback::run(const char* confname)
{
  ConfigFile config("slowcontrol", "hvcontrol/" + std::string(confname));
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  const std::string nodename = config.get("nsm.nodename");
  LogFile::open("hvcontrold/" + nodename, LogFile::DEBUG);
  m_callback = new PyHVControlCallbackImp(NSMNode(nodename), this);
  m_callback->setDB(config.get("hv.tablename"), &db);
  m_callback->setTimeout(config.getInt("hv.interval"));
  LogFile::info("config.standby : " + config.get("hv.config.standby"));
  m_callback->setStandbyConfig(config.get("hv.config.standby"));
  m_callback->setPeakConfig(config.get("hv.config.peak"));
  NSMNodeDaemon(m_callback, config.get("nsm.host"),
                config.getInt("nsm.port")).run();
  delete m_callback;
}

void PyHVControlCallbackWrapper::initialize(const DAQDBObject& db)
{
  try {
    if (boost::python::override func = get_override("initialize")) {
      func(db);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

void PyHVControlCallbackWrapper::_set(const char* name, int crate, int slot, int channel, bool value)
{
  try {
    if (boost::python::override func = get_override(name)) {
      func(crate, slot, channel, value);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

void PyHVControlCallbackWrapper::_set(const char* name, int crate, int slot, int channel, int value)
{
  try {
    if (boost::python::override func = get_override(name)) {
      func(crate, slot, channel, value);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

void PyHVControlCallbackWrapper::_set(const char* name, int crate, int slot, int channel, float value)
{
  try {
    if (boost::python::override func = get_override(name)) {
      func(crate, slot, channel, value);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

bool PyHVControlCallbackWrapper::_get_bool(const char* name, int crate, int slot, int channel)
{
  try {
    if (boost::python::override func = get_override(name)) {
      return func(crate, slot, channel);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
  return false;
}

int PyHVControlCallbackWrapper::_get_int(const char* name, int crate, int slot, int channel)
{
  try {
    if (boost::python::override func = get_override(name)) {
      return func(crate, slot, channel);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
  return 0;
}

float PyHVControlCallbackWrapper::_get_float(const char* name, int crate, int slot, int channel)
{
  try {
    if (boost::python::override func = get_override(name)) {
      return func(crate, slot, channel);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
  return 0;
}
