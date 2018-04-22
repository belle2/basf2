#include <alignment/calibration/MillepedeAlgorithm.h>

#include <TH1F.h>
#include <TTree.h>

#include <alignment/dataobjects/MilleData.h>
#include <alignment/PedeApplication.h>
#include <alignment/PedeResult.h>
#include <alignment/dataobjects/PedeSteering.h>

#include <alignment/GlobalLabel.h>
#include <alignment/GlobalParam.h>

#include <alignment/Manager.h>

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

  // Write out binary files from tree and add to steering
  prepareMilleBinary();
  auto mille = getObjectPtr<MilleData>("mille");
  for (auto file : mille->getFiles())
    m_steering.addFile(file);

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

  GlobalParamVector result(m_components);
  GlobalCalibrationManager::initGlobalVector(result);

  // <UniqueID, ElementID, ParameterId, value>
  std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>> resultTuple;

  // This function gives you the vector of ExpRuns that were requested for this execution only
  //auto expRuns = getRunList();

  // Or you can inspect all the input files to get the full RunRange
  auto expRuns = getRunListFromAllData();

  for (auto& exprun : expRuns) {
    auto event1 = EventMetaData(1, exprun.second, exprun.first);
    result.loadFromDB(event1);
    break;
  }

  // Construct all remaining components not loaded from DB
  // to easily create new objects not previously in DB :-)
  // TODO: remove?
  result.construct();

  int undeterminedParams = 0;
  double maxCorrectionPull = 0.;
  int maxCorrectionPullLabel = 0;
  int nParams = 0;
  double paramChi2 = 0.;

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
    resultTuple.push_back({label.getUniqueId(), label.getElementId(), label.getParameterId(), correction});

  }

  result.postReadFromResult(resultTuple);

  for (auto object : result.releaseObjects()) {
    saveCalibration(object);
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
