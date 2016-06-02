#define BOOST_PYTHON_STATIC_LIB
#include <boost/python.hpp>
#include <boost/ref.hpp>

#include "daq/slc/pyb2daq/apps/py2nsmd/Py2nsmCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/pyb2daq/PyNSMCallback.h>

#include <stdlib.h>

namespace bpy = boost::python;
using namespace Belle2;

Py2nsmCallback::Py2nsmCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
}

Py2nsmCallback::~Py2nsmCallback() throw()
{
  Py_Finalize();
}

bpy::object global_ns;
bpy::object init_func;
bpy::object timeout_func;

void Py2nsmCallback::init(NSMCommunicator&) throw()
{
  Py_Initialize();
  try {
    global_ns = bpy::import("__main__").attr("__dict__");
    bpy::exec("from pyb2daq import *\n", global_ns, global_ns);
    bpy::exec_file("pysample.py", global_ns, global_ns);
    PyNSMCallback::setCallback(this);
    init_func = global_ns["init"];
    timeout_func = global_ns["timeout"];
    init_func();
  } catch (const bpy::error_already_set&) {
    PyErr_Print();
  }
}

void Py2nsmCallback::timeout(NSMCommunicator&) throw()
{
  timeout_func();
}

