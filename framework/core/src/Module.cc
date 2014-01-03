/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python/register_ptr_to_python.hpp>
#include <boost/python/class.hpp>
#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/overloads.hpp>
#include <boost/python/enum.hpp>

#include <framework/core/Module.h>
#include <framework/core/PyModule.h>
#include <framework/core/ModuleManager.h>


using namespace std;
using namespace Belle2;
using namespace boost::python;


Module::Module() :
  m_name(""),
  m_description("Not set by the author"),
  m_propertyFlags(0),
  m_logConfig(),
  m_hasReturnValue(false),
  m_returnValue(0),
  m_hasCondition(false),
  m_afterConditionPath(EAfterConditionPath::c_End)
{
}


Module::~Module()
{

}


void Module::setLogLevel(int logLevel)
{
  m_logConfig.setLogLevel(static_cast<LogConfig::ELogLevel>(logLevel));
}


void Module::setDebugLevel(int debugLevel)
{
  m_logConfig.setDebugLevel(debugLevel);
}


void Module::setAbortLevel(int abortLevel)
{
  m_logConfig.setAbortLevel(static_cast<LogConfig::ELogLevel>(abortLevel));
}


void Module::setLogInfo(int logLevel, unsigned int logInfo)
{
  m_logConfig.setLogInfo(static_cast<LogConfig::ELogLevel>(logLevel), logInfo);
}


void Module::if_value(const std::string& expression, boost::shared_ptr<Path> path, EAfterConditionPath afterConditionPath)
{
  if (m_hasCondition) {
    B2ERROR("You can only set a single condition per module!");
  }

  CondParser condParser;
  if (condParser.parseCondition(expression, m_conditionOperator, m_conditionValue)) {
    m_hasCondition = true;
    m_conditionPath = path;
    m_afterConditionPath = afterConditionPath;
  } else {
    B2ERROR("Invalid condition: could not parse condition: '" + expression + "'! The condition is NOT active!");
  }
}


void Module::if_false(boost::shared_ptr<Path> path, EAfterConditionPath afterConditionPath)
{
  if_value("<1", path, afterConditionPath);
}


bool Module::evalCondition()
{
  if (!m_hasCondition) return false;

  //okay, a condition was set for this Module...
  if (!m_hasReturnValue) {
    B2FATAL("A condition was set for '" << getName() << "', but the module did not set a return value!");
    return false;
  }
  return CondParser::evalCondition(m_returnValue, m_conditionValue, m_conditionOperator);
}


bool Module::hasProperties(unsigned int propertyFlags) const
{
  return (propertyFlags & m_propertyFlags) == propertyFlags;
}


bool Module::hasUnsetForcedParams() const
{
  return m_moduleParamList.hasUnsetForcedParams();
}

//============================================================================
//                          Protected methods
//============================================================================

void Module::setPropertyFlags(unsigned int propertyFlags)
{
  m_propertyFlags = propertyFlags;
}


void Module::setDescription(const std::string& description)
{
  m_description = description;
}


void Module::setReturnValue(int value)
{
  m_hasReturnValue = true;
  m_returnValue = value;
}


void Module::setReturnValue(bool value)
{
  m_hasReturnValue = true;
  m_returnValue = value;
}


void Module::setParamPython(const std::string& name, const boost::python::object& pyObj)
{
  LogSystem& logSystem = LogSystem::Instance();
  logSystem.setModuleLogConfig(&(getLogConfig()), getName());

  m_moduleParamList.setParamPython(name, pyObj);

  logSystem.setModuleLogConfig(NULL);
}


void Module::setParamPythonDict(const boost::python::dict& dictionary)
{

  LogSystem& logSystem = LogSystem::Instance();
  logSystem.setModuleLogConfig(&(getLogConfig()), getName());

  boost::python::list dictKeys = dictionary.keys();
  int nKey = boost::python::len(dictKeys);

  //Loop over all keys in the dictionary
  for (int iKey = 0; iKey < nKey; ++iKey) {
    boost::python::object currKey = dictKeys[iKey];
    boost::python::extract<std::string> keyProxy(currKey);

    if (keyProxy.check()) {
      const boost::python::object& currValue = dictionary[currKey];
      setParamPython(keyProxy, currValue);
    } else {
      B2ERROR("Setting the module parameters from a python dictionary: invalid key in dictionary!");
    }
  }

  logSystem.setModuleLogConfig(NULL);
}


//=====================================================================
//                          Python API
//=====================================================================

boost::python::list* Module::getParamInfoListPython() const
{
  return m_moduleParamList.getParamInfoListPython();
}

/** Same as above member function, but return-by-value. */
boost::python::list _getParamInfoListPython(const Module* m)
{
  boost::shared_ptr<boost::python::list> p(m->getParamInfoListPython());
  return *p.get(); //copy
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(if_value_overloads, if_value, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(if_false_overloads, if_false, 1, 2)


void Module::exposePythonAPI()
{
  void (Module::*setReturnValueInt)(int) = &Module::setReturnValue;
  void (Module::*setReturnValueBool)(bool) = &Module::setReturnValue;

  enum_<Module::EAfterConditionPath>("AfterConditionPath")
  .value("END", Module::EAfterConditionPath::c_End)
  .value("CONTINUE", Module::EAfterConditionPath::c_Continue)
  ;

  //Python class definition
  class_<Module, PyModule>("Module")
  .def("__str__", &Module::getPathString)
  .def("name", &Module::getName, return_value_policy<copy_const_reference>())
  .def("set_name", &Module::setModuleName)
  .def("description", &Module::getDescription, return_value_policy<copy_const_reference>())
  .def("package", &Module::getPackage, return_value_policy<copy_const_reference>())
  .def("if_value", &Module::if_value, if_value_overloads())
  .def("if_false", &Module::if_false, if_false_overloads())
  .def("param", &Module::setParamPython)
  .def("param", &Module::setParamPythonDict)
  .def("available_params", &_getParamInfoListPython)
  .add_property("logging",
                make_function(&Module::getLogConfig, return_value_policy<reference_existing_object>()),
                &Module::setLogConfig)
  .def("return_value", setReturnValueInt)
  .def("return_value", setReturnValueBool)
  .def("set_log_level", &Module::setLogLevel)
  .def("set_debug_level", &Module::setDebugLevel)
  .def("set_abort_level", &Module::setAbortLevel)
  .def("set_log_info", &Module::setLogInfo)
  //tell python about the default implementations of virtual functions
  .def("initialize", &PyModule::def_initialize)
  .def("beginRun", &PyModule::def_beginRun)
  .def("event", &PyModule::def_event)
  .def("endRun", &PyModule::def_endRun)
  .def("terminate", &PyModule::def_terminate)
  ;

  register_ptr_to_python<ModulePtr>();
}


//=====================================================================
//                          ModuleProxyBase
//=====================================================================

ModuleProxyBase::ModuleProxyBase(const std::string& moduleName, const std::string& package) : m_moduleName(moduleName), m_package(package)
{
  ModuleManager::Instance().registerModuleProxy(this);
}
