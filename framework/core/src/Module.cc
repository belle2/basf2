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
#include <framework/core/Path.h>


using namespace std;
using namespace Belle2;
using namespace boost::python;


Module::Module() :
  m_name(),
  m_type(),
  m_description("Not set by the author"),
  m_propertyFlags(0),
  m_logConfig(),
  m_hasReturnValue(false),
  m_returnValue(0),
  m_hasCondition(false),
  m_conditionValue(0),
  m_afterConditionPath(EAfterConditionPath::c_End)
{
}


Module::~Module()
{

}

const std::string& Module::getType() const
{
  if (m_type.empty())
    B2FATAL("Module type not set for " << getName());
  return m_type;
}

void Module::setType(const std::string& type)
{
  if (!m_type.empty())
    B2FATAL("Trying to change module type from " << m_type << " is not allowed, the value is assumed to be fixed.");
  m_type = type;
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

void Module::if_true(boost::shared_ptr<Path> path, EAfterConditionPath afterConditionPath)
{
  if_value(">=1", path, afterConditionPath);
}


bool Module::evalCondition()
{
  if (!m_hasCondition) return false;

  //okay, a condition was set for this Module...
  if (!m_hasReturnValue) {
    B2FATAL("A condition was set for '" << getName() << "', but the module did not set a return value!");
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


boost::shared_ptr<PathElement> Module::clone() const
{
  ModulePtr newModule = ModuleManager::Instance().registerModule(getType());
  newModule->m_moduleParamList.setParameters(getParamList());
  newModule->setName(getName());
  newModule->m_package = m_package;
  newModule->m_propertyFlags = m_propertyFlags;
  newModule->m_logConfig = m_logConfig;

  newModule->m_hasCondition = m_hasCondition;
  if (m_conditionPath) {
    boost::shared_ptr<Path> p = boost::static_pointer_cast<Path>(m_conditionPath->clone());
    newModule->m_conditionPath = p;
  }
  newModule->m_conditionOperator = m_conditionOperator;
  newModule->m_conditionValue = m_conditionValue;
  newModule->m_afterConditionPath = m_afterConditionPath;

  return newModule;
}

std::string Module::getPathString() const
{

  std::string output = getName();

  if (m_hasCondition) {
    output += "(? ";
    switch (m_conditionOperator) {
      case Belle2::CondParser::c_GT: output += ">"; break;
      case Belle2::CondParser::c_ST: output += "<"; break;
      case Belle2::CondParser::c_GE: output += ">="; break;
      case Belle2::CondParser::c_SE: output += "<="; break;
      case Belle2::CondParser::c_NE: output += "!="; break;
      case Belle2::CondParser::c_EQ: output += "=="; break;
      default: output += "???";
    }
    output += std::to_string(m_conditionValue);
    output += m_conditionPath->getPathString();
    output += " )";
  }

  return output;
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

boost::shared_ptr<boost::python::list> Module::getParamInfoListPython() const
{
  return m_moduleParamList.getParamInfoListPython();
}

namespace {
  /** Same as above member function, but return-by-value. */
  boost::python::list _getParamInfoListPython(const Module* m)
  {
    return *(m->getParamInfoListPython().get()); //copy the list object
  }
}

#if !defined(__GNUG__) || defined(__clang__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(if_value_overloads, if_value, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(if_false_overloads, if_false, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(if_true_overloads, if_true, 1, 2)
#if !defined(__GNUG__) || defined(__clang__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif


void Module::exposePythonAPI()
{
  void (Module::*setReturnValueInt)(int) = &Module::setReturnValue;
  void (Module::*setReturnValueBool)(bool) = &Module::setReturnValue;

  enum_<Module::EAfterConditionPath>("AfterConditionPath")
  .value("END", Module::EAfterConditionPath::c_End)
  .value("CONTINUE", Module::EAfterConditionPath::c_Continue)
  ;

  /* Do not change the names of >, <, ... we use them to serialize conditional pathes */
  enum_<Belle2::CondParser::EConditionOperators>("ConditionOperator")
  .value(">", Belle2::CondParser::EConditionOperators::c_GT)
  .value("<", Belle2::CondParser::EConditionOperators::c_ST)
  .value(">=", Belle2::CondParser::EConditionOperators::c_GE)
  .value("<=", Belle2::CondParser::EConditionOperators::c_SE)
  .value("==", Belle2::CondParser::EConditionOperators::c_EQ)
  .value("!=", Belle2::CondParser::EConditionOperators::c_NE)
  ;

  enum_<Module::EModulePropFlags>("ModulePropFlags")
  .value("INPUT", Module::EModulePropFlags::c_Input)
  .value("OUTPUT", Module::EModulePropFlags::c_Output)
  .value("PARALLELPROCESSINGCERTIFIED", Module::EModulePropFlags::c_ParallelProcessingCertified)
  .value("HISTOGRAMMANAGER", Module::EModulePropFlags::c_HistogramManager)
  .value("INTERNALSERIALIZER", Module::EModulePropFlags::c_InternalSerializer)
  .value("TERMINATEINALLPROCESSES", Module::EModulePropFlags::c_TerminateInAllProcesses)
  ;

  //Python class definition
  class_<Module, PyModule>("Module")
  .def("__str__", &Module::getPathString)
  .def("name", &Module::getName, return_value_policy<copy_const_reference>())
  .def("type", &Module::getType, return_value_policy<copy_const_reference>())
  .def("set_name", &Module::setName)
  .def("description", &Module::getDescription, return_value_policy<copy_const_reference>())
  .def("package", &Module::getPackage, return_value_policy<copy_const_reference>())
  .def("available_params", &_getParamInfoListPython)
  .def("has_properties", &Module::hasProperties)
  .def("set_property_flags", &Module::setPropertyFlags)
  .def("if_value", &Module::if_value, if_value_overloads())
  .def("if_false", &Module::if_false, if_false_overloads())
  .def("if_true", &Module::if_true, if_true_overloads())
  .def("has_condition", &Module::hasCondition)
  .def("get_condition_path", &Module::getConditionPath)
  .def("get_condition_option", &Module::getAfterConditionPath)
  .def("get_condition_value", &Module::getConditionValue)
  .def("get_condition_operator", &Module::getConditionOperator)
  .add_property("logging",
                make_function(&Module::getLogConfig, return_value_policy<reference_existing_object>()),
                &Module::setLogConfig)
  .def("param", &Module::setParamPython)
  .def("param", &Module::setParamPythonDict)
  .def("return_value", setReturnValueInt)
  .def("return_value", setReturnValueBool)
  .def("set_log_level", &Module::setLogLevel)
  .def("set_debug_level", &Module::setDebugLevel)
  .def("set_abort_level", &Module::setAbortLevel)
  .def("set_log_info", &Module::setLogInfo)
  //tell python about the default implementations of virtual functions
  .def("initialize", &Module::def_initialize)
  .def("beginRun", &Module::def_beginRun)
  .def("event", &Module::def_event)
  .def("endRun", &Module::def_endRun)
  .def("terminate", &Module::def_terminate)
  ;

  register_ptr_to_python<ModulePtr>();
}


//=====================================================================
//                          ModuleProxyBase
//=====================================================================

ModuleProxyBase::ModuleProxyBase(const std::string& moduleType, const std::string& package) : m_moduleType(moduleType),
  m_package(package)
{
  ModuleManager::Instance().registerModuleProxy(this);
}
