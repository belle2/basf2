/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/PathManager.h>


using namespace Belle2;
using namespace std;


PathManager::PathManager()
{

}


PathManager::~PathManager()
{

}


PathPtr PathManager::createPath() throw(PathNotCreatedError)
{
  try {
    PathPtr newPath(new Path());
    m_createdPathList.push_back(newPath);
    return newPath;
  } catch (...) {
    throw PathNotCreatedError();
  }
}


ModulePtrList PathManager::buildModulePathList(PathPtr startPath) const
{
  ModulePtrList tmpModuleList;

  //Build recursively the list of modules, by following the conditions of modules
  fillModulePathList(startPath, tmpModuleList);

  return tmpModuleList;
}


//============================================================================
//                              Private methods
//============================================================================

void PathManager::fillModulePathList(PathPtr path, ModulePtrList& modList) const
{
  ModulePtrList::const_iterator moduleIter;
  const ModulePtrList& currModList = path->getModules();

  for (moduleIter = currModList.begin(); moduleIter != currModList.end(); ++moduleIter) {
    Module* module = moduleIter->get();

    //If module was not already added to the list, add it.
    ModulePtrList::iterator findIter = find_if(modList.begin(), modList.end(), bind2nd(ModulePtrOperatorsEq(), *moduleIter));
    if (findIter == modList.end()) {
      modList.push_back(*moduleIter);

      //If the module has a condition, call the method recursively
      if (module->hasCondition()) {
        fillModulePathList(module->getConditionPath(), modList);
      }
    }
  }
}
