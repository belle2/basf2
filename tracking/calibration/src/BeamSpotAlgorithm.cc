/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dbobjects/BeamSpot.h>
#include <framework/database/EventDependency.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

#include <iostream>
#include <iomanip>

#include <tracking/calibration/BeamSpotAlgorithm.h>
#include <tracking/calibration/standAloneBSalgo.h>
#include <tracking/calibration/Splitter.h>


using namespace std;
using namespace Belle2;

BeamSpotAlgorithm::BeamSpotAlgorithm() : CalibrationAlgorithm("BeamSpotCollector")
{
  setDescription("BeamSpot calibration algorithm");
}


/** Create BS object */
static TObject* getBeamSpotObj(TVector3 ipVtx, TMatrixDSym  ipVtxUnc, TMatrixDSym  sizeMat)
{
  auto payload = new BeamSpot();
  payload->setIP(ipVtx, ipVtxUnc);
  payload->setSizeCovMatrix(sizeMat);
  TObject* obj  = static_cast<TObject*>(payload);
  return obj;
}

/** get id of the time point t */
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

/** BeamSpot data in the BS-size interval + relevant intervals */
struct CalibrationData {
  /** vector of the BS-position intervals */
  std::vector<std::map<ExpRun, std::pair<double, double>>> bsPosIntervals;

  vector<ExpRunEvt> breakPoints; ///< vector with break points positions

  vector<TVector3> ips; ///< vector of BS positions for each BS-pos calib. interval
  vector<TMatrixDSym> ipsUnc; ///< vector of BS positions uncs. for each BS-pos calib. interval
  TMatrixDSym  sizeMat; ///< BS size CovMatrix

  bool isCalibrated = false; ///< true if calibration run was successful

};


