/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <boost/python/register_ptr_to_python.hpp>
#include <boost/python/class.hpp>
#include <boost/python/list.hpp>
#include <boost/python/docstring_options.hpp>
#include <utility>

#include <framework/core/Path.h>
#include <framework/core/Module.h>
#include <framework/core/ModuleManager.h>
#include <framework/core/SubEventModule.h>
#include <framework/core/SwitchDataStoreModule.h>
#include <framework/core/MergeDataStoreModule.h>
#include <framework/core/SteerRootInputModule.h>
#include <framework/core/CreateConsistencyInfoModule.h>
#include <framework/core/PyObjConvUtils.h>

using namespace Belle2;
using namespace boost::python;

Path::Path() = default;

Path::~Path() = default;

void Path::addModule(const ModulePtr& module)
{
  m_elements.push_back(module);
}

void Path::addPath(const PathPtr& path)
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
    if (dynamic_cast<Module*>(elem.get()) != nullptr) {
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
          // Avoid loops in path conditions
          if (conditionPath.get() == this) B2FATAL("Found recursion in conditional path");
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


void Path::forEach(const std::string& loopObjectName, const std::string& arrayName, PathPtr path)
{
  ModulePtr module = ModuleManager::Instance().registerModule("SubEvent");
  static_cast<SubEventModule&>(*module).initSubEvent(loopObjectName, arrayName, std::move(path));
  addModule(module);
}

void Path::doWhile(PathPtr path, const std::string& condition, unsigned int maxIterations)
{
  ModulePtr module = ModuleManager::Instance().registerModule("SubEvent");
  static_cast<SubEventModule&>(*module).initSubLoop(std::move(path), condition, maxIterations);
  addModule(module);
}

void Path::addIndependentPath(const PathPtr& independent_path, std::string ds_ID, const boost::python::list& merge_back)
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

void Path::addIndependentMergePath(const PathPtr& independent_path, std::string ds_ID, const boost::python::list& merge_back,
                                   std::string consistency_check, bool event_mixing, bool merge_same_file)
{
  if (ds_ID.empty()) {
    static int dscount = 1;
    ds_ID = "DS " + std::to_string(dscount++);
  }
  auto mergeBack = PyObjConvUtils::convertPythonObject(merge_back, std::vector<std::string>());
  ModulePtr switchStart = ModuleManager::Instance().registerModule("MergeDataStore");
  static_cast<MergeDataStoreModule&>(*switchStart).init(ds_ID, true, mergeBack);
  ModulePtr switchEnd = ModuleManager::Instance().registerModule("MergeDataStore");
  static_cast<MergeDataStoreModule&>(*switchEnd).init("", false, mergeBack);
  switchStart->setName("MergeDataStore ('' -> '" + ds_ID + "')");
  switchEnd->setName("MergeDataStore ('' <- '" + ds_ID + "')");

  ModulePtr fillConsistencyInfo = ModuleManager::Instance().registerModule("CreateConsistencyInfo");
  static_cast<CreateConsistencyInfoModule&>(*fillConsistencyInfo).init(consistency_check, event_mixing);

  ModulePtr steerInput = ModuleManager::Instance().registerModule("SteerRootInput");
  static_cast<SteerRootInputModule&>(*steerInput).init(event_mixing, merge_same_file);

  //set c_ParallelProcessingCertified flag if _all_ modules have it set
  auto flag = Module::c_ParallelProcessingCertified;
  if (ModuleManager::allModulesHaveFlag(buildModulePathList(), flag)) {
    switchStart->setPropertyFlags(flag);
    switchEnd->setPropertyFlags(flag);
  }

  // switch to the second (empty) data store
  addModule(switchStart);
  // execute independent path
  addPath(independent_path);
  // do the merging
  addModule(switchEnd);
  // check events to be merged is consistent (typically charge)
  addModule(fillConsistencyInfo);
  // decide which events have to be processed next
  addModule(steerInput);
  // the current combination of events might not be sensible or unphysical
  // in this case end path and try next combination
  steerInput->if_value("==0", std::make_shared<Path>());
}

bool Path::contains(const std::string& moduleType) const
{
  const std::list<ModulePtr>& modules = getModules();

  auto sameTypeFunc = [moduleType](const ModulePtr & m) -> bool { return m->getType() == moduleType; };
  return std::find_if(modules.begin(), modules.end(), sameTypeFunc) != modules.end();
}

std::shared_ptr<PathElement> Path::clone() const
{
  PathPtr path(new Path);
  for (const auto& elem : m_elements) {
    const auto* m = dynamic_cast<const Module*>(elem.get());
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
  docstring_options options(true, false, false); //userdef, py sigs, c++ sigs
  using bparg = boost::python::arg;

  class_<Path>("Path",
               R"(Implements a path consisting of Module and/or Path objects (arranged in a linear order).

.. seealso:: :func:`basf2.process`)")
  .def("__str__", &Path::getPathString)
  .def("_add_module_object", &Path::addModule) // actual add_module() is found in basf2.py
  .def("add_path", &Path::addPath, args("path"), R"(add_path(path)

Insert another path at the end of this one.
For example,

    >>> path.add_module('A')
    >>> path.add_path(otherPath)
    >>> path.add_module('B')

would create a path [ A -> [ contents of otherPath ] -> B ].)

Parameters:
  path (Path): path to add to this path)")
  .def("modules", &_getModulesPython, R"(modules()

Returns an ordered list of all modules in this path.)")
  .def("for_each", &Path::forEach, R"(for_each(loop_object_name, array_name, path)

Similar to `add_path()`, this will
execute the given ``path`` at the current position, but in each event it will
execute it once for each object in the given StoreArray ``arrayName``. It will
create a StoreObject named ``loop_object_name``  of same type as array which will
point to each element in turn for each execution.

This has the effect of calling the ``event()`` methods of modules in ``path``
for each entry in ``arrayName``.

The main use case is to use it after using the `RestOfEventBuilder` on a
``ParticeList``, where you can use this feature to perform actions on only a part
of the event for a given list of candidates:

    >>> path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

You can read this as

  "for each ``RestOfEvent`` in the array of "RestOfEvents", execute ``roe_path``"

For example, if 'RestOfEvents' contains two elements then ``roe_path`` will be
executed twice and during the execution a StoreObjectPtr 'RestOfEvent' will be
available, which will point to the first element in the first execution, and
the second element in the second execution.

.. seealso::
    A working example of this `for_each` RestOfEvent is to build a veto against
    photons from :math:`\pi^0\to\gamma\gamma`. It is described in `HowToVeto`.

.. note:: This feature is used by both the `FlavorTagger` and :ref:`FullEventInterpretation` algorithms.

Changes to existing arrays / objects will be available to all modules after the
`for_each()`, including those made to the loop object itself (it will simply modify
the i'th item in the array looped over.)

StoreArrays / StoreObjects (of event durability) *created* inside the loop will
be removed at the end of each iteration. So if you create a new particle list
inside a `for_each()` path execution the particle list will not exist for the
next iteration or after the `for_each()` is complete.

Parameters:
  loop_object_name (str): The name of the object in the datastore during each execution
  array_name (str): The name of the StoreArray to loop over where the i-th
    element will be available as ``loop_object_name`` during the i-th execution
    of ``path``
  path (basf2.Path): The path to execute for each element in ``array_name``)",
       args("loop_object_name", "array_name", "path"))
  .def("do_while", &Path::doWhile, R"(do_while(path, condition='<1', max_iterations=10000)

Similar to `add_path()` this will execute a path at the current position but it
will repeat execution of this path as long as the return value of the last
module in the path fulfills the given ``condition``.

This is useful for event generation with special cuts like inclusive particle generation.

See Also:
  `Module.if_value` for an explanation of the condition expression.

Parameters:
  path (basf2.Path): sub path to execute repeatedly
  condition (str): condition on the return value of the last module in ``path``.
    The execution will be repeated as long as this condition is fulfilled.
  max_iterations (int): Maximum number of iterations per event. If this number is exceeded
    the execution is aborted.
       )", (bparg("path"), bparg("condition") = "<1", bparg("max_iterations") = 10000))
  .def("_add_independent_path", &Path::addIndependentPath)
  .def("_add_independent_merge_path", &Path::addIndependentMergePath)
  .def("__contains__", &Path::contains, R"(Does this Path contain a module of the given type?

    >>> path = basf2.Path()
    >>> 'RootInput' in path
    False
    >>> path.add_module('RootInput')
    >>> 'RootInput' in path
    True)", args("moduleType"))
  ;

  register_ptr_to_python<PathPtr>();
}
