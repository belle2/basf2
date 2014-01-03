/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *               R.Itoh, addition of parallel processing function         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pybasf2/Framework.h>

#include <framework/core/Environment.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/PathManager.h>
#include <framework/pcore/pEventProcessor.h>

#include <framework/logging/Logger.h>

#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/object.hpp>
#include <boost/python/class.hpp>
#include <boost/python/overloads.hpp>

using namespace std;
using namespace boost::python;
using namespace Belle2;


Framework::Framework()
{
  m_pathManager = new PathManager();

  RandomNumbers::initialize();
  Environment::Instance().setup();
}


Framework::~Framework()
{
  delete m_pathManager;

  //empty module manager of modules
  //since modules may contain shared pointers of Path objects created in Python,
  //these shared pointers have special cleanup hooks that can cause crashes if run
  //after Py_Finalize(). The framework object is cleaned up before, so this is a good place.
  ModuleManager::Instance().reset();
}


void Framework::addModuleSearchPath(const string& path)
{
  ModuleManager::Instance().addModuleSearchPath(path);
}


void Framework::setExternalsPath(const std::string& path)
{
  Environment::Instance().setExternalsPath(path);
}


ModulePtr Framework::registerModule(const string& moduleName) throw(ModuleManager::ModuleNotCreatedError)
{
  return ModuleManager::Instance().registerModule(moduleName);
}


ModulePtr Framework::registerModule(const string& moduleName, const string& sharedLibPath) throw(ModuleManager::ModuleNotCreatedError)
{
  return ModuleManager::Instance().registerModule(moduleName, sharedLibPath);
}


PathPtr Framework::createPath()
{
  return m_pathManager->createPath();
}


void Framework::process(PathPtr startPath, long maxEvent)
{
  if (Environment::Instance().getDryRun()) {
    Environment::Instance().setJobInformation(startPath);
    return; //processing disabled!
  }

  static bool already_executed = false;
  if (already_executed) {
    B2FATAL("You can only call process() once per steering file!")
    return;
  }

  try {
    already_executed = true;
    if (Environment::Instance().getNumberProcesses() == 0) {
      EventProcessor processor(*m_pathManager);
      processor.process(startPath, maxEvent);
    } else {
      pEventProcessor processor(*m_pathManager);
      processor.process(startPath, maxEvent);
    }
  } catch (std::exception& e) {
    B2ERROR("Uncaught exception encountered: " << e.what()); //should show module name
    throw; //and let python's global handler do the rest
  } catch (...) {
    B2ERROR("Uncaught exception encountered!"); //should show module name
    throw; //and let python's global handler do the rest
    //TODO: having a stack trace would be nicer, but somehow a handler I set using std::set_terminate() never gets called
  }
}


void Framework::setNumberProcesses(int number)
{
  Environment::Instance().setNumberProcesses(number);
}


int Framework::getNumberProcesses() const
{
  return Environment::Instance().getNumberProcesses();
}


//=====================================================================
//                          Python API
//=====================================================================

boost::python::list Framework::getModuleSearchPathsPython() const
{
  boost::python::list returnList;

  for (std::list<string>::const_iterator listIter = ModuleManager::Instance().getModuleSearchPaths().begin();
       listIter != ModuleManager::Instance().getModuleSearchPaths().end(); ++listIter) {
    returnList.append(boost::python::object(*listIter));
  }
  return returnList;
}


boost::python::dict Framework::getAvailableModulesPython() const
{
  boost::python::dict returnDict;

  for (map<string, string>::const_iterator mapIter = ModuleManager::Instance().getAvailableModules().begin();
       mapIter != ModuleManager::Instance().getAvailableModules().end(); ++mapIter) {
    returnDict[boost::python::object(mapIter->first)] = boost::python::object(mapIter->second);
  }
  return returnDict;
}


boost::python::list Framework::getRegisteredModulesPython() const
{
  boost::python::list returnList;
  std::list<ModulePtr> avModList = ModuleManager::Instance().getCreatedModules();

  for (std::list<ModulePtr>::iterator listIter = avModList.begin(); listIter != avModList.end(); ++listIter) {
    returnList.append(boost::python::object(ModulePtr(*listIter)));
  }
  return returnList;
}


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(process_overloads, process, 1, 2)

void Framework::exposePythonAPI()
{
  //Overloaded methods
  ModulePtr(Framework::*registerModule1)(const string&) = &Framework::registerModule;
  ModulePtr(Framework::*registerModule2)(const string&, const string&) = &Framework::registerModule;

  //Expose framework class
  class_<Framework>("Framework")
  .def("add_module_search_path", &Framework::addModuleSearchPath)
  .def("set_externals_path", &Framework::setExternalsPath)
  .def("list_module_search_paths", &Framework::getModuleSearchPathsPython)
  .def("list_available_modules", &Framework::getAvailableModulesPython)
  .def("register_module", registerModule1)
  .def("register_module", registerModule2)
  .def("list_registered_modules", &Framework::getRegisteredModulesPython)
  .def("create_path", &Framework::createPath)
  .def("process", &Framework::process, process_overloads())
  .def("set_nprocess", &Framework::setNumberProcesses)
  ;
}
