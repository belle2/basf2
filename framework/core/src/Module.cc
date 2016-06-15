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
#include <boost/python/docstring_options.hpp>

#include <framework/core/Module.h>
#include <framework/core/ModuleCondition.h>
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
  m_condition(nullptr)
{
}


Module::~Module()
{
  delete m_condition;
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
  if (m_condition) {
    throw std::runtime_error("You can only set a single condition per module!");
  }

  m_condition = new ModuleCondition(expression, path, afterConditionPath);
}


void Module::if_false(boost::shared_ptr<Path> path, EAfterConditionPath afterConditionPath)
{
  if_value("<1", path, afterConditionPath);
}

void Module::if_true(boost::shared_ptr<Path> path, EAfterConditionPath afterConditionPath)
{
  if_value(">=1", path, afterConditionPath);
}


bool Module::evalCondition() const
{
  if (!m_condition) return false;

  //okay, a condition was set for this Module...
  if (!m_hasReturnValue) {
    B2FATAL("A condition was set for '" << getName() << "', but the module did not set a return value!");
  }
  return m_condition->evaluate(m_returnValue);
}

boost::shared_ptr<Path> Module::getConditionPath() const
{
  PathPtr p;
  if (m_condition)
    p = m_condition->getPath();
  return p;
}

Module::EAfterConditionPath Module::getAfterConditionPath() const
{
  if (m_condition)
    return m_condition->getAfterConditionPath();
  return EAfterConditionPath::c_End;
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

  if (!m_condition) {
    newModule->m_condition = nullptr;
  } else {
    newModule->m_condition = new ModuleCondition(*m_condition);
  }

  return newModule;
}

std::string Module::getPathString() const
{

  std::string output = getName();

  if (m_condition) {
    output += m_condition->getString();
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
  int _getConditionValuePython(const Module* m)
  {
    if (m->getCondition())
      return m->getCondition()->getConditionValue();
    throw runtime_error("No condition was set for module " + m->getName());
  }
  ModuleCondition::EConditionOperators _getConditionOperatorPython(const Module* m)
  {
    if (m->getCondition())
      return m->getCondition()->getConditionOperator();
    throw runtime_error("No condition was set for module " + m->getName());
  }
}

#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(if_value_overloads, if_value, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(if_false_overloads, if_false, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(if_true_overloads, if_true, 1, 2)
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif


void Module::exposePythonAPI()
{
  docstring_options options(true, true, false); //userdef, py sigs, c++ sigs

  void (Module::*setReturnValueInt)(int) = &Module::setReturnValue;
  void (Module::*setReturnValueBool)(bool) = &Module::setReturnValue;

  enum_<Module::EAfterConditionPath>("AfterConditionPath")
  .value("END", Module::EAfterConditionPath::c_End)
  .value("CONTINUE", Module::EAfterConditionPath::c_Continue)
  ;

  /* Do not change the names of >, <, ... we use them to serialize conditional pathes */
  enum_<Belle2::ModuleCondition::EConditionOperators>("ConditionOperator")
  .value(">", Belle2::ModuleCondition::EConditionOperators::c_GT)
  .value("<", Belle2::ModuleCondition::EConditionOperators::c_ST)
  .value(">=", Belle2::ModuleCondition::EConditionOperators::c_GE)
  .value("<=", Belle2::ModuleCondition::EConditionOperators::c_SE)
  .value("==", Belle2::ModuleCondition::EConditionOperators::c_EQ)
  .value("!=", Belle2::ModuleCondition::EConditionOperators::c_NE)
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
  class_<Module, PyModule>("Module", R"(
Base class for Modules.

A module is the smallest building block of the framework.
A typical event processing chain consists of a Path containing
modules. By inheriting from this base class, various types of
modules can be created.

Each module is identified by its unique name, and should end in ...Module.
To make the module 'SomeRecoModule' known to the framework, use REG_MODULE(SomeReco).
It will then show up in the module list as 'SomeReco'.

Modules can also define a return value (int or bool) using setReturnValue(),
which can be used in the steering file to split the Path based on the set value:
    >>> module_with_condition.if_value("<1", another_path)

In case the module condition for a given event is less than 1, the execution
will be diverted into another_path for this event. You could for example set
a special return value if an error occurs, and divert the execution into a
path containing RootOutput if it is found; saving only the data producing/
produced by the error.

The 'Module Development' section in the manual provides detailed information
on how to create modules, setting parameters, or using return values/conditions:
https://belle2.cc.kek.jp/~twiki/bin/view/Software/Basf2manual#Module_Development)")
  .def("__str__", &Module::getPathString)
  .def("name", &Module::getName, return_value_policy<copy_const_reference>(),
       "Returns the name of the module. Can be changed via set_name(), use type() for identifying a particular module class.")
  .def("type", &Module::getType, return_value_policy<copy_const_reference>(),
       "Returns the type of the module (i.e. class name minus 'Module')")
  .def("set_name", &Module::setName)
  .def("description", &Module::getDescription, return_value_policy<copy_const_reference>())
  .def("package", &Module::getPackage, return_value_policy<copy_const_reference>())
  .def("available_params", &_getParamInfoListPython, "Return list of all module parameters")
  .def("has_properties", &Module::hasProperties)
  .def("set_property_flags", &Module::setPropertyFlags)
  .def("if_value", &Module::if_value, if_value_overloads())
  .def("if_false", &Module::if_false, if_false_overloads())
  .def("if_true", &Module::if_true, if_true_overloads())
  .def("has_condition", &Module::hasCondition)
  .def("get_condition_path", &Module::getConditionPath)
  .def("get_condition_option", &Module::getAfterConditionPath)
  .def("get_condition_value", &_getConditionValuePython)
  .def("get_condition_operator", &_getConditionOperatorPython)
  .add_property("logging",
                make_function(&Module::getLogConfig, return_value_policy<reference_existing_object>()),
                &Module::setLogConfig)
  .def("param", &Module::setParamPython, "Set parameter with given name to value.", args("name", "value"))
  .def("param", &Module::setParamPythonDict,
       "Set parameters using a dictionary: parameters given by the dictionary keys will be set to the corresponding values.")
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
