#include <boost/python.hpp>

#ifdef PYHSLB
#include <daq/slc/copper/HSLB.h>
#include <boost/python.hpp>
#else
#include <framework/utilities/RegisterPythonModule.h>
#endif

#include <daq/slc/system/LogFile.h>

#include <daq/slc/pyb2daq/DAQDBObject.h>
#include <daq/slc/pyb2daq/PyNSMCallback.h>
#include <daq/slc/pyb2daq/PyHVControlCallback.h>
#include <daq/slc/pyb2daq/PyRCCallback.h>

using namespace boost::python;
using namespace Belle2;

BOOST_PYTHON_MODULE(pyb2daq)
{

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

  boost::python::enum_<LogFile::Priority>("LogPriority")
  .value("UNKNOWN", LogFile::UNKNOWN)
  .value("DEBUG", LogFile::DEBUG)
  .value("INFO", LogFile::INFO)
  .value("NOTICE", LogFile::NOTICE)
  .value("WARNING", LogFile::WARNING)
  .value("ERROR", LogFile::ERROR)
  .value("FATAL", LogFile::FATAL)
  ;

  boost::python::enum_<DBField::Type>("DAQDBType")
  .value("UNKNOWN", DBField::UNKNOWN)
  .value("BOOL", DBField::BOOL)
  .value("CHAR", DBField::CHAR)
  .value("SHORT", DBField::SHORT)
  .value("INT", DBField::INT)
  .value("LONG", DBField::LONG)
  .value("FLOAT", DBField::FLOAT)
  .value("DOUBLE", DBField::DOUBLE)
  .value("TEXT", DBField::TEXT)
  .value("OBJECT", DBField::OBJECT)
  ;

  boost::python::class_<DAQDBObject>("DAQDBObject", boost::python::init<const char*, const char*>())
  .def(boost::python::init<>())
  .def("getDBList", &DAQDBObject::getDBList)
  .def("create", &DAQDBObject::create)
  .def("setPath", &DAQDBObject::setPath)
  .def("setName", &DAQDBObject::setName)
  .def("getPath", &DAQDBObject::getPath, return_value_policy<copy_const_reference>())
  .def("getName", &DAQDBObject::getName, return_value_policy<copy_const_reference>())
  .def("getFieldNames", &DAQDBObject::getFieldNames)
  .def("getFieldType", &DAQDBObject::getType)
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
  .def("__call__", &DAQDBObject::getObject)//, return_value_policy<copy_non_const_reference>())
  .def("__call__", &DAQDBObject::getObjects)//, return_value_policy<copy_non_const_reference>())
  .def("getObject", &DAQDBObject::getObject)//, return_value_policy<copy_non_const_reference>())
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

  boost::python::class_<PyNSMCallbackWrapper, boost::noncopyable>("NSMCallback", boost::python::init<>())
  .def("addInt", &PyNSMCallback::addInt)
  .def("addFloat", &PyNSMCallback::addFloat)
  .def("addText", &PyNSMCallback::addText)
  .def("setInt", &PyNSMCallback::setInt)
  .def("setFloat", &PyNSMCallback::setFloat)
  .def("setText", &PyNSMCallback::setText)
  .def("log", &PyNSMCallback::log)
  .def("run", &PyNSMCallback::run)
  //.def("init", &PyNSMCallback::init)
  //.def("term", &PyNSMCallback::term)
  //.def("timeout", &PyNSMCallback::timeout)
  ;

  boost::python::enum_<HVMessage::State>("HVState")
  .value("OFF", HVMessage::OFF)
  .value("ON", HVMessage::ON)
  .value("OCP", HVMessage::OCP)
  .value("OVP", HVMessage::OVP)
  .value("ERR", HVMessage::ERR)
  .value("RAMPUP", HVMessage::RAMPUP)
  .value("RAMPDOWN", HVMessage::RAMPDOWN)
  .value("TRIP", HVMessage::TRIP)
  .value("ERTIP", HVMessage::ETRIP)
  .value("INTERLOCK", HVMessage::INTERLOCK)
  ;

  boost::python::class_<PyHVControlCallbackWrapper, boost::noncopyable>("HVControlCallback", boost::python::init<>())
  .def("addInt", &PyHVControlCallback::addInt)
  .def("addFloat", &PyHVControlCallback::addFloat)
  .def("addText", &PyHVControlCallback::addText)
  .def("setInt", &PyHVControlCallback::setInt)
  .def("setFloat", &PyHVControlCallback::setFloat)
  .def("setText", &PyHVControlCallback::setText)
  .def("log", &PyHVControlCallback::log)
  .def("run", &PyHVControlCallback::run)
  //.def("init", &PyHVControlCallback::init)
  //.def("term", &PyHVControlCallback::term)
  //.def("timeout", &PyHVControlCallback::timeout)
  ;

  boost::python::class_<PyRCCallbackWrapper, boost::noncopyable>("RCCallback", boost::python::init<>())
  .def("addInt", &PyRCCallback::addInt)
  .def("addFloat", &PyRCCallback::addFloat)
  .def("addText", &PyRCCallback::addText)
  .def("setInt", &PyRCCallback::setInt)
  .def("setFloat", &PyRCCallback::setFloat)
  .def("setText", &PyRCCallback::setText)
  .def("log", &PyRCCallback::log)
  .def("run", &PyRCCallback::run)
  //.def("init", &PyRCCallback::init)
  //.def("term", &PyRCCallback::term)
  //.def("timeout", &PyRCCallback::timeout)
  ;

}

#ifndef PYHSLB
REGISTER_PYTHON_MODULE(pyb2daq)
#else
#endif
