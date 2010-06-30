/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <fwcore/Framework.h>
#include <fwcore/ModuleManager.h>
#include <logging/Logger.h>
#include <logging/LogConnectionIOStream.h>
#include <logging/LogConnectionTxtFile.h>

#include <datastore/StoreObjPtr.h>
#include <datastore/StoreDefs.h>
#include <datastore/EventMetaData.h>


using namespace std;
using namespace Belle2;

using namespace boost::python;


Framework::Framework()
{
  m_moduleChain = new ModuleChain();
}


Framework::~Framework()
{
  delete m_moduleChain;
}


void Framework::addLibFileExtension(const std::string& fileExt)
{
  ModuleManager::Instance().addLibFileExtension(fileExt);
}


void Framework::loadModuleLibs(const std::string& path)
{
  ModuleManager::Instance().loadModuleLibs(path);
}


ModulePtr Framework::registerModule(const std::string& type) throw(FwExcModuleNotRegistered)
{
  return m_moduleChain->registerModule(type);
}


PathPtr Framework::createPath() throw(FwExcPathNotCreated)
{
  try {
    return m_moduleChain->addPath();
  } catch (FwExcPathNotAdded& exc) {
    throw FwExcPathNotCreated();
  }
}


void Framework::setLoggingToShell()
{
  LogSystem::Instance().setLogConnection(new LogConnectionIOStream(std::cout));
}


void Framework::setLoggingToTxtFile(const std::string& filename, bool append)
{
  LogSystem::Instance().setLogConnection(new LogConnectionTxtFile(filename, append));
}


void Framework::process(PathPtr startPath)
{
  //Get list of modules which could be executed during the data processing.
  ModulePtrList moduleList = m_moduleChain->getModulePathList(startPath);

  //Initialize modules
  m_moduleChain->processInitialize(moduleList);

  //Check if there is at least one module in the chain, which can specify the end of the data flow.
  ModulePtrList selEoDModuleList = m_moduleChain->getModulesByProperties(moduleList, Module::c_TriggersEndOfData);
  if (selEoDModuleList.size() == 0) {
    ERROR("There must be at least one module in the chain which can specify the end of the data flow.")
    return;
  }

  //Check if there is exactly one module in the chain, which can specify the begin of a new run.
  ModulePtrList selBnRModuleList = m_moduleChain->getModulesByProperties(moduleList, Module::c_TriggersNewRun);
  if (selBnRModuleList.size() != 1) {
    ERROR("There are currently " << selBnRModuleList.size() << " modules in the chain which specify the beginning of a new run. There is exactly one module of this type allowed.")
    return;
  }

  //Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogCommon::c_Error);
  if (numLogError > 0) {
    ERROR(numLogError << " ERROR(S) occurred ! The processing of events will not be started.");
    return;
  }

  m_moduleChain->processChain(startPath, moduleList);
  m_moduleChain->processTerminate(moduleList);
}


void Framework::process(PathPtr startPath, long maxEvent)
{
  //Get list of modules which could be executed during the data processing.
  ModulePtrList moduleList = m_moduleChain->getModulePathList(startPath);

  if (maxEvent < 0) {
    ERROR("The maximum event number has to be 0 or greater. Current value is: " << maxEvent)
    return;
  }

  m_moduleChain->processInitialize(moduleList);

  //Check if there is exactly one module in the chain, which can specify the begin of a new run.
  ModulePtrList selBnRModuleList = m_moduleChain->getModulesByProperties(moduleList, Module::c_TriggersNewRun);
  if (selBnRModuleList.size() != 1) {
    ERROR("There are currently " << selBnRModuleList.size() << " modules in the chain which specify the beginning of a new run. There is exactly one module of this type allowed.")
    return;
  }

  //Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogCommon::c_Error);
  if (numLogError > 0) {
    ERROR(numLogError << " ERROR(S) occurred ! The processing of events will not be started.");
    return;
  }

  m_moduleChain->processChain(startPath, moduleList, maxEvent);
  m_moduleChain->processTerminate(moduleList);
}


void Framework::process(PathPtr startPath, long maxEvent, unsigned long runNumber)
{
  //Get list of modules which could be executed during the data processing.
  ModulePtrList moduleList = m_moduleChain->getModulePathList(startPath);

  if (maxEvent < 0) {
    ERROR("The maximum event number has to be 0 or greater. Current value is: " << maxEvent)
    return;
  }

  m_moduleChain->processInitialize(moduleList);

  //Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogCommon::c_Error);
  if (numLogError > 0) {
    ERROR(numLogError << " ERROR(S) occurred ! The processing of events will not be started.");
    return;
  }

  //Store the run number in the MetaData object
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", c_Persistent);
  eventMetaDataPtr->setRun(runNumber);

  //Start event processing
  m_moduleChain->processBeginRun(moduleList);
  m_moduleChain->processChain(startPath, moduleList, maxEvent);
  m_moduleChain->processTerminate(moduleList);
}


//=====================================================================
//                          Python API
//=====================================================================

boost::python::list Framework::getAvailModulesPython() const
{
  boost::python::list returnList;
  std::list<ModulePtr> avModList = ModuleManager::Instance().getAvailableModules();

  for (std::list<ModulePtr>::iterator listIter = avModList.begin(); listIter != avModList.end(); listIter++) {
    returnList.append(boost::python::object(ModulePtr(*listIter)));
  }

  return returnList;
}


boost::python::dict Framework::getLogStatisticPython() const
{
  boost::python::dict returnDict;
  LogSystem& logSys = LogSystem::Instance();

  for (int iLevel = 0; iLevel < LogCommon::ELogLevelCount; ++iLevel) {
    LogCommon::ELogLevel logLevel = static_cast<LogCommon::ELogLevel>(iLevel);
    returnDict[boost::python::object(LogCommon::logLevelToString(logLevel))] = boost::python::object(logSys.getMessageCounter(logLevel));
  }

  return returnDict;
}


void translateExcModuleNotRegistered(FwExcModuleNotRegistered const& exc)
{
  // Use the Python 'C' API to set up an exception object
  string excMessage = "The module \"" + exc.getModuleType() + "\" could not be registered !";
  PyErr_SetString(PyExc_RuntimeError, excMessage.c_str());
}


void translateExcPathNotCreated(FwExcPathNotCreated const&)
{
  // Use the Python 'C' API to set up an exception object
  string excMessage = "Could not create path !";
  PyErr_SetString(PyExc_RuntimeError, excMessage.c_str());
}


void Framework::exposePythonAPI()
{
  //Overloaded process methods
  void(Framework::*process1)(PathPtr) = &Framework::process;
  void(Framework::*process2)(PathPtr, long) = &Framework::process;
  void(Framework::*process3)(PathPtr, long, unsigned long) = &Framework::process;

  //Expose framework class
  class_<Framework>("Framework")
  .def("add_lib_extension", &Framework::addLibFileExtension)
  .def("load_module_libs", &Framework::loadModuleLibs)
  .def("available_modules", &Framework::getAvailModulesPython)
  .def("register_module", &Framework::registerModule)
  .def("create_path", &Framework::createPath)
  .def("process", process1)
  .def("process", process2)
  .def("process", process3)
  .def("log_to_shell", &Framework::setLoggingToShell)
  .def("log_to_txtfile", &Framework::setLoggingToTxtFile)
  .def("log_statistics", &Framework::getLogStatisticPython)
  ;

  //Register exception translators
  register_exception_translator<FwExcModuleNotRegistered>(&translateExcModuleNotRegistered);
  register_exception_translator<FwExcPathNotCreated>(&translateExcPathNotCreated);
}
