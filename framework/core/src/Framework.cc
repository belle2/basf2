/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *               R.Itoh, addition of parallel processing function         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Framework.h>

#include <framework/logging/Logger.h>
#include <framework/logging/LogConnectionIOStream.h>
#include <framework/logging/LogConnectionTxtFile.h>
#include <framework/logging/LogConnectionFilter.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/EventMetaData.h>

#include "TDatabasePDG.h"


using namespace std;
using namespace Belle2;

using namespace boost::python;

// Static variable
int Framework::m_nproc = 0;


Framework::Framework()
{
  m_pathManager = new PathManager();
  m_eventProcessor = new EventProcessor(*m_pathManager);
  m_peventProcessor = new pEventProcessor(*m_pathManager);
  m_nproc = 0;
}


Framework::~Framework()
{
  delete m_peventProcessor;
  delete m_eventProcessor;
  delete m_pathManager;
}


void Framework::addModuleSearchPath(const string& path)
{
  ModuleManager::Instance().addModuleSearchPath(path);
}


ModulePtr Framework::registerModule(const string moduleName) throw(ModuleManager::ModuleNotCreatedError)
{
  return ModuleManager::Instance().registerModule(moduleName);
}


ModulePtr Framework::registerModule(const string moduleName, const string sharedLibPath) throw(ModuleManager::ModuleNotCreatedError)
{
  return ModuleManager::Instance().registerModule(moduleName, sharedLibPath);
}


PathPtr Framework::createPath() throw(PathManager::PathNotCreatedError)
{
  return m_pathManager->createPath();
}


void Framework::process(PathPtr startPath)
{
  if (m_nproc == 0)
    m_eventProcessor->process(startPath);
  else
    m_peventProcessor->process(startPath);

}


void Framework::process(PathPtr startPath, long maxEvent)
{
  m_eventProcessor->process(startPath, maxEvent);
}


void Framework::process(PathPtr startPath, long maxEvent, long runNumber)
{
  m_eventProcessor->process(startPath, maxEvent, runNumber);
}


void Framework::set_nprocess(int nproc)
{
  m_nproc = nproc;
  m_peventProcessor->nprocess(nproc);
}

int Framework::nprocess(void)
{
  return m_nproc;
}


bool Framework::readEvtGenTableFromFile(const std::string& filename)
{
  if (ModuleUtils::isFile(filename)) {
    TDatabasePDG::Instance()->ReadEvtGenTable(filename.c_str());
  } else {
    return false;
  }
  return true;
}


void Framework::setLogLevel(int logLevel)
{
  LogSystem::Instance().getLogConfig()->setLogLevel(static_cast<LogConfig::ELogLevel>(logLevel));
}


void Framework::setDebugLevel(int debugLevel)
{
  LogSystem::Instance().getLogConfig()->setDebugLevel(debugLevel);
}


void Framework::setAbortLevel(int abortLevel)
{
  LogSystem::Instance().getLogConfig()->setAbortLevel(static_cast<LogConfig::ELogLevel>(abortLevel));
}


void Framework::setLogInfo(int logLevel, unsigned int logInfo)
{
  LogSystem::Instance().getLogConfig()->setLogInfo(static_cast<LogConfig::ELogLevel>(logLevel), logInfo);
}


void Framework::setPackageLogLevel(std::string package, int logLevel, int debugLevel)
{
  LogSystem::Instance().addPackageLogConfig(package, LogConfig(static_cast<LogConfig::ELogLevel>(logLevel), debugLevel));
}


void Framework::addLoggingToShell(bool color)
{
  LogSystem::Instance().addLogConnection(new LogConnectionFilter(new LogConnectionIOStream(std::cout, color)));
}


void Framework::addLoggingToTxtFile(const std::string& filename, bool append)
{
  LogSystem::Instance().addLogConnection(new LogConnectionFilter(new LogConnectionTxtFile(filename, append)));
}


void Framework::resetLogging()
{
  LogSystem::Instance().resetLogConnections();
}


//=====================================================================
//                          Python API
//=====================================================================

boost::python::list Framework::getModuleSearchPathsPython() const
{
  boost::python::list returnList;

  for (std::list<string>::const_iterator listIter = ModuleManager::Instance().getModuleSearchPaths().begin();
       listIter != ModuleManager::Instance().getModuleSearchPaths().end(); listIter++) {
    returnList.append(boost::python::object(*listIter));
  }
  return returnList;
}


boost::python::dict Framework::getAvailableModulesPython() const
{
  boost::python::dict returnDict;

  for (map<string, string>::const_iterator mapIter = ModuleManager::Instance().getAvailableModules().begin();
       mapIter != ModuleManager::Instance().getAvailableModules().end(); mapIter++) {
    returnDict[boost::python::object(mapIter->first)] = boost::python::object(mapIter->second);
  }
  return returnDict;
}


boost::python::list Framework::getRegisteredModulesPython() const
{
  boost::python::list returnList;
  std::list<ModulePtr> avModList = ModuleManager::Instance().getCreatedModules();

  for (std::list<ModulePtr>::iterator listIter = avModList.begin(); listIter != avModList.end(); listIter++) {
    returnList.append(boost::python::object(ModulePtr(*listIter)));
  }
  return returnList;
}


boost::python::dict Framework::getLogStatisticPython() const
{
  boost::python::dict returnDict;
  LogSystem& logSys = LogSystem::Instance();

  for (int iLevel = 0; iLevel < LogConfig::c_Default; ++iLevel) {
    LogConfig::ELogLevel logLevel = static_cast<LogConfig::ELogLevel>(iLevel);
    returnDict[boost::python::object(LogConfig::logLevelToString(logLevel))] = boost::python::object(logSys.getMessageCounter(logLevel));
  }

  return returnDict;
}


void Framework::exposePythonAPI()
{
  //Overloaded methods
  ModulePtr(Framework::*registerModule1)(string) = &Framework::registerModule;
  ModulePtr(Framework::*registerModule2)(string, string) = &Framework::registerModule;
  void(Framework::*process1)(PathPtr) = &Framework::process;
  void(Framework::*process2)(PathPtr, long) = &Framework::process;
  void(Framework::*process3)(PathPtr, long, long) = &Framework::process;
  void(Framework::*set_nprocess)(int) = &Framework::set_nprocess;

  //Expose framework class
  class_<Framework>("Framework")
  .def("add_module_search_path", &Framework::addModuleSearchPath)
  .def("list_module_search_paths", &Framework::getModuleSearchPathsPython)
  .def("list_available_modules", &Framework::getAvailableModulesPython)
  .def("register_module", registerModule1)
  .def("register_module", registerModule2)
  .def("list_registered_modules", &Framework::getRegisteredModulesPython)
  .def("create_path", &Framework::createPath)
  .def("process", process1)
  .def("process", process2)
  .def("process", process3)
  .def("set_nprocess", set_nprocess)
  .def("read_evtgen_table", &Framework::readEvtGenTableFromFile)
  .def("set_log_level", &Framework::setLogLevel)
  .def("set_debug_level", &Framework::setDebugLevel)
  .def("set_abort_level", &Framework::setAbortLevel)
  .def("set_log_info", &Framework::setLogInfo)
  .def("set_log_package", &Framework::setPackageLogLevel)
  .def("log_to_shell", &Framework::addLoggingToShell)
  .def("log_to_txtfile", &Framework::addLoggingToTxtFile)
  .def("reset_log", &Framework::resetLogging)
  .def("get_log_statistics", &Framework::getLogStatisticPython)
  ;
}
