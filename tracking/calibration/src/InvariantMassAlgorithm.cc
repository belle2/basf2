/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dbobjects/CollisionInvariantMass.h>
#include <tracking/calibration/InvariantMassAlgorithm.h>
#include <tracking/calibration/InvariantMassMuMuStandAlone.h>
#include <tracking/calibration/InvariantMassBhadStandAlone.h>
#include <tracking/calibration/calibTools.h>

#include <Eigen/Dense>
#include <iostream>
using std::cout;
using std::endl;

using Eigen::VectorXd;
using Eigen::MatrixXd;

using namespace Belle2;


//Using boostVector collector for the input
InvariantMassAlgorithm::InvariantMassAlgorithm() : CalibrationAlgorithm("eCmsCollector")
{
  setDescription("Collision invariant mass calibration algorithm");
}


/** Create InvarinatMass object */
static TObject* getInvariantMassObj(VectorXd vMass, MatrixXd  vMassUnc, MatrixXd vMassSpread)
{
  auto payload = new CollisionInvariantMass();

  double mass    = vMass(0);
  double unc     = vMassUnc(0, 0);
  double spread  = vMassSpread(0, 0);

  payload->setMass(mass, unc, spread);
  TObject* obj  = static_cast<TObject*>(payload);
  return obj;
}


template<typename Fun>
std::vector<CalibrationData>  runMuMuCalibration(const std::vector<Belle2::InvariantMassMuMuCalib::Event>& evts,  Fun calibAnalysis,
                                                 TString m_lossFunctionOuter, TString m_lossFunctionInner)
{
  //Time range for each ExpRun
  std::map<ExpRun, std::pair<double, double>> runsInfoOrg = getRunInfo(evts);
  std::map<ExpRun, std::pair<double, double>> runsRemoved; //map with time intervals of very short runs
  auto runsInfo = filter(runsInfoOrg, 2. / 60, runsRemoved); //include only runs longer than 2mins

  // If nothing remains
  if (runsInfo.size() == 0) {
    B2WARNING("Too short run");
    return  {};
  }

  // Get intervals based on the input loss functions
  Splitter splt;
  auto splits = splt.getIntervals(runsInfo, evts, m_lossFunctionOuter, m_lossFunctionInner, 100 /*maximal lenght of the run */);

  //Loop over all calibration intervals
  std::vector<CalibrationData> calVec;
  for (auto s : splits) {
    CalibrationData calD = runAlgorithm(evts, s, calibAnalysis); // run the calibration over the interval s
    calVec.push_back(calD);
  }

  // extrapolate results to the low-stat intervals
  extrapolateCalibration(calVec);

  // Include removed short runs
  for (auto shortRun : runsRemoved) {
    addShortRun(calVec,  shortRun);
  }

  return calVec;
}


std::vector<CalibrationData> addSpreadAndOffset(std::vector<CalibrationData> mumuCalResults, double spread, double offset)
{
  std::vector<CalibrationData> mumuCalResultsNew = mumuCalResults;

  for (unsigned i = 0; i < mumuCalResults.size(); ++i) {
    auto& calibNew = mumuCalResultsNew[i];

    for (unsigned j = 0; j < calibNew.subIntervals.size(); ++j) {
      calibNew.pars.cnt[j](0)      += offset;
      calibNew.pars.spreadMat(0, 0) = spread;
    }
  }
  return mumuCalResultsNew;
}





