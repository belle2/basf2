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
#include <framework/core/InputController.h>

using namespace std;
using namespace Belle2;
using namespace calibration;

const std::map<CalibrationModule::ECalibrationModuleState, string> CalibrationModule::m_stateNames = {{
    {c_Waiting, "waiting"}, {c_Running, "running"}, {c_Monitoring, "monitoring"}, {c_Done, "done"}, {c_Failed, "failed"}, {c_Blocked, "blocked"}
  }
};

CalibrationModule::CalibrationModule() : Module(),
  m_calibration_iov(),
  m_mode("offline"),
  m_dependencyList(""),
  m_calibrationFile(nullptr)
{
  // This description has to overridden by implementing module
  setDescription("A standard calibration module");

  // Default parameters of the base module
  addParam("datasetCategory", m_datasetCategory,
           "Used dataset category", string("data"));
  addParam("granularityOfCalibration", m_granularityOfCalibration,
           "Granularity of calibration: run, experiment, or other", string("data"));
  addParam("numberOfIterations", m_numberOfIterations,
           "Maximal number of iterations", int(2));
  addParam("calibrationFileName", m_calibrationFileName,
           "Name of the calibration output file. If empty, no file will be created. If '*' (default), it will be set to module_name.root",
           string("*"));
  addParam("isCollector", m_isCollector,
           "Perform data collection with this module", bool(true));
  addParam("isCalibrator", m_isCalibrator,
           "Perform calibration with this module", bool(true));
  addParam("baseName", m_baseName,
           "Name which identifies the module (set from python). Same for calibrator and collector.", string(""));
  m_dependencies.clear();
}

CalibrationModule::~CalibrationModule() {}

void CalibrationModule::initialize()
{
  if (m_baseName == "")
    m_baseName = getName();
  // Init calibration file
  //TDirectory* saveDir = gDirectory;
  //m_calibrationFile = new TFile((getBaseName() + ".root").c_str(), "RECREATE");
  //delete m_calibrationFile;
  //saveDir->cd();


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

  // Parse the granularity
  // Split by ","
  if (m_granularityOfCalibration == "data") {
    ECalibrationModuleState state = c_Waiting;
    int iteration = 0;
    CalibrationManager::getInstance().loadCachedState(getName(), CalibrationManager::getInstance().getFullRange(), state, iteration);
    m_states.push_back(std::make_tuple(CalibrationManager::getInstance().getFullRange(), state, iteration));
  } else {
    std::vector<std::string> iovs;
    boost::algorithm::split(iovs, m_granularityOfCalibration, boost::is_any_of(","));
    for (auto iov : iovs) {
      boost::trim(iov);
      std::vector<std::string> iovData;
      // split by "."
      boost::split(iovData, iov, boost::is_any_of("."));
      if (iovData.size() != 4)
        B2FATAL("Error in parsing granularity of calibration.");

      ExpRunRange IOV(std::stoi(iovData[0]), std::stoi(iovData[1]), std::stoi(iovData[2]), std::stoi(iovData[3]));
      ECalibrationModuleState state = c_Waiting;
      int iteration = 0;
      CalibrationManager::getInstance().loadCachedState(getName(), IOV, state, iteration);
      m_states.push_back(std::make_tuple(IOV, state, iteration));
    }

  }

  CalibrationManager::getInstance().register_module(this);

  Prepare();
}

void CalibrationModule::beginRun()
{
  StoreObjPtr<EventMetaData> emd;

  // Get current calibration IOV index
  m_currentStateIndex = -1;
  for (unsigned int i = 0; i < m_states.size(); i++) {
    ExpRunRange iov = std::get<0>(m_states[i]);
    if (iov.contains(*emd)) {
      m_currentStateIndex = i;
      break;
    }
  }
  m_calibration_iov = std::get<0>(m_states[m_currentStateIndex]);

}

void CalibrationModule::event()
{
  if (m_isCollector)
    if (getState() == CalibrationModule::c_Running)
      if (CalibrationManager::getInstance().doCollection())
        CollectData();
}

void CalibrationModule::endRun()
{
  StoreObjPtr<EventMetaData> emd;

  if (m_currentStateIndex < 0)
    return;


  EventMetaData next(*emd);
  next.setRun(next.getRun() + 1);

  if (!std::get<0>(m_states[m_currentStateIndex]).contains(next))
    if (getState() == CalibrationModule::c_Running)
      if (tryStartCalibration()) {
        increaseIterations();
        m_currentStateIndex = -1;
      }
}

