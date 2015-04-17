/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *               Sergey Yashchenko (sergey.yaschenko@desy.de)             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <calibration/CalibrationModule.h>
#include <calibration/CalibrationManager.h>
#include <framework/core/Environment.h>
#include <framework/core/ModuleManager.h>
#include <framework/pcore/ProcHandler.h>
#include <boost/algorithm/string.hpp>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>
#include <TSystem.h>

using namespace std;
using namespace Belle2;
using namespace calibration;

//-----------------------------------------------------------------
//     DO NOT            Register the Module
//-----------------------------------------------------------------
//REG_MODULE(Calibration)

CalibrationModule::CalibrationModule() : Module(),
  m_mode("offline"),
  m_state(CalibrationModule::c_Waiting),
  m_iterationNumber(0),
  m_datasetCategory(""),
  m_granularityOfCalibration("experiment"),
  m_numberOfIterations(0),
  m_calibrationFileName(""),
  //m_calibrationModuleInitialized(false),
  //m_calibrationManagerInitialized(false),
  m_dependencyList(""),
  m_calibrationFile(nullptr)
{

  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  // This description has to overridden by implementing module
  setDescription("A standard calibration module");

  // Default parameters of the base module
  addParam("datasetCategory", m_datasetCategory,
           "Used dataset category", string("data"));
  addParam("granularityOfCalibration", m_granularityOfCalibration,
           "Granularity of calibration: run, experiment, or other", string("data"));
  addParam("numberOfIterations", m_numberOfIterations,
           "Maximal number of iterations", int(1));
  addParam("calibrationFileName", m_calibrationFileName,
           "Name of the calibration output file. If empty, no file will be created. If '*' (default), it will be set to module_name.root",
           string("*"));

  m_dependencies.clear();


}

CalibrationModule::~CalibrationModule()
{

}

void CalibrationModule::initialize()
{
  // Init calibration file name
  if (m_calibrationFileName == "*")
    m_calibrationFileName = getName() + ".root";

  // Parse the dependency list
  std::vector<std::string> dependencies;
  // Split by ","
  if (m_dependencyList != "") {
    boost::algorithm::split(dependencies, m_dependencyList, boost::is_any_of(","));
    for (auto dependency : dependencies) {
      boost::trim(dependency);
      std::vector<std::string> moduleAndState;
      // split by ":"
      boost::split(moduleAndState, dependency, boost::is_any_of(":"));
      if (moduleAndState.size() == 0 || moduleAndState.size() > 2)
        B2FATAL("Error in parsing module dependencies.");

      std::string module("");
      std::string state("");

      // We allow to omit ":state" - in such case we assume ":done"
      if (moduleAndState.size() == 1) {
        module = moduleAndState[0];
        state = "done";
      } else {
        module = moduleAndState[0];
        state = moduleAndState[1];
      }
      boost::trim(module);
      boost::trim(state);
      ModuleDependency moduleDependency = std::make_pair(module, CalibrationModule::stringToState(state));
      m_dependencies.push_back(moduleDependency);
    }
  }

  CalibrationManager::getInstance().register_module(this);
  // calibManager.setNumberOfIterations(m_numberOfIterations);

  // also register histograms/ntuples:
  Prepare();

  m_iterationNumber = 0;
}

void CalibrationModule::beginRun()
{

}

void CalibrationModule::event()
{
  if (getState() == CalibrationModule::c_Running)
    CollectData();


  if (m_granularityOfCalibration == "other") {
    if (tryStartCalibration())
      m_iterationNumber++;
  }
}

void CalibrationModule::endRun()
{
  if (m_granularityOfCalibration == "run") {
    if (tryStartCalibration())
      m_iterationNumber++;
  }
}

void CalibrationModule::terminate()
{
  if (m_granularityOfCalibration == "data") {
    if (tryStartCalibration())
      m_iterationNumber++;
  }

}

void CalibrationModule::Prepare() {}

void CalibrationModule::CollectData() {}

