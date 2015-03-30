/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *               Sergey Yashchenko (sergey.yaschenko@desy.de)             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <calibration/CalibrationManager.h>
#include <calibration/CalibrationModule.h>

#include <framework/pcore/ProcHandler.h>
#include <framework/core/Environment.h>
#include <framework/core/ModuleManager.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace calibration;
//---------------------------------------------------------------------------------------
// Singleton implementation
//---------------------------------------------------------------------------------------

// Initialization of static member to check for non-existence of the singleton
bool CalibrationManager::m_calibrationManagerDestroyed = false;

CalibrationManager& CalibrationManager::getInstance()
{
  if (m_calibrationManagerDestroyed)
    B2FATAL("The CalibrationManager singleton was already destroyed and Instance() was called. Didn't you call it from some destructor?");

  static CalibrationManager theInstance;
  return theInstance;
}

CalibrationManager::~CalibrationManager()
{
  m_calibrationManagerDestroyed = true;
}

CalibrationManager::CalibrationManager() : m_numberOfIterations(0)
{
  if (ProcHandler::EvtProcID() != -1)
    B2FATAL("CalibrationManager was initialized out of the main process.");

  m_calibrationModules.clear();
}
//---------------------------------------------------------------------------------------

int CalibrationManager::getNumberOfIterations() const
{
  return m_numberOfIterations;
}

void CalibrationManager::setNumberOfIterations(int numberOfIterations)
{
  m_numberOfIterations = numberOfIterations;
}

void CalibrationManager::register_module(CalibrationModule* module)
{
  // Correct order of registration is checked here
  if (!checkDependencies(module, true)) {
    B2FATAL("Dependencies not fullfiled.");
  }

  m_calibrationModules.push_back(module);

  // Refresh states of all registered modules
  for (CalibrationModule* mod : m_calibrationModules) {
    // Only if the module is in waiting state (default), set it
    // to running state (if done/failed/monitoring - do not change the state)
    if (checkDependencies(mod, false) && mod->getState() == CalibrationModule::c_Waiting)
      mod->setState(CalibrationModule::c_Running);
  }
}

bool CalibrationManager::checkDependencies(CalibrationModule* module, bool ignoreState)
{
  for (auto& dep : module->getDependencies()) {
    bool depOK = false;
    for (CalibrationModule* mod : m_calibrationModules) {
      if ((mod->getName() == dep.first)) {
        if (ignoreState)
          depOK = true;
        else if (mod->getState() == dep.second)
          depOK = true;
      }
    }
    if (!depOK)
      return false;
  }
  return true;
}
