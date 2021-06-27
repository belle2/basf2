
#include <alignment/calibration/MillepedeAlgorithm.h>

#include <alignment/dataobjects/MilleData.h>
#include <alignment/dataobjects/PedeSteering.h>
#include <alignment/GlobalLabel.h>
#include <alignment/GlobalParam.h>
#include <alignment/GlobalTimeLine.h>
#include <alignment/Manager.h>
#include <alignment/PedeApplication.h>
#include <alignment/PedeResult.h>
#include <framework/database/EventDependency.h>
#include <framework/utilities/FileSystem.h>

#include <TH1F.h>
#include <TTree.h>



using namespace std;
using namespace Belle2;
using namespace alignment;

MillepedeAlgorithm::MillepedeAlgorithm() : CalibrationAlgorithm("MillepedeCollector")
{
  setDescription("Millepede calibration & alignment algorithm");
}

CalibrationAlgorithm::EResult MillepedeAlgorithm::calibrate()
{
  auto chisqHist = getObjectPtr<TH1F>("chi2_per_ndf");
  B2INFO(" Mean of Chi2 / NDF of tracks before calibration: " << chisqHist->GetMean());

  if (chisqHist->GetEntries() < m_minEntries) {
    B2INFO("Less than " << m_minEntries << " collected: " << chisqHist->GetEntries() << ". Return c_NotEnoughData.");
    return c_NotEnoughData;
  }

  // Write out binary files from tree and add to steering
  prepareMilleBinary();
  auto mille = getObjectPtr<MilleData>("mille");
  for (auto file : mille->getFiles()) {
    if (!FileSystem::fileExists(file)) {
      B2ERROR("Missing file: " << file);
      continue;
    }
    m_steering.addFile(file);
  }

  // Run calibration on steering
  m_result = m_pede.calibrate(m_steering);

  if (!m_pede.success() || !m_result.isValid()) {
    B2ERROR(m_pede.getExitMessage());
    return c_Failure;
  }

  if (m_result.getNoParameters() == 0) {
    B2INFO("No parameters to store. Failure.");
    return c_Failure;
  }

//   for (int ipar = 0; ipar < m_result.getNoParameters(); ipar++) {
//     if (!m_result.isParameterDetermined(ipar)) {
//       if (!m_result.isParameterFixed(ipar)) {
//         ++undeterminedParams;
//       }
//     }
//   }



  // This function gives you the vector of ExpRuns that were requested for this execution only
  auto expRuns = getRunList();

  // Or you can inspect all the input files to get the full RunRange
  //auto expRuns = getRunListFromAllData();

  int undeterminedParams = 0;
  double maxCorrectionPull = 0.;
  int maxCorrectionPullLabel = 0;
  int nParams = 0;
  double paramChi2 = 0.;

  if (m_events.empty()) {
    GlobalParamVector result(m_components);
    GlobalParamVector errors(m_components);
    GlobalParamVector corrections(m_components);

    GlobalCalibrationManager::initGlobalVector(result);
    GlobalCalibrationManager::initGlobalVector(errors);
    GlobalCalibrationManager::initGlobalVector(corrections);

    // <UniqueID, ElementID, ParameterId, value>
    std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>> resultTuple;

    for (auto& exprun : expRuns) {
      auto event1 = EventMetaData(1, exprun.second, exprun.first);
      result.loadFromDB(event1);
      errors.construct();
      corrections.construct();
      break;
    }

    // Construct all remaining components not loaded from DB
    // to easily create new objects not previously in DB :-)
    // TODO: remove?
    result.construct();



    // Loop over all determined parameters:
    for (int ipar = 0; ipar < m_result.getNoParameters(); ipar++) {
      if (!m_result.isParameterDetermined(ipar)) {
        if (!m_result.isParameterFixed(ipar)) {
          ++undeterminedParams;
        }
        continue;
      }

      GlobalLabel label(m_result.getParameterLabel(ipar));
      double correction = m_result.getParameterCorrection(ipar);
      double error = m_result.getParameterError(ipar);
      double pull = correction / error;

      ++nParams;
      paramChi2 += pull * pull;

      if (fabs(pull) > fabs(maxCorrectionPull)) {
        maxCorrectionPull = pull;
        maxCorrectionPullLabel = label.label();
      }

      if (m_invertSign) correction = - correction;

      result.updateGlobalParam(correction, label.getUniqueId(), label.getElementId(), label.getParameterId());
      errors.setGlobalParam(error, label.getUniqueId(), label.getElementId(), label.getParameterId());
      corrections.setGlobalParam(correction, label.getUniqueId(), label.getElementId(), label.getParameterId());

      resultTuple.push_back({label.getUniqueId(), label.getElementId(), label.getParameterId(), correction});

    }

    result.postReadFromResult(resultTuple);

    for (auto object : result.releaseObjects()) {
      saveCalibration(object);
    }
    for (auto object : errors.releaseObjects()) {
      saveCalibration(object, DataStore::objectName(object->IsA(), "") + "_ERRORS");
    }
    for (auto object : corrections.releaseObjects()) {
      saveCalibration(object, DataStore::objectName(object->IsA(), "") + "_CORRECTIONS");
    }
  } else {

    GlobalParamVector gpv(m_components);
    GlobalCalibrationManager::initGlobalVector(gpv);
    GlobalLabel label_system;

    alignment::timeline::GlobalParamTimeLine timeline(m_events, label_system, gpv);
    alignment::timeline::GlobalParamTimeLine timeline_errors(m_events, label_system, gpv);
    alignment::timeline::GlobalParamTimeLine timeline_corrections(m_events, label_system, gpv);
    timeline.loadFromDB();
    timeline_corrections.loadFromDB();
    timeline_errors.loadFromDB();

    // Loop over all determined parameters:
    for (int ipar = 0; ipar < m_result.getNoParameters(); ipar++) {
      if (!m_result.isParameterDetermined(ipar)) {
        if (!m_result.isParameterFixed(ipar)) {
          ++undeterminedParams;
        }
        continue;
      }

      GlobalLabel label(m_result.getParameterLabel(ipar));
      double correction = m_result.getParameterCorrection(ipar);
      double error = m_result.getParameterError(ipar);
      double pull = correction / error;

      ++nParams;
      paramChi2 += pull * pull;

      if (fabs(pull) > fabs(maxCorrectionPull)) {
        maxCorrectionPull = pull;
        maxCorrectionPullLabel = label.label();
      }

      if (m_invertSign) correction = - correction;

      timeline.updateGlobalParam(label, correction);
      timeline_errors.updateGlobalParam(label, error, true);
      timeline_corrections.updateGlobalParam(label, correction, true);
    }

    //TODO: if data do not cover all parts of timeline, this will lead to memory leak for
    // objects not being saved in DB -> delete them here
    auto objects = timeline.releaseObjects();
    for (auto iov_obj : objects) {
      if (iov_obj.second && !iov_obj.first.overlap(getIovFromAllData()).empty()) {
        if (auto evdep = dynamic_cast<EventDependency*>(iov_obj.second)) {
          saveCalibration(iov_obj.second, DataStore::objectName(evdep->getAnyObject()->IsA(), ""),
                          iov_obj.first.overlap(getIovFromAllData()));
        } else {
          saveCalibration(iov_obj.second, DataStore::objectName(iov_obj.second->IsA(), ""), iov_obj.first.overlap(getIovFromAllData()));
        }
      } else {
        if (iov_obj.second) delete iov_obj.second;
      }
    }

    auto objects_errors = timeline_errors.releaseObjects();
    for (auto iov_obj : objects_errors) {
      if (iov_obj.second && !iov_obj.first.overlap(getIovFromAllData()).empty()) {
        if (auto evdep = dynamic_cast<EventDependency*>(iov_obj.second)) {
          saveCalibration(iov_obj.second, DataStore::objectName(evdep->getAnyObject()->IsA(), "") + "_ERRORS",
                          iov_obj.first.overlap(getIovFromAllData()));
        } else {
          saveCalibration(iov_obj.second, DataStore::objectName(iov_obj.second->IsA(), "") + "_ERRORS",
                          iov_obj.first.overlap(getIovFromAllData()));
        }
      } else {
        if (iov_obj.second) delete iov_obj.second;
      }
    }

    auto objects_corrections = timeline_corrections.releaseObjects();
    for (auto iov_obj : objects_corrections) {
      if (iov_obj.second && !iov_obj.first.overlap(getIovFromAllData()).empty()) {
        if (auto evdep = dynamic_cast<EventDependency*>(iov_obj.second)) {
          saveCalibration(iov_obj.second, DataStore::objectName(evdep->getAnyObject()->IsA(), "") + "_CORRECTIONS",
                          iov_obj.first.overlap(getIovFromAllData()));
        } else {
          saveCalibration(iov_obj.second, DataStore::objectName(iov_obj.second->IsA(), "") + "_CORRECTIONS",
                          iov_obj.first.overlap(getIovFromAllData()));
        }
      } else {
        if (iov_obj.second) delete iov_obj.second;
      }
    }

  }

  if (undeterminedParams) {
    B2WARNING("There are " << undeterminedParams << " undetermined parameters.");
    if (!m_ignoreUndeterminedParams) {
      B2WARNING("Not enough data for calibration.");
      return c_NotEnoughData;
    }
  }

  //commit();

  if (paramChi2 / nParams > 1. || fabs(maxCorrectionPull) > 10.) {
    B2INFO("Largest correction/error is " << maxCorrectionPull << " for parameter with label " << maxCorrectionPullLabel);
    B2INFO("Parameter corrections Chi2/NDF, e.g. sum[(correction/error)^2]/#params = " << paramChi2 / nParams
           << " = " << paramChi2 << " / " << nParams << " > 1.");

    B2INFO("Requesting iteration.");
    return c_Iterate;
  }

  return c_OK;
}

