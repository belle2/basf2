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
    return;
  }
  m_elements.push_back(path);
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


ModulePtrList Path::buildModulePathList() const
{
  ModulePtrList tmpModuleList;

  //Build list of modules recursively by following the conditions of modules
  this->fillModulePathList(tmpModuleList);

  return tmpModuleList;
}


//============================================================================
//                              Private methods
//============================================================================

void Path::fillModulePathList(ModulePtrList& modList) const
{
  ModulePtrList::const_iterator moduleIter;
  const ModulePtrList& currModList = getModules();

  for (moduleIter = currModList.begin(); moduleIter != currModList.end(); ++moduleIter) {
    const Module* module = moduleIter->get();

    //If module was not already added to the list, add it.
    ModulePtrList::iterator findIter = find_if(modList.begin(), modList.end(), bind2nd(ModulePtrOperatorsEq(), *moduleIter));
    if (findIter == modList.end()) {
      modList.push_back(*moduleIter);

      //If the module has a condition, call the method recursively
      if (module->hasCondition()) {
        module->getConditionPath()->fillModulePathList(modList);
      }
    }
  }
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
  const std::list<ModulePtr>& modules = path->getModules();

  for (std::list<ModulePtr>::const_iterator listIter = modules.begin(); listIter != modules.end(); ++listIter)
    returnList.append(boost::python::object(ModulePtr(*listIter)));

  return returnList;
}


void Path::exposePythonAPI()
{
  class_<Path>("Path", no_init)
  .def("__str__", &Path::getPathString)
  .def("add_module", &Path::addModule)
  .def("add_path", &Path::addPath)
  .def("modules", &_getModulesPython)
  ;

  register_ptr_to_python<PathPtr>();
}