/* Main calibration method calling dedicated functions */
CalibrationAlgorithm::EResult InvariantMassAlgorithm::calibrate()
{
  std::vector<std::string> files = getVecInputFileNames();

  std::vector<std::string> filesHad, filesMuMu;
  for (auto f : files) {
    if (f.find("/mumu/") != std::string::npos)
      filesMuMu.push_back(f);
    else if (f.find("/hadB/") != std::string::npos)
      filesHad.push_back(f);
    else {
      B2ASSERT("Data directory is missing", false);
    }
  }

  for (auto r : filesMuMu)
    B2INFO("MuMuFile name " << r);
  for (auto r : filesHad)
    B2INFO("HadFile name " << r);

  ///////////////////////////////////////
  // Read and process mumu data
  ///////////////////////////////////////

  clearCalibrationData();
  setPrefix("BoostVectorCollector");
  setInputFileNames(filesMuMu);
  fillRunToInputFilesMap();
  TTree* tracksMuMu = getObjectPtr<TTree>("events").get();

  // Check that there are at least some mumu data
  if (!tracksMuMu || tracksMuMu->GetEntries() < 15) {
    if (tracksMuMu)
      B2WARNING("Too few data : " << tracksMuMu->GetEntries());
    return CalibrationAlgorithm::EResult::c_NotEnoughData;
  }
  B2INFO("Number of mumu events: " << tracksMuMu->GetEntries());
  const auto evtsMuMu = InvariantMassMuMuCalib::getEvents(tracksMuMu);


  // Run the mumuCalibration
  const std::vector<CalibrationData> mumuCalResults = runMuMuCalibration(evtsMuMu,
                                                      InvariantMassMuMuCalib::runMuMuInvariantMassAnalysis,
                                                      m_lossFunctionOuter, m_lossFunctionInner);
  // For running only the mumu calib

  if (!m_runHadB) {
    std::vector<CalibrationData> mumuCalResultsCorr = addSpreadAndOffset(mumuCalResults, m_eCMSmumuSpread, m_eCMSmumuShift);
    storePayloadsNoIntraRun(mumuCalResultsCorr, "CollisionInvariantMass", getInvariantMassObj);
    return CalibrationAlgorithm::EResult::c_OK;
  }

  ///////////////////////////////////////
  // Read Bhad data
  ///////////////////////////////////////

  clearCalibrationData();
  setPrefix("eCmsCollector");
  setInputFileNames(filesHad);
  TTree* tracksHad = getObjectPtr<TTree>("events").get();
  //tracksHad->Print();

  // Check that there are at least some mumu data
  if (!tracksHad || tracksHad->GetEntries() < 15) {
    if (tracksHad)
      B2WARNING("Too few data : " << tracksHad->GetEntries());
    return CalibrationAlgorithm::EResult::c_NotEnoughData;
  }
  B2INFO("Number of Bhad events: " << tracksHad->GetEntries());
  const auto evtsHad = InvariantMassBhadCalib::getEvents(tracksHad);

  auto combCalResults = mumuCalResults;
  //Update the calibration with Bhad data
  for (unsigned i = 0; i < mumuCalResults.size(); ++i) {
    const auto& calibMuMu = mumuCalResults[i];
    auto& calibComb = combCalResults[i];

    std::vector<std::pair<double, double>> limits, mumuVals;

    for (unsigned j = 0; j < calibMuMu.subIntervals.size(); ++j) {
      double s = calibMuMu.subIntervals[j].begin()->second.first;
      double e = calibMuMu.subIntervals[j].rbegin()->second.second;
      limits.push_back({s, e});

      double val = calibMuMu.pars.cnt[j](0);
      double unc = calibMuMu.pars.cntUnc[j](0, 0);
      mumuVals.push_back({val, unc});
    }

    auto res = InvariantMassBhadCalib::doBhadFit(evtsHad, limits, mumuVals);

    //update
    for (unsigned j = 0; j < calibMuMu.subIntervals.size(); ++j) {
      calibComb.pars.cnt[j](0)      = res[j][0];
      calibComb.pars.cntUnc[j](0, 0) = res[j][1];
      calibComb.pars.spreadMat(0, 0) = res[j][2];
    }
  }


  storePayloadsNoIntraRun(combCalResults, "CollisionInvariantMass", getInvariantMassObj);

  return CalibrationAlgorithm::EResult::c_OK;
}