void MillepedeAlgorithm::prepareMilleBinary()
{
  // Clear list of binaries in steering
  m_steering.clearFiles();

  // Temp file name
  const std::string milleFileName("gbl-data.mille");

  // For no entries, no binary file is created
  auto gblDataTree = getObjectPtr<TTree>("GblDataTree");
  if (!gblDataTree) {
    B2WARNING("No GBL data tree object in collected data.");
    return;
  } else if (!gblDataTree->GetEntries()) {
    B2WARNING("No trajectories in GBL data tree.");
    return;
  }

  // Create new mille binary
  auto milleBinary = new gbl::MilleBinary(milleFileName);

  // Containers for GBL data
  double aValue(0.);
  double aErr(0.);
  std::vector<unsigned int>* indLocal;
  std::vector<double>* derLocal;
  std::vector<int>* labGlobal;
  std::vector<double>* derGlobal;

  // Read vectors of GblData from tree branch
  std::vector<gbl::GblData>* currentGblData = new std::vector<gbl::GblData>();
  gblDataTree->SetBranchAddress("GblData", &currentGblData);

  B2INFO("Writing Millepede binary files...");
  for (unsigned int iRecord = 0; iRecord < gblDataTree->GetEntries(); ++iRecord) {
    gblDataTree->GetEntry(iRecord);

    if (!currentGblData)
      continue;

    for (gbl::GblData& theData : *currentGblData) {
      theData.getAllData(aValue, aErr, indLocal, derLocal, labGlobal,
                         derGlobal);
      milleBinary->addData(aValue, aErr, *indLocal, *derLocal, *labGlobal,
                           *derGlobal);
    }
    milleBinary->writeRecord();
  }
  // Closes the file
  delete milleBinary;
  B2INFO("Millepede binary files written.");

  // Add binary to steering
  m_steering.addFile(milleFileName);
}
