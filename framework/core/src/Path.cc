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
#include <boost/python/docstring_options.hpp>

#include <framework/core/Path.h>
#include <framework/core/Module.h>
#include <framework/core/ModuleManager.h>
#include <framework/core/SubEventModule.h>
#include <framework/core/SwitchDataStoreModule.h>
#include <framework/core/PyObjConvUtils.h>

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
  for (const std::shared_ptr<PathElement>& elem : m_elements) {
    if (dynamic_cast<Module*>(elem.get()) != 0) {
      //this path element is a Module, create a ModulePtr that shares ownership with 'elem'
      modules.push_back(std::static_pointer_cast<Module>(elem));
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
        for (const auto& conditionPath : module->getAllConditionPaths()) {
          const std::list<ModulePtr>& modulesInElem = conditionPath->buildModulePathList(unique);
          modList.insert(modList.end(), modulesInElem.begin(), modulesInElem.end());
        }
      }
    }
  }

  return modList;
}

void Path::putModules(const std::list<std::shared_ptr<Module> >& mlist)
{
  m_elements.assign(mlist.begin(), mlist.end());
}


void Path::forEach(std::string loopObjectName, std::string arrayName, PathPtr path)
{
  ModulePtr module = ModuleManager::Instance().registerModule("SubEvent");
  static_cast<SubEventModule&>(*module).initSubEvent(loopObjectName, arrayName, path);
  addModule(module);
}

void Path::addIndependentPath(PathPtr independent_path, std::string ds_ID, boost::python::list merge_back)
{
  if (ds_ID.empty()) {
    static int dscount = 1;
    ds_ID = "DS " + std::to_string(dscount++);
  }
  auto mergeBack = PyObjConvUtils::convertPythonObject(merge_back, std::vector<std::string>());
  ModulePtr switchStart = ModuleManager::Instance().registerModule("SwitchDataStore");
  static_cast<SwitchDataStoreModule&>(*switchStart).init(ds_ID, true, mergeBack);
  ModulePtr switchEnd = ModuleManager::Instance().registerModule("SwitchDataStore");
  static_cast<SwitchDataStoreModule&>(*switchEnd).init("", false, mergeBack);
  switchStart->setName("SwitchDataStore ('' -> '" + ds_ID + "')");
  switchEnd->setName("SwitchDataStore ('' <- '" + ds_ID + "')");

  //set c_ParallelProcessingCertified flag if _all_ modules have it set
  auto flag = Module::c_ParallelProcessingCertified;
  if (ModuleManager::allModulesHaveFlag(buildModulePathList(), flag)) {
    switchStart->setPropertyFlags(flag);
    switchEnd->setPropertyFlags(flag);
  }

  addModule(switchStart);
  addPath(independent_path);
  addModule(switchEnd);
}

bool Path::contains(std::string moduleType) const
{
  const std::list<ModulePtr>& modules = getModules();

  auto sameTypeFunc = [moduleType](ModulePtr m) -> bool { return m->getType() == moduleType; };
  return std::find_if(modules.begin(), modules.end(), sameTypeFunc) != modules.end();
}

std::shared_ptr<PathElement> Path::clone() const
{
  PathPtr path(new Path);
  for (const auto& elem : m_elements) {
    const Module* m = dynamic_cast<const Module*>(elem.get());
    if (m and m->getType() == "PyModule") {
      //B2WARNING("Python module " << m->getName() << " encountered, please make sure it correctly reinitialises itself to ensure multiple process() calls work.");
      path->addModule(std::static_pointer_cast<Module>(elem));
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
  for (const std::shared_ptr<PathElement>& elem : m_elements) {
    if (!firstElem) {
      out += " -> ";
    } else {
      firstElem = false;
    }

    out += elem->getPathString();
  }
  return "[" + out + "]";
}


namespace {
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
}

void Path::exposePythonAPI()
{
  docstring_options options(true, true, false); //userdef, py sigs, c++ sigs

  class_<Path>("Path",
               R"(Implements a path consisting of Module and/or Path objects (arranged in a linear order). Use `basf2.create_path()` to create a new object.

.. seealso:: :func:`basf2.process`)")
  .def("__str__", &Path::getPathString)
  .def("_add_module_object", &Path::addModule) // actual add_module() is found in basf2.py
  .def("add_path", &Path::addPath, args("path"), R"(Insert another path at the end of this one.
For example,

    >>> path.add_module('A')
    >>> path.add_path(otherPath)
    >>> path.add_module('B')

would create a path [ A -> [ contents of otherPath ] -> B ].)")
  .def("modules", &_getModulesPython, "Returns an ordered list of all modules in this path.")
  .def("for_each", &Path::forEach, R"(Similar to add_path(), this will execute path at the current position, but
will run it once for each object in the given array 'arrayName', and set the loop variable
'loopObjectName' (a StoreObjPtr of same type as array) to the current object.

Main use case is after using the RestOfEventBuilder on a ParticeList, where
you can use this feature to perform actions on only a part of the event
for a given list of candidates:

    >>> path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
    read: for each  $objName   in $arrayName   run over $path

If 'RestOfEvents' contains two elements, during the execution of roe_path a StoreObjectPtr 'RestOfEvent'
will be available, which will point to the first element in the first run, and the second element
in the second run. You can use the variable 'isInRestOfEvent' to select Particles that
originate from this part of the event.

Changes to existing arrays / objects will be available to all modules after the for_each(),
including those made to the loop variable (it will simply modify the i'th item in the array looped over.)
Arrays / objects of event durability created inside the loop will however be limited to the validity of the loop variable. That is,
creating a list of Particles matching the current MCParticle (loop object) will no longer exist after switching
to the next MCParticle or exiting the loop.)", args("loopObjectName", "arrayName", "path"))
  .def("_add_independent_path", &Path::addIndependentPath)
  .def("__contains__", &Path::contains, R"(Does this Path contain a module of the given type?

    >>> path = create_path()
    >>> 'RootInput' in path
    False
    >>> path.add_module('RootInput')
    >>> 'RootInput' in path
    True)", args("moduleType"))
  ;

  register_ptr_to_python<PathPtr>();
}
