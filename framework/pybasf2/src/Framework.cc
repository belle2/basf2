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

#include <framework/core/PyObjConvUtils.h>
#include <framework/core/Environment.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/ModuleManager.h>
#include <framework/datastore/DataStore.h>
#include <framework/database/DBStore.h>
#include <framework/pcore/pEventProcessor.h>

#include <framework/logging/Logger.h>
#include <framework/logging/LogSystem.h>

#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/object.hpp>
#include <boost/python/class.hpp>
#include <boost/python/overloads.hpp>
#include <boost/python/exception_translator.hpp>

#include <set>

using namespace boost::python;
using namespace Belle2;


Framework::Framework()
{
  DataStore::s_DoCleanup = true;
  LogSystem::Instance().enableErrorSummary(true);

  RandomNumbers::initialize();
  Environment::Instance();
}


Framework::~Framework()
{
  //empty module manager of modules
  //since modules may contain shared pointers of Path objects created in Python,
  //these shared pointers have special cleanup hooks that can cause crashes if run
  //after Py_Finalize(). The framework object is cleaned up before, so this is a good place.
  ModuleManager::Instance().reset();
  DataStore::s_DoCleanup = false;
}


void Framework::addModuleSearchPath(const std::string& path)
{
  ModuleManager::Instance().addModuleSearchPath(path);
}


void Framework::setExternalsPath(const std::string& path)
{
  Environment::Instance().setExternalsPath(path);
}


ModulePtr Framework::registerModule(const std::string& moduleName)
{
  return ModuleManager::Instance().registerModule(moduleName);
}


ModulePtr Framework::registerModule(const std::string& moduleName, const std::string& sharedLibPath)
{
  return ModuleManager::Instance().registerModule(moduleName, sharedLibPath);
}


void Framework::process(PathPtr startPath, long maxEvent)
{
  if (Environment::Instance().getDryRun()) {
    Environment::Instance().setJobInformation(startPath);
    return; //processing disabled!
  }

  static bool already_executed = false;
  static std::set<const Module*> previously_run_modules; //not a shared pointer to not screw up ownership
  static int errors_from_previous_run = 0;
  const auto moduleListUnique = startPath->buildModulePathList(true);
  if (already_executed) {
    B2WARNING("Calling process() more than once per steering file is still experimental, please check results carefully! Python modules especially should reinitialise their state in initialise() to avoid problems");
    if (startPath->buildModulePathList(true) != startPath->buildModulePathList(false)) {
      B2FATAL("Your path contains the same module instance in multiple places. Calling process() multiple times is not implemented for this case.");
    }

    //were any modules in moduleListUnique already run?
    for (auto m : moduleListUnique) {
      if (previously_run_modules.count(m.get()) > 0) {
        //only clone if modules have been run before
        startPath = std::static_pointer_cast<Path>(startPath->clone());
        break;
      }
    }
  }
  for (auto m : moduleListUnique) {
    previously_run_modules.insert(m.get());
  }

  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError != errors_from_previous_run) {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  try {
    LogSystem::Instance().resetMessageCounter();
    DataStore::Instance().reset();
    DataStore::Instance().setInitializeActive(true);

    already_executed = true;
    if (Environment::Instance().getNumberProcesses() == 0) {
      EventProcessor processor;
      processor.setProfileModuleName(Environment::Instance().getProfileModuleName());
      processor.process(startPath, maxEvent);
    } else {
      pEventProcessor processor;
      processor.process(startPath, maxEvent);
    }
    errors_from_previous_run = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);

    DBStore::Instance().reset();
  } catch (std::exception& e) {
    B2ERROR("Uncaught exception encountered: " << e.what()); //should show module name
    DataStore::Instance().reset(); // ensure we are executed before ROOT's exit handlers
    throw; //and let python's global handler do the rest
  } catch (...) {
    B2ERROR("Uncaught exception encountered!"); //should show module name
    DataStore::Instance().reset(); // ensure we are executed before ROOT's exit handlers
    throw; //and let python's global handler do the rest
    //TODO: having a stack trace would be nicer, but somehow a handler I set using std::set_terminate() never gets called
  }
}


void Framework::setNumberProcesses(int numProcesses)
{
  Environment::Instance().setNumberProcesses(numProcesses);
}


int Framework::getNumberProcesses() const
{
  return Environment::Instance().getNumberProcesses();
}


void Framework::setPicklePath(std::string path)
{
  Environment::Instance().setPicklePath(path);
}


std::string Framework::getPicklePath() const
{
  return Environment::Instance().getPicklePath();
}

void Framework::setStreamingObjects(boost::python::list streamingObjects)
{
  auto vec = PyObjConvUtils::convertPythonObject(streamingObjects, std::vector<std::string>());
  Environment::Instance().setStreamingObjects(vec);
}

//=====================================================================
//                          Python API
//=====================================================================

boost::python::list Framework::getModuleSearchPathsPython() const
{
  boost::python::list returnList;

  for (std::string path : ModuleManager::Instance().getModuleSearchPaths())
    returnList.append(boost::python::object(path));
  return returnList;
}


boost::python::dict Framework::getAvailableModulesPython() const
{
  boost::python::dict returnDict;
  for (auto modulePair : ModuleManager::Instance().getAvailableModules())
    returnDict[boost::python::object(modulePair.first)] = boost::python::object(modulePair.second);
  return returnDict;
}


boost::python::list Framework::getRegisteredModulesPython() const
{
  boost::python::list returnList;

  for (ModulePtr mod : ModuleManager::Instance().getCreatedModules())
    returnList.append(boost::python::object(mod));
  return returnList;
}


#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(process_overloads, process, 1, 2)
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif

namespace {
  PyObject* PyExc_ModuleNotCreatedError{nullptr};
  /** Translate the ModuleNotCreatedError to a special python exception so that
   * we can differentiate on the python side */
  void moduleNotCreatedTranslator(const ModuleManager::ModuleNotCreatedError& e)
  {
    PyErr_SetString(PyExc_ModuleNotCreatedError, e.what());
  }
}

void Framework::exposePythonAPI()
{
  PyExc_ModuleNotCreatedError = PyErr_NewExceptionWithDoc("basf2.ModuleNotCreatedError",
                                                          "This exception is raised when a basf2 module could not be created for any reason",
                                                          PyExc_RuntimeError, NULL);
  scope().attr("ModuleNotCreatedError") = handle<>(borrowed(PyExc_ModuleNotCreatedError));
  register_exception_translator<ModuleManager::ModuleNotCreatedError>(moduleNotCreatedTranslator);
  //Overloaded methods
  ModulePtr(Framework::*registerModule1)(const std::string&) = &Framework::registerModule;
  ModulePtr(Framework::*registerModule2)(const std::string&, const std::string&) = &Framework::registerModule;

  //Expose framework class
  class_<Framework>("Framework")
  .def("add_module_search_path", &Framework::addModuleSearchPath)
  .def("set_externals_path", &Framework::setExternalsPath)
  .def("list_module_search_paths", &Framework::getModuleSearchPathsPython)
  .def("list_available_modules", &Framework::getAvailableModulesPython)
  .def("register_module", registerModule1)
  .def("register_module", registerModule2)
  .def("list_registered_modules", &Framework::getRegisteredModulesPython)
  .def("process", &Framework::process, process_overloads())
  .def("get_pickle_path", &Framework::getPicklePath)
  .def("set_pickle_path", &Framework::setPicklePath)
  .def("set_nprocesses", &Framework::setNumberProcesses)
  .def("get_nprocesses", &Framework::getNumberProcesses)
  .def("set_streamobjs", &Framework::setStreamingObjects)
  ;
}
