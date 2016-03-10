/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python/register_ptr_to_python.hpp>
#include <boost/python/class.hpp>
#include <boost/python/list.hpp>

#include <framework/core/Path.h>
#include <framework/core/Module.h>
#include <framework/core/ModuleManager.h>
#include <framework/core/SubEventModule.h>
#include <framework/core/SwitchDataStoreModule.h>

using namespace Belle2;
using namespace boost::python;


Path::Path()
{

}


Path::~Path()
{

}


void Path::addModule(ModulePtr module)
{
  m_elements.push_back(module);
}

void Path::addPath(PathPtr path)
{
  if (path.get() == this) {
    B2FATAL("Attempting to add a path to itself!");
  }
  m_elements.push_back(path);
}

bool Path::isEmpty() const
{
  return m_elements.empty();
}

std::list<ModulePtr> Path::getModules() const
{
  std::list<ModulePtr> modules;
  for (const boost::shared_ptr<PathElement>& elem : m_elements) {
    if (dynamic_cast<Module*>(elem.get()) != 0) {
      //this path element is a Module, create a ModulePtr that shares ownership with 'elem'
      modules.push_back(boost::static_pointer_cast<Module>(elem));
    } else {
      //some PathElement with submodules
      const std::list<ModulePtr>& modulesInElem = elem->getModules();
      modules.insert(modules.end(), modulesInElem.begin(), modulesInElem.end());
    }
  }
  return modules;
}


ModulePtrList Path::buildModulePathList(bool unique) const
{
  ModulePtrList modList;
  for (const ModulePtr& module : getModules()) {
    if (!unique or find(modList.begin(), modList.end(), module) == modList.end()) {
      modList.push_back(module);

      //If the module has a condition, call the method recursively
      if (module->hasCondition()) {
        const std::list<ModulePtr>& modulesInElem = module->getConditionPath()->buildModulePathList(unique);
        modList.insert(modList.end(), modulesInElem.begin(), modulesInElem.end());
      }
    }
  }

  return modList;
}

void Path::putModules(const std::list<boost::shared_ptr<Module> >& mlist)
{
  m_elements.assign(mlist.begin(), mlist.end());
}


void Path::forEach(std::string loopObjectName, std::string arrayName, PathPtr path)
{
  ModulePtr module = ModuleManager::Instance().registerModule("SubEvent");
  static_cast<SubEventModule&>(*module).initSubEvent(loopObjectName, arrayName, path);
  addModule(module);
}

void Path::addSkimPath(PathPtr skim_path, std::string ds_ID)
{
  ModulePtr switchStart = ModuleManager::Instance().registerModule("SwitchDataStore");
  static_cast<SwitchDataStoreModule&>(*switchStart).init(ds_ID, true);
  ModulePtr switchEnd = ModuleManager::Instance().registerModule("SwitchDataStore");
  static_cast<SwitchDataStoreModule&>(*switchEnd).init("", false);
  switchStart->setName("SwitchDataStore ('' -> '" + ds_ID + "')");
  switchEnd->setName("SwitchDataStore ('' <- '" + ds_ID + "')");

  auto flag = Module::c_ParallelProcessingCertified;
  //set c_ParallelProcessingCertified flag if _all_ modules have it set
  bool allCertified = true;
  for (const auto& mod : buildModulePathList()) {
    if (!mod->hasProperties(flag)) {
      allCertified = false;
      break;
    }
  }
  if (allCertified) {
    switchStart->setPropertyFlags(flag);
    switchEnd->setPropertyFlags(flag);
  }

  addModule(switchStart);
  addPath(skim_path);
  addModule(switchEnd);
}

bool Path::contains(std::string moduleType) const
{
  const std::list<ModulePtr>& modules = getModules();

  auto sameTypeFunc = [moduleType](ModulePtr m) -> bool { return m->getType() == moduleType; };
  return std::find_if(modules.begin(), modules.end(), sameTypeFunc) != modules.end();
}

boost::shared_ptr<PathElement> Path::clone() const
{
  PathPtr path(new Path);
  for (const auto& elem : m_elements) {
    const Module* m = dynamic_cast<const Module*>(elem.get());
    if (m and m->getType() == "PyModule") {
      //B2WARNING("Python module " << m->getName() << " encountered, please make sure it correctly reinitialises itself to ensure multiple process() calls work.");
      path->addModule(boost::static_pointer_cast<Module>(elem));
    } else {
      path->m_elements.push_back(elem->clone());
    }
  }
  return path;
}


//=====================================================================
//                          Python API
//=====================================================================

std::string Path::getPathString() const
{
  std::string out("");
  bool firstElem = true;
  for (const boost::shared_ptr<PathElement>& elem : m_elements) {
    if (!firstElem) {
      out += " -> ";
    } else {
      firstElem = false;
    }

    out += elem->getPathString();
  }
  return "[" + out + "]";
}


/**
 * Returns a list of the modules in given path (as python list).
 */
boost::python::list _getModulesPython(const Path* path)
{
  boost::python::list returnList;

  for (const ModulePtr& module : path->getModules())
    returnList.append(boost::python::object(ModulePtr(module)));

  return returnList;
}

void Path::exposePythonAPI()
{
  class_<Path>("Path", no_init)
  .def("__str__", &Path::getPathString)
  .def("_add_module_object", &Path::addModule)
  .def("add_path", &Path::addPath)
  .def("modules", &_getModulesPython)
  .def("for_each", &Path::forEach)
  .def("_add_skim_path", &Path::addSkimPath)
  .def("__contains__", &Path::contains)
  ;

  register_ptr_to_python<PathPtr>();
}
