#include <boost/python.hpp>

#ifdef PYHSLB
#include <daq/slc/copper/HSLB.h>
#else
#include <framework/utilities/RegisterPythonModule.h>
#endif

#include <daq/slc/python/DAQDBObject.h>

BOOST_PYTHON_MODULE(b2daq)
{

  using namespace boost::python;
  using namespace Belle2;

#ifdef PYHSLB
  boost::python::class_<HSLB>("HSLB")
  .def("open", &HSLB::open)
  .def("close", &HSLB::close)
  .def("monitor", &HSLB::monitor)
  .def("boot", &HSLB::boot)
  .def("isError", &HSLB::isError)
  .def("isBelle2LinkeDown", &HSLB::isBelle2LinkDown)
  .def("isCOPPERFifoFull", &HSLB::isCOPPERFifoFull)
  .def("isCOPPERLengthFifoFull", &HSLB::isCOPPERLengthFifoFull)
  .def("isFifoFull", &HSLB::isFifoFull)
  .def("isCRCError", &HSLB::isCRCError)
  .def("fd", &HSLB::fd)
  .def("get_finid", &HSLB::get_finid)
  .def("readfn", &HSLB::readfn)
  .def("writefn", &HSLB::writefn)
  .def("readfn32", &HSLB::readfn32)
  .def("writefn32", &HSLB::writefn32)
  .def("hswait_quiet", &HSLB::hswait_quiet)
  .def("hswait", &HSLB::hswait)
  .def("readfee8", &HSLB::readfee8)
  .def("writefee8", &HSLB::writefee8)
  .def("readfee32", &HSLB::readfee32)
  .def("writefee32", &HSLB::writefee32)
  .def("writestream", &HSLB::writestream)
  .def("bootfpga", &HSLB::bootfpga)
  .def("reset_b2l", &HSLB::reset_b2l)
  .def("test", &HSLB::test)
  ;
#endif

  boost::python::class_<DAQDBObject>("DAQDBObject", boost::python::init<const char*, const char*>())
  .def("getPath", &DAQDBObject::getPath, return_value_policy<copy_const_reference>())
  .def("getName", &DAQDBObject::getName, return_value_policy<copy_const_reference>())
  .def("getFieldNames", &DAQDBObject::getFieldNames)
  .def("getNameList", &DAQDBObject::getNameList)
  .def("print", &DAQDBObject::print)
  .def("getId", &DAQDBObject::getId)
  .def("hasField", &DAQDBObject::hasField)
  .def("hasNumber", &DAQDBObject::hasValue)
  .def("hasText", &DAQDBObject::hasText)
  .def("getBool", &DAQDBObject::getBool)
  .def("getChar", &DAQDBObject::getChar)
  .def("getShort", &DAQDBObject::getShort)
  .def("getInt", &DAQDBObject::getInt)
  .def("getFloat", &DAQDBObject::getFloat)
  .def("getDouble", &DAQDBObject::getDouble)
  .def("getText", &DAQDBObject::getText, return_value_policy<copy_const_reference>())
  .def("getObject", &DAQDBObject::getObject)
  .def("setBool", &DAQDBObject::setBool)
  .def("setChar", &DAQDBObject::setChar)
  .def("setShort", &DAQDBObject::setShort)
  .def("setInt", &DAQDBObject::setInt)
  .def("setFloat", &DAQDBObject::setFloat)
  .def("setDouble", &DAQDBObject::setDouble)
  .def("setText", &DAQDBObject::setText, return_value_policy<copy_const_reference>())
  .def("addBool", &DAQDBObject::addBool)
  .def("addChar", &DAQDBObject::addChar)
  .def("addShort", &DAQDBObject::addShort)
  .def("addInt", &DAQDBObject::addInt)
  .def("addFloat", &DAQDBObject::addFloat)
  .def("addDouble", &DAQDBObject::addDouble)
  .def("addText", &DAQDBObject::addText, return_value_policy<copy_const_reference>())
  .def("addObject", &DAQDBObject::addObject)
  ;

}

#ifndef PYHSLB
REGISTER_PYTHON_MODULE(b2daq)
#endif
