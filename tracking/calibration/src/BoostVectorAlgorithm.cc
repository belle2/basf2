/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dbobjects/CollisionBoostVector.h>
#include <framework/database/EventDependency.h>
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

#include <iostream>
#include <iomanip>

#include <tracking/calibration/BoostVectorAlgorithm.h>
#include <tracking/calibration/BoostVectorStandAlone.h>
#include <tracking/calibration/Splitter.h>
#include <tracking/calibration/calibTools.h>


using namespace std;
using namespace Belle2;

using Belle2::BoostVectorCalib::Event;
using Belle2::BoostVectorCalib::getEvents;
using Belle2::BoostVectorCalib::runBoostVectorAnalysis;


BoostVectorAlgorithm::BoostVectorAlgorithm() : CalibrationAlgorithm("BoostVectorCollector")
{
  setDescription("BoostVector calibration algorithm");
}


/** Create BoostVector object */
static TObject* getBoostVectorObj(TVector3 vBoost, TMatrixDSym  vBoostUnc, TMatrixDSym /*vBoostSpread*/)
{
  auto payload = new CollisionBoostVector();
  payload->setBoost(vBoost, vBoostUnc);
  TObject* obj  = static_cast<TObject*>(payload);
  return obj;
}



/* Main calibration method calling dedicated functions */
CalibrationAlgorithm::EResult BoostVectorAlgorithm::calibrate()
{
  auto tracks = getObjectPtr<TTree>("events");
  if (!tracks || tracks->GetEntries() < 15) {
    if (tracks)
      B2WARNING("Too few data : " << tracks->GetEntries());
    return c_NotEnoughData;
  }
  B2INFO("Number of tracks: " << tracks->GetEntries());

  // Tree to vector
  vector<Event> evts = getEvents(tracks.get());

  //Time range for each ExpRun
  map<ExpRun, pair<double, double>> runsInfoOrg = getRunInfo(evts);
  map<ExpRun, pair<double, double>> runsRemoved;
  auto runsInfo = filter(runsInfoOrg, 2. / 60, runsRemoved); //include only runs longer than 2mins

  if (runsInfo.size() == 0) {
    B2WARNING("Too short run");
    return c_NotEnoughData;
  }

  // Get intervals
  Splitter splt;
  auto splits = splt.getIntervals(runsInfo, m_tSize, m_tPos, m_gapPenalty);

  //Loop over all BeamSize intervals
  vector<CalibrationData> calVec; //(splits.size());
  for (auto s : splits) {
    CalibrationData calD = runAlgorithm(evts, s, runBoostVectorAnalysis);
    calVec.push_back(calD);
  }

  // exptrapolate to low-stat intervals
  extrapolateCalibration(calVec);

  // Include removed short runs
  for (auto shortRun : runsRemoved) {
    addShortRun(calVec,  shortRun);
  }

  // Store Payloads to files
  storePayloads(evts, calVec, "BoostVector", getBoostVectorObj);

  return c_OK;
}

