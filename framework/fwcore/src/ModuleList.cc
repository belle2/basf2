/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/fwcore/ModuleList.h>
#include <framework/fwcore/ModuleManager.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


ModuleList::ModuleList()
{

}


ModuleList::~ModuleList()
{

}


ModulePtr ModuleList::createModule(const string& type) throw(FwExcModuleNotCreated)
{
  try {
    ModulePtr newModule = ModuleManager::Instance().createModule(type);
    newModule->setRegisteredToFramework(true);
    push_back(newModule);
    return newModule;
  } catch (FwExcModuleNotCreated& exc) {
    ERROR("Could not create module of type '" + type + "' !");
    throw;
  }
}


//============================================================================
//                              Private methods
//============================================================================