/** run BeamSpot algorithm for single BS-size interval */
CalibrationData runAlgorithm(const vector<Event>& evts, std::vector<std::map<ExpRun, std::pair<double, double>>> range)
{
  CalibrationData calD;
  auto& r = range;
  double rStart, rEnd;
  //tie(calD.s, calD.e) = Splitter::getStartEnd(r);
  tie(rStart, rEnd) = Splitter::getStartEnd(r);
  B2INFO("Start of loop startTime endTime : " << rStart << " " << rEnd);

  auto breaks =  Splitter::getBreaks(r);

  vector<Event> evtsNow;

  vector<int> Counts(breaks.size() + 1, 0);
  // Select events belonging to the interval
  for (const auto& ev : evts) {
    if (rStart <= ev.t && ev.t < rEnd) {
      evtsNow.push_back(ev);
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

  B2INFO("#events "  << " : " << evtsNow.size());
  B2INFO("Breaks size " << " : " << breaks.size());

  calD.breakPoints = convertSplitPoints(evtsNow, breaks);

  calD.bsPosIntervals = r;

  if (breaks.size() > 0)
    B2INFO("StartOfCalibInterval (run,evtNo,vtxIntervalsSize) " <<   calD.breakPoints.at(0).run << " " <<
           calD.breakPoints.at(0).evt << " " << calD.breakPoints.size());


  //If too few events, let the BeamSpot pars empty
  //Will be filled with the closest neighbor at the next stage
  if (evtsNow.size() < 50) {
    return calD;
  }

  // Run the BeamSpot analysis
  calD.sizeMat.ResizeTo(3, 3);
  B2INFO("Start of running BS analysis ID : ");
  tie(calD.ips, calD.ipsUnc, calD.sizeMat) = runBeamSpotAnalysis(evtsNow, breaks);
  B2INFO("End of running BS analysis - SizeMatZ : " << sqrt(abs(calD.sizeMat(0, 0))));
  B2ASSERT("All intervals have IP calibration", calD.ips.size() == r.size());
  B2ASSERT("All intervals have IPunc calibration", calD.ipsUnc.size() == r.size());

  calD.isCalibrated = true;

  return calD;
}


/* Extrapolate calibration to intervals where it failed */
void extrapolateCalibration(vector<CalibrationData>&  calVec)
{
  //put closest neighbor, where stat was low or algo failed
  for (unsigned i = 0; i < calVec.size(); ++i) {
    if (calVec[i].ips.size() != 0) continue;
    const auto& r =  calVec[i].bsPosIntervals;
    double Start, End;
    tie(Start, End) = Splitter::getStartEnd(r);

    TVector3 ipNow;
    TMatrixDSym ipeNow(3);
    TMatrixDSym  sizeMatNow(3);

    double distMin = 1e20;
    //Find the closest calibrated interval
    for (unsigned j = 0; j < calVec.size(); ++j) {
      if (calVec[j].isCalibrated == false) continue; //skip not-calibrated intervals
      const auto& rJ = calVec[j].bsPosIntervals;
      for (unsigned jj = 0; jj < rJ.size(); ++jj) {
        const auto& rNow = rJ[jj];
        double s = rNow.begin()->second.first;
        double e = rNow.rbegin()->second.second;

        double dist1 = (s - End >= 0) ? (s - End) : 1e20;
        double dist2 = (Start - e >= 0) ? (Start - e) : 1e20;
        double dist = min(dist1, dist2);

        if (dist < distMin) {
          ipNow = calVec[j].ips.at(jj);
          ipeNow = calVec[j].ipsUnc.at(jj);
          sizeMatNow = calVec[j].sizeMat;
          distMin = dist;
        }
      }
    }

    //Store it to vectors
    calVec[i].ips.resize(r.size());
    calVec[i].ipsUnc.resize(r.size());
    for (unsigned ii = 0; ii < r.size(); ++ii) {
      calVec[i].ips.at(ii) = ipNow;
      calVec[i].ipsUnc.at(ii).ResizeTo(3, 3);
      calVec[i].ipsUnc.at(ii) = ipeNow;
    }
    calVec[i].sizeMat.ResizeTo(3, 3);
    calVec[i].sizeMat = sizeMatNow;
  }

}


/** Store BeamSpot payloads to files */
void storePayloads(const vector<Event>& evts, vector<CalibrationData>&  calVec)
{

  // Loop to store payloads
  ExpRun exprunLast(-1, -1); //last exprun
  EventDependency* intraRun = nullptr;

  for (unsigned i = 0; i < calVec.size(); ++i) {
    const auto& r = calVec[i].bsPosIntervals; //   splits[i];
    // Loop over vtx calibration intervals
    for (int k = 0; k < int(r.size()); ++k) {

      for (auto I : r[k]) { //interval required to be within single run
        ExpRun exprun = I.first;

        //Encode Start+End time of the payload
        calVec[i].ipsUnc.at(k)(0, 1) = I.second.first / 1e20;
        calVec[i].ipsUnc.at(k)(0, 2) = I.second.second / 1e20;
        TObject* obj = getBeamSpotObj(calVec[i].ips.at(k), calVec[i].ipsUnc.at(k), calVec[i].sizeMat);
        if (exprun != exprunLast) { //if new run
          if (intraRun) { //if not first -> store
            auto m_iov = IntervalOfValidity(exprunLast.exp, exprunLast.run, exprunLast.exp, exprunLast.run);
            Database::Instance().storeData("BeamSpot", intraRun, m_iov);
          }

          intraRun = new EventDependency(obj);
        } else {
          int breakPoint;
          if (k - 1 >= 0) {
            breakPoint = calVec[i].breakPoints.at(k - 1).evt;
            B2ASSERT("Payload saving consistency", calVec[i].breakPoints.at(k - 1).run == exprun.run);
          } else {
            B2ASSERT("Payload saving consistency", i != 0);
            double rStart, rEnd;
            tie(rStart, rEnd) = Splitter::getStartEnd(r);
            auto pos = getPosition(evts, rStart);
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
}

/** Extrapolate calibration to the shortRun which was filtered before */
void addShortRun(vector<CalibrationData>& calVec,  pair<ExpRun, pair<double, double>> shortRun)
{
  double shortStart = shortRun.second.first;
  double shortEnd   = shortRun.second.second;

  double distMin = 1e20;
  int iMin = -1, jMin = -1;

  for (unsigned i = 0; i < calVec.size(); ++i) {
    if (calVec[i].isCalibrated == false)
      continue;
    for (unsigned j = 0; j < calVec[i].bsPosIntervals.size(); ++j) {
      for (auto I : calVec[i].bsPosIntervals[j]) {
        double s = I.second.first;
        double e = I.second.second;

        double dist1 = (s - shortEnd >= 0)   ? (s - shortEnd) : 1e20;
        double dist2 = (shortStart - e >= 0) ? (shortStart - e) : 1e20;
        double dist = min(dist1, dist2);

        if (dist < distMin) {
          distMin = dist;
          iMin = i;
          jMin = j;
        }
      }
    }
  }

  B2ASSERT("Must be found", iMin != -1 && jMin != -1);
  calVec[iMin].bsPosIntervals[jMin].insert(shortRun);
}

/* Main calibration method calling dedicated functions */
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
    CalibrationData calD = runAlgorithm(evts, s);
    calVec.push_back(calD);
  }

  // exptrapolate to low-stat intervals
  extrapolateCalibration(calVec);

  // Include removed short runs
  for (auto shortRun : runsRemoved) {
    addShortRun(calVec,  shortRun);
  }

  // Store Payloads to files
  storePayloads(evts, calVec);

  return c_OK;
}



