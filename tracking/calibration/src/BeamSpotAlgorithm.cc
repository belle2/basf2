/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/calibration/BeamSpotAlgorithm.h>

#include <mdst/dbobjects/BeamSpot.h>

#include <framework/database/EventDependency.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

#include "standAloneBSalgo.h"

#include "Splitter.h"

#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

BeamSpotAlgorithm::BeamSpotAlgorithm() : CalibrationAlgorithm("BeamSpotCollector")
{
  setDescription("BeamSpot calibration algorithm");
}



static TObject* getBeamSpotObj(TVector3 ipVtx, TMatrixDSym  ipVtxUnc, TMatrixDSym  sizeMat)
{
  auto payload = new BeamSpot();
  payload->setIP(ipVtx, ipVtxUnc);
  payload->setSizeCovMatrix(sizeMat);
  TObject* obj  = static_cast<TObject*>(payload);
  return obj;
}

//get id of the
static int getID(const vector<double>& breaks, double t)
{
  for (int i = 0; i < int(breaks.size()) + 1; ++i) {
    double s = (i == 0)             ? 0 : breaks[i - 1];
    double e = (i == int(breaks.size())) ? 1e20 : breaks[i];
    if (s  <= t   &&  t < e)
      return i;
  }
  return -1;
}



CalibrationAlgorithm::EResult BeamSpotAlgorithm::calibrate()
{
  auto tracks = getObjectPtr<TTree>("tracks");
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
  auto runsInfo = filter(runsInfoOrg, 2. / 60, runsRemoved); //include only 2m or longer runs TODO include these to payloads

  if (runsInfo.size() == 0) {
    B2WARNING("Too short run");
    return c_NotEnoughData;
  }

  // Get intervals
  Splitter splt;
  auto splits = splt.getIntervals(runsInfo, 2, 0.5, 10);

  //Loop over all BeamSize intervals
  vector<vector<Event>> evtsVec(splits.size());

  vector<double> sVec(splits.size()), eVec(splits.size()); //vector with starts and ends of the size-blocks (time)
  vector<vector<ExpRunEvt>> breakPointsVec(splits.size()); //vector with break points positions

  vector<vector<TVector3>> ipsVec(splits.size());
  vector<vector<TMatrixDSym>> ipsUncVec(splits.size());
  vector<TMatrixDSym>  sizeMatVec(splits.size());

  //TODO paralelize this loop over calibration intervals
  //#pragma omp parallel for
  for (unsigned i = 0; i < splits.size(); ++i) {
    auto& r = splits[i];
    tie(sVec[i], eVec[i]) = Splitter::getStartEnd(r);
    B2INFO("Start of loop startTime endTime : " << sVec[i] << " " << eVec[i]);

    auto breaks =  Splitter::getBreaks(r);

    vector<int> Counts(breaks.size() + 1, 0);
    // Select events belonging to the interval
    for (const auto& ev : evts) {
      if (sVec[i] <= ev.t && ev.t < eVec[i]) {
        evtsVec.at(i).push_back(ev);
        ++Counts.at(getID(breaks, ev.t));
      }
    }

    B2ASSERT("Number of intervals vs number of breakPoints", r.size()  == breaks.size() + 1);

    //Merge smallest interval if with low stat (try it 10times)
    for (int k = 0; k < 10; ++k)  {
      int iMin = min_element(Counts.begin(), Counts.end()) - Counts.begin();
      if (Counts.size() >= 2 &&  Counts[iMin] < 50) { //merge with neighbor if possible
        auto iM = -1;
        if (iMin == 0)
          iM = iMin + 1;
        else if (iMin == int(Counts.size()) - 1)
          iM = iMin - 1;
        else {
          if (Counts[iMin + 1] < Counts[iMin - 1])
            iM = iMin + 1;
          else
            iM = iMin - 1;
        }
        B2ASSERT("Number of intervals equal to size of counters", r.size() == Counts.size());

        r.at(iM) = Splitter::mergeIntervals(r[iM], r[iMin]);
        r.erase(r.begin() + iMin);
        breaks =  Splitter::getBreaks(r);
        Counts[iM] += Counts[iMin];
        Counts.erase(Counts.begin() + iMin);
      }
    }

    B2INFO("#events " << i << " : " << evtsVec.at(i).size());
    B2INFO("Breaks size " << i << " : " << breaks.size());

    breakPointsVec[i] = convertSplitPoints(evtsVec.at(i), breaks);

    if (breaks.size() > 0)
      B2INFO("StartOfCalibInterval (run,evtNo,vtxIntervalsSize) " <<   breakPointsVec[i].at(0).run << " " <<   breakPointsVec[i].at(
               0).evt << " " << breakPointsVec[i].size());


    //If too few events, let the BeamSpot pars empty
    if (evtsVec.at(i).size() < 50) {
      continue;
    }

    // Run the BeamSpot analysis
    sizeMatVec[i].ResizeTo(3, 3);
    B2INFO("Start of running BS analysis ID : " << i);
    tie(ipsVec[i], ipsUncVec[i], sizeMatVec[i]) = runBeamSpotAnalysis(evtsVec.at(i), breaks);
    B2INFO("End of running BS analysis - SizeMatZ : " << sqrt(abs(sizeMatVec[i](0, 0))));
    B2ASSERT("All intervals have IP calibration", ipsVec[i].size() == r.size());
    B2ASSERT("All intervals have IPunc calibration", ipsUncVec[i].size() == r.size());
  }



  //put closest neighbor, where stat was low or algo failed
  for (unsigned i = 0; i < splits.size(); ++i) {
    if (ipsVec[i].size() != 0) continue;
    const auto& r = splits[i];
    double Start, End;
    tie(Start, End) = Splitter::getStartEnd(r);

    TVector3 ipNow;
    TMatrixDSym ipeNow(3);
    TMatrixDSym  sizeMatNow(3);

    double distMin = 1e20;
    //Find the closest non-dummy interval
    for (unsigned j = 0; j < splits.size(); ++j) {
      if (ipsVec[j].size() == 0) continue; //skip empty
      for (unsigned jj = 0; jj < splits[j].size(); ++jj) {
        const auto& rNow = splits[j][jj];
        double s = rNow.begin()->second.first;
        double e = rNow.rbegin()->second.second;

        double dist1 = (s - End >= 0) ? (s - End) : 1e20;
        double dist2 = (Start - e >= 0) ? (Start - e) : 1e20;
        double dist = min(dist1, dist2);

        if (dist < distMin) {
          ipNow = ipsVec[j].at(jj);
          ipeNow = ipsUncVec[j].at(jj);
          sizeMatNow = sizeMatVec[j];
          distMin = dist;
        }
      }
    }

    //Store it to vectors
    ipsVec[i].resize(r.size());
    ipsUncVec[i].resize(r.size());
    for (unsigned ii = 0; ii < r.size(); ++ii) {
      ipsVec[i][ii] = ipNow;
      ipsUncVec[i][ii].ResizeTo(3, 3);
      ipsUncVec[i][ii] = ipeNow;
    }
    sizeMatVec[i].ResizeTo(3, 3);
    sizeMatVec[i] = sizeMatNow;
  }


  // Loop to store payloads
  ExpRun exprunLast(-1, -1); //last exprun
  EventDependency* intraRun = nullptr;

  for (unsigned i = 0; i < splits.size(); ++i) {
    const auto& r = splits[i];
    // Loop over vtx calibration intervals
    for (int k = 0; k < int(r.size()); ++k) {

      for (auto I : r[k]) { //interval required to be within single run
        ExpRun exprun = I.first;

        //Encode Start+End time of the payload
        ipsUncVec[i].at(k)(0, 1) = I.second.first / 1e20;
        ipsUncVec[i].at(k)(0, 2) = I.second.second / 1e20;
        TObject* obj = getBeamSpotObj(ipsVec[i].at(k), ipsUncVec[i].at(k), sizeMatVec[i]);
        if (exprun != exprunLast) { //if new run
          if (intraRun) { //if not first -> store
            auto m_iov = IntervalOfValidity(exprunLast.exp, exprunLast.run, exprunLast.exp, exprunLast.run);
            Database::Instance().storeData("BeamSpot", intraRun, m_iov);
          }

          intraRun = new EventDependency(obj);
        } else {
          int breakPoint;
          if (k - 1 >= 0) {
            breakPoint = breakPointsVec[i].at(k - 1).evt;
            B2ASSERT("Payload saving consistency", breakPointsVec[i].at(k - 1).run == exprun.run);
          } else {
            B2ASSERT("Payload saving consistency", i != 0);
            auto pos = getPosition(evts, sVec[i]);
            breakPoint = pos.evt;
            B2ASSERT("Payload saving consistency", pos.run == exprun.run);
          }
          intraRun->add(breakPoint, obj);
        }
        exprunLast = exprun;
      }
    } //end loop over vtx-intervals


  } //end loop over size-intervals

  //Store the last entry
  auto m_iov = IntervalOfValidity(exprunLast.exp, exprunLast.run, exprunLast.exp, exprunLast.run);
  Database::Instance().storeData("BeamSpot", intraRun, m_iov);


  return c_OK;
}



