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
#include <boost/shared_ptr.hpp>

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
  m_hasCondition(false)
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


void Module::setCondition(const std::string& expression, boost::shared_ptr<Path> path)
{
  CondParser condParser;
  if (condParser.parseCondition(expression, m_conditionOperator, m_conditionValue)) {
    m_hasCondition = true;
    m_conditionPath = path;
  } else {
    B2ERROR("Invalid condition: could not parse condition: '" + expression + "'! The condition is NOT active!");
  }
}


void Module::setCondition(boost::shared_ptr<Path> path)
{
  CondParser condParser;
  if (condParser.parseCondition("<1", m_conditionOperator, m_conditionValue)) {
    m_hasCondition = true;
    m_conditionPath = path;
  } else {
    B2ERROR("Invalid condition: could not parse condition: '<1'! The condition is NOT active!");
  }
}


bool Module::evalCondition()
{
  if ((!m_hasCondition) || (!m_hasReturnValue)) return false;
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


void Module::setParamObject(const std::string& name, const boost::python::object& pyObj)
{
  m_moduleParamList.setParamObjectPython(name, pyObj);
}


void Module::setParamList(const std::string& name, const boost::python::list& pyList)
{
  m_moduleParamList.setParamListPython(name, pyList);
}


void Module::setParamDict(const boost::python::dict& dictionary)
{
  boost::python::list dictKeys = dictionary.keys();
  int nKey = boost::python::len(dictKeys);

  //Loop over all keys in the dictionary
  for (int iKey = 0; iKey < nKey; ++iKey) {
    boost::python::object currKey = dictKeys[iKey];
    boost::python::extract<std::string> keyProxy(currKey);

    if (keyProxy.check()) {
      boost::python::object currValue = dictionary[currKey];
      boost::python::extract<boost::python::list> listProxy(currValue);

      if (listProxy.check()) {
        setParamList(keyProxy, listProxy);
      } else {
        setParamObject(keyProxy, currValue);
      }
    } else {
      B2ERROR("Setting the module parameters from a python dictionary: invalid key in dictionary!");
    }
  }
}


//=====================================================================
//                          Python API
//=====================================================================

boost::python::list Module::getParamInfoListPython() const
{
  return m_moduleParamList.getParamInfoListPython();
}


void Module::exposePythonAPI()
{
  //For overloaded functions, member function pointer variables are needed (see boost::python documentation)
  void (Module::*setConditionString)(const std::string&, boost::shared_ptr<Path>) = &Module::setCondition;
  void (Module::*setConditionBool)(boost::shared_ptr<Path>) = &Module::setCondition;

  //Python class definition
  class_<Module, PyModule>("Module")
  .def("name", &Module::getName, return_value_policy<copy_const_reference>())
  .def("setName", &PyModule::setName)
  .def("description", &Module::getDescription, return_value_policy<copy_const_reference>())
  .def("condition", setConditionString)
  .def("condition", setConditionBool)
  .def("param", &Module::setParamObject)
  .def("param", &Module::setParamList)
  .def("param", &Module::setParamDict)
  .def("available_params", &Module::getParamInfoListPython)
  .add_property("logging",
                make_function(&Module::getLogConfig, return_value_policy<reference_existing_object>()),
                &Module::setLogConfig)
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

ModuleProxyBase::ModuleProxyBase(const std::string& moduleName) : m_moduleName(moduleName)
{
  ModuleManager::Instance().registerModuleProxy(this);
}
