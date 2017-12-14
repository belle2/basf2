#include <daq/slc/pyb2daq/PyRCCallback.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/runcontrol/RCNodeDaemon.h>
#include <daq/slc/psql/PostgreSQLInterface.h>

using namespace Belle2;

void PyRCCallback::run(const char* confname)
{
  ConfigFile config("slowcontrol", confname);
  m_callback = new PyRCCallbackImp(this);
  RCNodeDaemon(config, m_callback, NULL,
               new PostgreSQLInterface(config.get("database.host"),
                                       config.get("database.dbname"),
                                       config.get("database.user"),
                                       config.get("database.password"),
                                       config.getInt("database.port"))).run();
  delete m_callback;
}

void PyRCCallbackWrapper::initialize(const DAQDBObject& db)
{
  try {
    if (boost::python::override func = get_override("initialize")) {
      func(db);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
  LogFile::info("KONNO");
}

void PyRCCallbackWrapper::boot(const DAQDBObject& db)
{
  LogFile::info("BOOTING");
  try {
    if (boost::python::override func = get_override("boot")) {
      func(db);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
  LogFile::info("BOOT");
}

void PyRCCallbackWrapper::load(const DAQDBObject& db)
{
  try {
    if (boost::python::override func = get_override("load")) {
      func(db);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

void PyRCCallbackWrapper::start(int expno, int runno)
{
  try {
    if (boost::python::override func = get_override("start")) {
      func(expno, runno);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

void PyRCCallbackWrapper::stop()
{
  try {
    if (boost::python::override func = get_override("stop")) {
      func();
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

void PyRCCallbackWrapper::recover(const DAQDBObject& db)
{
  try {
    if (boost::python::override func = get_override("recover")) {
      func(db);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

bool PyRCCallbackWrapper::resume(int subno)
{
  try {
    if (boost::python::override func = get_override("resume")) {
      return func(subno);
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
  return true;
}

bool PyRCCallbackWrapper::pause()
{
  try {
    if (boost::python::override func = get_override("pause")) {
      return func();
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
  return true;
}

void PyRCCallbackWrapper::abort()
{
  try {
    if (boost::python::override func = get_override("abort")) {
      func();
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

void PyRCCallbackWrapper::monitor()
{
  try {
    if (boost::python::override func = get_override("monitor")) {
      func();
    }
  } catch (...) {
    PyErr_Print();
    throw (NSMHandlerException());
  }
}

