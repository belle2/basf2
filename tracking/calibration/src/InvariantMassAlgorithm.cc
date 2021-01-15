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
#include <framework/database/EventDependency.h>
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

#include <iostream>
#include <iomanip>

#include <tracking/calibration/InvariantMassAlgorithm.h>
#include <tracking/calibration/InvariantMassStandAlone.h>
#include <tracking/calibration/Splitter.h>
#include <tracking/calibration/calibTools.h>


using namespace std;
using namespace Belle2;

using Belle2::InvariantMassCalib::Event;
using Belle2::InvariantMassCalib::getEvents;
using Belle2::InvariantMassCalib::runInvariantMassAnalysis;

//Using boostVector collector for the input
InvariantMassAlgorithm::InvariantMassAlgorithm() : CalibrationAlgorithm("BoostVectorCollector")
{
  setDescription("Collision invariant mass calibration algorithm");
}


/** Create InvarinatMass object */
static TObject* getInvariantMassObj(TVector3 vMass, TMatrixDSym  vMassUnc, TMatrixDSym vMassSpread)
{
  auto payload = new CollisionInvariantMass();

  double mass    = vMass.X();
  double unc2    = vMassUnc(0, 0);
  double spread2 = vMassSpread(0, 0);

  payload->setMass(mass, sqrt(unc2), sqrt(spread2));
  TObject* obj  = static_cast<TObject*>(payload);
  return obj;
}



/* Main calibration method calling dedicated functions */
CalibrationAlgorithm::EResult InvariantMassAlgorithm::calibrate()
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
  //auto splits = splt.getIntervals(runsInfo, evts, m_tSize, m_tPos, m_gapPenalty);
  auto splits = splt.getIntervals(runsInfo, evts, m_lossFunctionOuter, m_lossFunctionInner); //  m_tSize, m_tPos, m_gapPenalty);

  //Loop over all BeamSize intervals
  vector<CalibrationData> calVec; //(splits.size());
  for (auto s : splits) {
    CalibrationData calD = runAlgorithm(evts, s, runInvariantMassAnalysis);
    calVec.push_back(calD);
  }

  // exptrapolate to low-stat intervals
  extrapolateCalibration(calVec);

  // Include removed short runs
  for (auto shortRun : runsRemoved) {
    addShortRun(calVec,  shortRun);
  }

  // Store Payloads to files
  storePayloads(evts, calVec, "CollisionInvariantMass", getInvariantMassObj);

  return c_OK;
}