void CalibrationModule::terminate()
{

  if (m_currentStateIndex >= 0) {
    stringstream str;
    str << std::get<0>(m_states[m_currentStateIndex]);
    int exp, run;
    str >> exp >> run;
    StoreObjPtr<EventMetaData> emd;
    auto oldExp = emd->getExperiment();
    auto oldRun = emd->getRun();
    auto oldEvent = emd->getEvent();
    emd->setExperiment(exp);
    emd->setRun(run);
    emd->setEvent(1);

    if (getState() == CalibrationModule::c_Running)
      if (tryStartCalibration())
        increaseIterations();

    emd->setExperiment(oldExp);
    emd->setRun(oldRun);
    emd->setEvent(oldEvent);
  }

  if (ProcHandler::parallelProcessingUsed() && !ProcHandler::isOutputProcess())
    return;

  calibration::CalibrationManager::getInstance().saveCachedStates();
}

//-----------------------------------------------------------------------------------------
//   Virtual functions to be implemented by calibration modules
//-----------------------------------------------------------------------------------------
void CalibrationModule::Prepare() {}

void CalibrationModule::CollectData() {}

CalibrationModule::ECalibrationModuleResult CalibrationModule::Calibrate() { return calibration::CalibrationModule::c_Success;}

CalibrationModule::ECalibrationModuleMonitoringResult CalibrationModule::Monitor() { return calibration::CalibrationModule::c_MonitoringSuccess;}

bool CalibrationModule::StoreInDataBase() { return true;}
//-----------------------------------------------------------------------------------------

CalibrationModule::ECalibrationModuleState CalibrationModule::stringToState(string state)
{
  for (auto stateName : m_stateNames) {
    if (stateName.second == state)
      return stateName.first;
  }

  B2FATAL("stringToState: Unknown CalibrationModule state!");
}

std::string CalibrationModule::stateToString(ECalibrationModuleState state)
{
  for (auto stateName : m_stateNames) {
    if (stateName.first == state)
      return stateName.second;
  }

  B2FATAL("stateToString: Unknown CalibrationModule state!");
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
  m_calibration_iov = std::get<0>(m_states[m_currentStateIndex]);
  stringstream str;
  str << m_calibration_iov;
  string iov;
  str >> iov;
  std::replace(iov.begin(), iov.end(), ',', '_');

  //if (m_isCollector)
  closeParallelFiles();
  if (ProcHandler::parallelProcessingUsed() and !ProcHandler::isOutputProcess()) {
    return false;
  }
  if (!m_isCalibrator)
    return false;

  if (!CalibrationManager::getInstance().doCalibration()) {
    TDirectory* saveDir = gDirectory;
    if (m_calibrationFile) {
      m_calibrationFile->Close();
      delete m_calibrationFile;
      m_calibrationFile = nullptr;
    }
    m_calibrationFile = new TFile((getBaseName() + "_" + iov + ".root").c_str(), "RECREATE");
    m_calibrationFile->cd();

    storeData();

    if (m_calibrationFile) {
      m_calibrationFile->Write();
      m_calibrationFile->Close();
      delete m_calibrationFile;
      m_calibrationFile = nullptr;
    }
    saveDir->cd();

    resetData();
    return false;
  }

  if (!CalibrationManager::getInstance().doCollection()) {
    TDirectory* saveDir = gDirectory;
    if (m_calibrationFile) {
      m_calibrationFile->Close();
      delete m_calibrationFile;
      m_calibrationFile = nullptr;
    }
    m_calibrationFile = new TFile((getBaseName() + "_" + iov + ".root").c_str(), "READ");
    m_calibrationFile->cd();

    loadData();

    if (m_calibrationFile) {
      m_calibrationFile->Write();
      m_calibrationFile->Close();
      delete m_calibrationFile;
      m_calibrationFile = nullptr;
    }
    saveDir->cd();

  }

  TDirectory* saveDir = gDirectory;
  if (m_calibrationFile) {
    m_calibrationFile->Close();
    delete m_calibrationFile;
    m_calibrationFile = nullptr;
  }
  m_calibrationFile = new TFile((getBaseName() + "_" + iov + ".root").c_str(), "RECREATE");
  m_calibrationFile->cd();

  ECalibrationModuleResult calibrationResult;
  if (getState() == calibration::CalibrationModule::c_Monitoring)
    calibrationResult = Belle2::calibration::CalibrationModule::c_Success;
  else
    calibrationResult = Calibrate();

  if (calibrationResult == c_Success) {

    StoreInDataBase();

    setState(CalibrationModule::c_Monitoring);
    ECalibrationModuleMonitoringResult result = Monitor();

    if (result == c_MonitoringSuccess)
      setState(CalibrationModule::c_Done);
    else if (result == c_MonitoringIterateCalibration) {
      setState(CalibrationModule::c_Running);
      if (getNumberOfIterations() + 1 >= m_numberOfIterations)
        setState(CalibrationModule::c_Done);
    } else if (result == c_MonitoringIterateMonitor) {
      setState(CalibrationModule::c_Running);
      if (getNumberOfIterations() + 1 > m_numberOfIterations)
        setState(CalibrationModule::c_Done);
    } else
      //TODO: cannot happen
      setState(CalibrationModule::c_Failed);

  } else {

    if (calibrationResult == calibration::CalibrationModule::c_Failure) {
      B2ERROR("Calibration failed.");
      setState(CalibrationModule::c_Failed);
      storeData();

      if (m_calibrationFile) {
        m_calibrationFile->Write();
        m_calibrationFile->Close();
        delete m_calibrationFile;
        m_calibrationFile = nullptr;
      }
      saveDir->cd();
      resetData();
      return false;
    }
    if (calibrationResult == calibration::CalibrationModule::c_NotEnoughData) {
      B2WARNING("Insufficient data for calibration.");
      setState(CalibrationModule::c_Running);
      storeData();

      if (m_calibrationFile) {
        m_calibrationFile->Write();
        m_calibrationFile->Close();
        delete m_calibrationFile;
        m_calibrationFile = nullptr;
      }
      saveDir->cd();
      //TODO: really reset data?
      resetData();
      return false;
    }
    if (calibrationResult == calibration::CalibrationModule::c_NoChange) {
      B2INFO("No change in calibration.");
      setState(CalibrationModule::c_Done);
      storeData();

      if (m_calibrationFile) {
        m_calibrationFile->Write();
        m_calibrationFile->Close();
        delete m_calibrationFile;
        m_calibrationFile = nullptr;
      }
      saveDir->cd();
      resetData();
      return false;
    }
  }
  storeData();

  if (m_calibrationFile) {
    m_calibrationFile->Write();
    m_calibrationFile->Close();
    delete m_calibrationFile;
    m_calibrationFile = nullptr;
  }
  saveDir->cd();
  resetData();
  return true;
}