CalibrationModule::ECalibrationModuleResult CalibrationModule::Calibrate() { return calibration::CalibrationModule::c_Success;}

CalibrationModule::ECalibrationModuleMonitoringResult CalibrationModule::Monitor() { return calibration::CalibrationModule::c_MonitoringSuccess;}

bool CalibrationModule::StoreInDataBase() { return true;}

CalibrationModule::ECalibrationModuleState CalibrationModule::stringToState(string state)
{
  std::map<ECalibrationModuleState, std::string> stateNames;
  stateNames.insert(std::make_pair(c_Waiting, "waiting"));
  stateNames.insert(std::make_pair(c_Running, "running"));
  stateNames.insert(std::make_pair(c_Monitoring, "monitoring"));
  stateNames.insert(std::make_pair(c_Done, "done"));
  stateNames.insert(std::make_pair(c_Failed, "failed"));
  for (auto stateName : stateNames) {
    if (stateName.second == state)
      return stateName.first;
  }

  B2FATAL("stringToState: Unknown CalibrationModule state!");
}

void CalibrationModule::addDefaultDependencyList(string list)
{
  addParam("Dependencies", m_dependencyList,
           "Sets dependencies on other calibration modules. Separate module names by comma. If the state of the module has to differ from 'done', use "
           "ModuleName:state where state=done|waiting|monitoring|failed|running", std::string(list));
}

TFile* CalibrationModule::getCalibrationFile()
{
  return m_calibrationFile;
}


bool CalibrationModule::tryStartCalibration()
{
  if (!CalibrationManager::getInstance().checkDependencies(this))
    return false;

  // If MP, only calibrate in output process at TERMINATE
  // but in event processes, try to close possible opened files (opened by each process individually at Collect Data)
  if (ProcHandler::parallelProcessingUsed() and !ProcHandler::isOutputProcess()) {
    if (ProcHandler::isEventProcess())
      closeParallelFiles();
    return false;
  }

  if (getCalibrationFileName() != "")  {
    m_calibrationFile = new TFile(getCalibrationFileName().c_str(), "RECREATE");
    m_calibrationFile->cd();
  }

  ECalibrationModuleResult calibrationResult;
  calibrationResult = Calibrate();

  if (m_calibrationFile) {
    m_calibrationFile->Close();
    delete m_calibrationFile;
    m_calibrationFile = nullptr;
  }

  if (calibrationResult == c_Success) {

    if (getCalibrationFileName() != "")  {
      m_calibrationFile = new TFile(getCalibrationFileName().c_str(), "RECREATE");
      m_calibrationFile->cd();
    }

    StoreInDataBase();
    setState(CalibrationModule::c_Monitoring);
    ECalibrationModuleMonitoringResult result = Monitor();

    if (m_calibrationFile) {
      m_calibrationFile->Close();
      delete m_calibrationFile;
      m_calibrationFile = nullptr;
    }

    if (result == c_MonitoringSuccess)
      setState(CalibrationModule::c_Done);
    else if (result == c_MonitoringIterateCalibration)
      setState(CalibrationModule::c_Running);
    else if (result == c_MonitoringIterateMonitor)
      setState(CalibrationModule::c_Running);
    else
      //TODO: cannot happen
      setState(CalibrationModule::c_Failed);

  } else {

    if (m_calibrationFile) {
      m_calibrationFile->Close();
      delete m_calibrationFile;
      m_calibrationFile = nullptr;
    }

    if (calibrationResult == calibration::CalibrationModule::c_Failure) {
      B2ERROR("Calibration failed.");
      setState(CalibrationModule::c_Failed);
      return false;
    }
    if (calibrationResult == calibration::CalibrationModule::c_NotEnoughData) {
      B2WARNING("Insufficient data for calibration.");
      setState(CalibrationModule::c_Waiting);
      return false;
    }
    if (calibrationResult == calibration::CalibrationModule::c_NoChange) {
      B2INFO("No change in calibration.");
      setState(CalibrationModule::c_Done);
      return false;
    }
  }
  return true;
}


