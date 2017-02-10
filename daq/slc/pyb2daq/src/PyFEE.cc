#include <daq/slc/pyb2daq/PyFEE.h>
#include <daq/slc/pyb2daq/DAQDBObject.h>
#include <daq/slc/pyb2daq/PyNSMCallback.h>

using namespace Belle2;

bool PyFEE::g_pyinitialized = false;
boost::python::object PyFEE::g_ns;
boost::python::object PyFEE::g_init;
boost::python::object PyFEE::g_boot;
boost::python::object PyFEE::g_load;
boost::python::object PyFEE::g_monitor;

PyFEE::PyFEE(const std::string& script)
{
  try {
    if (!g_pyinitialized) {
      g_pyinitialized = true;
      Py_Initialize();
      g_ns = boost::python::import("__main__").attr("__dict__");
      boost::python::exec("from pyb2daq import *\n", g_ns, g_ns);
      boost::python::exec_file(script.c_str(), g_ns, g_ns);
      g_init = g_ns["init"];
      g_boot = g_ns["boot"];
      g_load = g_ns["load"];
      g_monitor = g_ns["monitor"];
    }
    m_ns = g_ns;
    m_init = g_init;
    m_boot = g_boot;
    m_load = g_load;
    m_monitor = g_monitor;
  } catch (...) {
    PyErr_Print();
    throw (RCHandlerException());
  }
}

PyFEE::~PyFEE() throw()
{
  if (g_pyinitialized) {
    g_pyinitialized = false;
    Py_Finalize();
  }
}

void PyFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  try {
    PyNSMCallback::setCallback(&callback);
    m_init(hslb, DAQDBObject(obj));
    PyNSMCallback::setCallback(NULL);
  } catch (const IOException& e) {
    throw (RCHandlerException(e.what()));
  } catch (...) {
    PyObject* ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    std::string msg = boost::python::extract<std::string>(pvalue);
    throw (RCHandlerException(msg));
  }
}

void PyFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  try {
    PyNSMCallback::setCallback(&callback);
    m_boot(hslb, DAQDBObject(obj));
    PyNSMCallback::setCallback(NULL);
  } catch (const IOException& e) {
    throw (RCHandlerException(e.what()));
  } catch (...) {
    PyObject* ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    std::string msg = boost::python::extract<std::string>(pvalue);
    throw (RCHandlerException(msg));
  }
}

void PyFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  try {
    PyNSMCallback::setCallback(&callback);
    m_load(hslb, DAQDBObject(obj));
    PyNSMCallback::setCallback(NULL);
  } catch (const IOException& e) {
    throw (RCHandlerException(e.what()));
  } catch (...) {
    PyObject* ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    std::string msg = boost::python::extract<std::string>(pvalue);
    throw (RCHandlerException(msg));
  }
}

void PyFEE::monitor(RCCallback& callback, HSLB& hslb)
{
  try {
    PyNSMCallback::setCallback(&callback);
    m_monitor(hslb);
    PyNSMCallback::setCallback(NULL);
  } catch (const IOException& e) {
    throw (RCHandlerException(e.what()));
  } catch (...) {
    PyObject* ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    std::string msg = boost::python::extract<std::string>(pvalue);
    throw (RCHandlerException(msg));
  }
}

