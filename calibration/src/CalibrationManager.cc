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

#include <fstream>
#include <sstream>
#include <iostream>

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
  m_calibrationModules.push_back(module);

  int iter = 0;
  bool someStateChanged = true;
  while (someStateChanged) {
    someStateChanged = false;
    if (iter++ > 10000)
      B2FATAL("Over 10000 iterations performed in resolving module dependencies. Some fancy circular dependency turning modules on/off continously?");
    // Refresh states of all registered modules
    for (CalibrationModule* mod : m_calibrationModules) {
      // Only if the module is in waiting state (default), set it
      // to running state (if done/failed/monitoring - do not change the state)

      for (ExpRunRange& iov : mod->getCalibrationIOVs()) {
        if (checkDependencies(mod, iov)) {
          if (mod->getState(iov) == CalibrationModule::c_Waiting) {
            mod->setState(iov, CalibrationModule::c_Running, mod->getNumberOfIterations(iov));
            someStateChanged = true;
          }
        } else if (mod->getState(iov) == CalibrationModule::c_Running) {
          mod->setState(iov, CalibrationModule::c_Waiting, mod->getNumberOfIterations(iov));
          someStateChanged = true;
        }
      }
    }
  }
}

bool CalibrationManager::checkDependencies(CalibrationModule* module, const ExpRunRange& iov)
{
  for (auto& dep : module->getDependencies())
    for (CalibrationModule* mod : m_calibrationModules)
      if ((mod->getName() == dep.first))
        for (ExpRunRange& modiov : mod->getCalibrationIOVs())
          if (iov.overlaps(modiov))
            if (mod->getState(modiov) != dep.second)
              return false;

  return true;
}

void CalibrationManager::loadCachedState(std::string module_to_find, ExpRunRange iov_to_find,
                                         CalibrationModule::ECalibrationModuleState& state_to_load, int& iteration_to_load)
{
  ifstream cache(m_stateFileName);
  if (!cache.is_open())
    return;

  string line;

  while (getline(cache, line)) {
    string module = "";
    ExpRunRange iov;
    string state = "blocked";
    int iteration = 0;

    stringstream stream;
    stream << line;
    stream >> module >> iov >> state >> iteration;

    if (module == module_to_find && iov == iov_to_find) {
      state_to_load = CalibrationModule::stringToState(state);
      iteration_to_load = iteration;
      return;
    }
  }
}

void CalibrationManager::saveCachedStates()
{
  ofstream cache(m_stateFileName);
  for (CalibrationModule* module : m_calibrationModules) {
    if (!module->isCalibrator())
      continue;
    for (ExpRunRange& iov : module->getCalibrationIOVs()) {
      cache << module->getName() << " " << iov << " " << CalibrationModule::stateToString(module->getState(
              iov)) << " " << module->getNumberOfIterations(iov) << endl;
    }
  }
}

bool CalibrationManager::done()
{
  for (CalibrationModule* module : m_calibrationModules) {
    for (ExpRunRange& iov : module->getCalibrationIOVs()) {
      if (module->isCalibrator() && (module->getState(iov) != CalibrationModule::c_Done
                                     && module->getState(iov) != CalibrationModule::c_Failed))
        return false;
    }
  }
  return true;
}
