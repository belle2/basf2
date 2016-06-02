#define BOOST_PYTHON_STATIC_LIB
#include <boost/python.hpp>
#include <boost/ref.hpp>

#include <daq/slc/pyb2daq/DAQDBObject.h>

#include <iostream>

namespace bpy = boost::python;
using namespace Belle2;

int main(int argc, char** argv)
{
  Py_Initialize();
  try {
    bpy::object global_ns = bpy::import("__main__").attr("__dict__");
    DAQDBObject obj("daq", "STORE_CDC@RC:local:cdc:cosmic:002");
    bpy::exec("from pyb2daq import *\n", global_ns, global_ns);
    //global_ns["obj"] = boost::ref(obj);
    bpy::exec_file(argv[1], global_ns, global_ns);
    //DAQDBObject obj2 = bpy::extract<DAQDBObject>( global_ns["obj"] );
    boost::python::object pythonFunc = global_ns["g"];
    pythonFunc(boost::ref(obj));
  } catch (const bpy::error_already_set&) {
    PyErr_Print();
  }
  Py_Finalize();
  return 0;
}