CalibrationModule::ECalibrationModuleState CalibrationModule::getState()
{
  if (m_currentStateIndex < 0)
    return CalibrationModule::c_Blocked;

  return std::get<1>(m_states[m_currentStateIndex]);
}

int CalibrationModule::getNumberOfIterations(const ExpRunRange& iov)
{
  for (auto entry : m_states) {
    ExpRunRange& _iov = std::get<0>(entry);
    if (_iov == iov) {
      return std::get<2>(entry);
    }
  }
  return 0;
}

vector< ExpRunRange > CalibrationModule::getCalibrationIOVs()
{
  std::vector<ExpRunRange> iovs;
  for (auto entry : m_states) {
    iovs.push_back(std::get<0>(entry));
  }
  return iovs;
}

void CalibrationModule::setState(ExpRunRange iov, CalibrationModule::ECalibrationModuleState state, int iteration)
{
  for (auto& entry : m_states) {
    ExpRunRange& _iov = std::get<0>(entry);
    if (_iov == iov) {
      std::get<1>(entry) = state;
      std::get<2>(entry) = iteration;
    }
  }
}

void CalibrationModule::setState(CalibrationModule::ECalibrationModuleState state)
{
  if (m_currentStateIndex < 0)
    B2FATAL("Cannot set state out of valid calibration IOV.");

  ECalibrationModuleState& myState = std::get<1>(m_states[m_currentStateIndex]);
  myState = state;
}

void CalibrationModule::increaseIterations()
{
  if (m_currentStateIndex < 0)
    return;

  int& iteration = std::get<2>(m_states[m_currentStateIndex]);
  iteration++;
}

int CalibrationModule::getNumberOfIterations()
{
  if (m_currentStateIndex < 0)
    return 0;

  return getNumberOfIterations(std::get<2>(m_states[m_currentStateIndex]));
}

CalibrationModule::ECalibrationModuleState CalibrationModule::getState(const ExpRunRange& iov)
{
  for (auto entry : m_states) {
    ExpRunRange& _iov = std::get<0>(entry);
    if (_iov == iov) {
      return std::get<1>(entry);
    }
  }
  return CalibrationModule::c_Blocked;
}




