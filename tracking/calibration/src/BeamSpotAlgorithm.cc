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

//double getMaxGap(map<pair<int, int>, pair<double, double>> runsInfo);

std::map<ExpRun, std::pair<double, double>> getRunInfo(const std::vector<event>& evts);
//std::pair<double, double> getMinMaxTime(std::map<std::pair<int, int>, std::pair<double, double>> runsInfo);
pair<double, double> getMinMaxTime(const vector<event>& evts);


// Hack to put the start & end time of the calibration interval into PayLoad
void encodeStartEndTime(TMatrixDSym& vtxErrMat, pair<double, double> StartEnd, int nDiv, int i)
{
  double start = StartEnd.first + (StartEnd.second - StartEnd.first) / nDiv  * i;
  double end   = StartEnd.first + (StartEnd.second - StartEnd.first) / nDiv  * (i + 1);

  vtxErrMat(0, 1) = start / 1e20;
  vtxErrMat(0, 2) = end / 1e20;
}




TObject* getBeamSpotObj(TVector3 ipVtx, TMatrixDSym  ipVtxUnc, TMatrixDSym  sizeMat)
{
  auto payload = new BeamSpot();
  payload->setIP(ipVtx, ipVtxUnc);
  payload->setSizeCovMatrix(sizeMat);
  TObject* obj  = static_cast<TObject*>(payload); //TODO try withou static_cast
  return obj;
}


int getID(double t, const vector<double>& breaks)
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
      cout << "Too few data : " << tracks->GetEntries() <<  endl;
    return c_NotEnoughData;
  }
  cout << "Tracks size " << tracks->GetEntries() << endl;

  // Tree to vector
  vector<event> evts = getEvents(tracks.get());

  //Time range for each ExpRun
  map<ExpRun, pair<double, double>> runsInfoOrg = getRunInfo(evts);
  map<ExpRun, pair<double, double>> runsRemoved;
  auto runsInfo = filter(runsInfoOrg, 2. / 60, runsRemoved); //include only 2m or longer runs TODO include these to payloads

  if (runsInfo.size() == 0) {
    cout << "Too short run" << endl;
    return c_NotEnoughData;
  }


  Splitter splt;
  auto splits = splt.getIntervals(runsInfo, 2, 0.5, 10);

  ExpRun exprunLast(-1, -1); //last exprun
  EventDependency* intraRun = nullptr;
  //Loop over all BeamSize intervals
  vector<vector<event>> evtsVec(splits.size());

  vector<double> sVec(splits.size()), eVec(splits.size()); //vector with starts and ends of the size-blocks (time)
  vector<vector<ExpRunEvt>> breakPointsVec(splits.size()); //vector with break points positions

  vector<vector<TVector3>> ipsVec(splits.size());
  vector<vector<TMatrixDSym>> ipsUncVec(splits.size());
  vector<TMatrixDSym>  sizeMatVec(splits.size());

  //#pragma omp parallel for
  for (unsigned i = 0; i < splits.size(); ++i) {
    auto& r = splits[i];
    cout << "Radek line : " << __LINE__ << endl;
    tie(sVec[i], eVec[i]) = Splitter::getStartEnd(r);
    cout << "Start of loop " << sVec[i] << " " << eVec[i] << endl;
    cout << "Radek line : " << __LINE__ << endl;

    //if(i != 77) continue;

    auto breaks =  Splitter::getBreaks(r);

    cout << "Radek line : " << __LINE__ << endl;

    vector<int> Counts(breaks.size() + 1, 0);
    // Select events belonging to the interval
    for (const auto& ev : evts) {
      if (sVec[i] <= ev.t && ev.t < eVec[i]) {
        evtsVec.at(i).push_back(ev);
        ++Counts.at(getID(ev.t, breaks));
      }
    }


    cout << "Size comparison " << r.size() << " " << breaks.size() << endl;
    cout << "Radek line : " << __LINE__ << endl;

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
        cout << "Radek line : " << __LINE__ << endl;
        assert(r.size() == Counts.size());

        cout << "Radek line : " << __LINE__ << " " << iM << " " <<  iMin <<  endl;
        r.at(iM) = Splitter::mergeIntervals(r[iM], r[iMin]);
        cout << "Radek line : " << __LINE__ << endl;
        r.erase(r.begin() + iMin);
        cout << "Radek line : " << __LINE__ << endl;
        breaks =  Splitter::getBreaks(r);
        cout << "Radek line : " << __LINE__ << endl;
        Counts[iM] += Counts[iMin];
        Counts.erase(Counts.begin() + iMin);
      }
    }
    cout << "Radek line : " << __LINE__ << endl;

    cout << "Radek line : " << __LINE__ << endl;
    cout << "Evt size " << evtsVec.at(i).size() << endl;
    cout << "Breaks size " << breaks.size() << endl;

    breakPointsVec[i] = convertSplitPoints(evtsVec.at(i), breaks);
    cout << "I am after" << endl;

    if (breaks.size() > 0)
      cout << "HELENKA " <<   breakPointsVec[i].at(0).run << " " <<   breakPointsVec[i].at(0).evt << " " << breakPointsVec[i].size() <<
           endl;
    //assert(breakPointsVec[i].at(k - 1).run == exprun.run);


    //If too few events, let the BeamSpot pars empty
    if (evtsVec.at(i).size() < 50) {
      continue;
    }

    // Run the BeamSpot analysis
    //vector<TVector3> ipVec;
    //vector<TMatrixDSym> ipUncVec;
    //TMatrixDSym  sizeMat(3);
    sizeMatVec[i].ResizeTo(3, 3);
    cout << "Start of running BS analysis ID : " << i <<  endl;
    tie(ipsVec[i], ipsUncVec[i], sizeMatVec[i]) = runBeamSpotAnalysis(evtsVec.at(i), breaks);
    cout << "End of running BS analysis - SizeMat : " << sqrt(abs(sizeMatVec[i](0, 0))) <<   endl;
    assert(ipsVec[i].size() == r.size());
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

    //Store it
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


  //return c_NotEnoughData; //radek

  for (unsigned i = 0; i < splits.size(); ++i) {
    const auto& r = splits[i];
    // Loop over vtx calibration intervals
    for (int k = 0; k < int(r.size()); ++k) {

      for (auto I : r[k]) { //interval required to be within single run
        ExpRun exprun = I.first;

        //Encode Start+End of the payload
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
            assert(breakPointsVec[i].at(k - 1).run == exprun.run);
          } else {
            assert(i != 0);
            auto pos = getPosition(evts, sVec[i]);
            breakPoint = pos.evt;
            assert(pos.run == exprun.run);
          }
          intraRun->add(breakPoint, obj); //TODO make it correct
        }
        exprunLast = exprun;
      }
    } //end loop over vtx-intervals


  } //end loop over size-intervals

  //Store the last entry
  auto m_iov = IntervalOfValidity(exprunLast.exp, exprunLast.run, exprunLast.exp, exprunLast.run);
  Database::Instance().storeData("BeamSpot", intraRun, m_iov);

  /*


  // Get splitpoints (in time) and breakpoints (in evNumer)
  const double tDiv = 1800; //in seconds
  auto StartEnd = getMinMaxTime(evts);
  double nSplits = max(1, static_cast<int>(round((StartEnd.second - StartEnd.first) / tDiv)));

  vector<double> splitPoints;
  for (int i = 1; i < nSplits; ++i) {
    splitPoints.push_back((i * 1.) / nSplits);
  }

  vector<ExpRunEvt> breakPoints = convertSplitPoints(evts, splitPoints);


  // Run the BeamSpot analysis
  vector<TVector3> ipVec;
  vector<TMatrixDSym> ipUncVec;
  TMatrixDSym  sizeMat(3);
  tie(ipVec, ipUncVec, sizeMat) = runBeamSpotAnalysis(evts, splitPoints);


  // hack to store the time into the payload
  for (unsigned i = 0; i < ipUncVec.size(); ++i)
    encodeStartEndTime(ipUncVec.at(i), StartEnd, ipUncVec.size(), i);



  //Loop over all runs
  int payloadID = 0;
  for (auto el : runsInfo) {
    ExpRun iov = el.first;

    cout << "Radecek is here " << __LINE__ << endl;
    TObject* obj = getBeamSpotObj(ipVec.at(payloadID), ipUncVec.at(payloadID), sizeMat);
    EventDependency intraRun(obj);

    cout << "Radek inside " << iov.run << " : " << payloadID <<  endl;
    cout << "Radecek is here " << __LINE__ << endl;
    while (payloadID < int(breakPoints.size()) &&  breakPoints.at(payloadID).exp == iov.exp
           && breakPoints.at(payloadID).run == iov.run) {
      ++payloadID;

      cout << "Radek inside inside : " << payloadID << endl;
      cout << "Radecek is here " << __LINE__ << endl;
      cout <<  "Radek " << breakPoints.size() << endl;
      cout << breakPoints.at(payloadID - 1).evt << endl;

      TObject* objNow = getBeamSpotObj(ipVec.at(payloadID), ipUncVec.at(payloadID), sizeMat);
      intraRun.add(breakPoints.at(payloadID - 1).evt, objNow);

      cout << "Radecek is here " << __LINE__ << endl;
    }

    cout << "Radecek is here " << __LINE__ << endl;
    auto m_iov = IntervalOfValidity(iov.exp, iov.run, iov.exp, iov.run);
    Database::Instance().storeData("BeamSpot", &intraRun, m_iov);
    cout << "Radecek is here " << __LINE__ << endl;
  }
  */



  // probably not needed - would trigger re-doing the collection
  //if ( ... too large corrections ... ) return c_Iterate;

  return c_OK;
}

/*
double getMaxGap(map<pair<int, int>, pair<double, double>> runsInfo)
{
  cout << "Radek inside " << __LINE__ << endl;
  auto f = begin(runsInfo);
  auto s = f;
  ++s;
  if (s == end(runsInfo))
    return 0;

  double maxGap = 0;
  cout << "Radek inside " << __LINE__ << runsInfo.size() << endl;
  int i = 0;
  while (f != end(runsInfo) && s != end(runsInfo)) {
    double t1End   = f->second.second;
    double t2Start = s->second.first;

    maxGap = max(maxGap, t2Start - t1End);
    ++f;
    ++s;
    //cout << "Radek inside loop " << __LINE__ << i << endl;
    ++i;
  }

  return maxGap;
}



pair<double, double> getMinMaxTime(map<pair<int, int>, pair<double, double>> runsInfo)
{

  double timeMin = 1e50, timeMax = -1e50;

  for (auto ri : runsInfo) {
    timeMin = min(timeMin, ri.second.first);
    timeMax = max(timeMax, ri.second.second);
  }

  return {timeMin, timeMax};
}
*/






bool BeamSpotAlgorithm::isBoundaryRequired(const Calibration::ExpRun& currentRun)
{
  const double maxTimeGap = 3600; //maximal time gap in seconds

  double timeMin = 0, timeMax = 100000;
  //tie(timeMin, timeMax) = getMinMaxTime();
  if (timeMin < 0) {
    cout << "Marketka " << currentRun.second << " Nogap" << endl;
    return false; //if dummy
  }

  if (!m_previousRunEndTime) {
    B2INFO("This is the first run encountered, let's say it is a boundary.");
    B2INFO("Initial timeMax was " << timeMax);
    cout << "Marketka " << currentRun.second << " Gap" << endl;
    m_previousRunEndTime.emplace(timeMax);
    return true;
  } else if (timeMin - m_previousRunEndTime.value() > maxTimeGap) {
    B2INFO("time gap " << timeMin - m_previousRunEndTime.value() << " is more than allowed");
    cout << "Marketka " << setprecision(10) << currentRun.second << " Gap : " << timeMin << " " << timeMax << " & " <<
         m_previousRunEndTime.value()  << endl;
    m_previousRunEndTime.emplace(timeMax);
    return true;
  } else {
    cout << "Marketka " << setprecision(10) << currentRun.second << " Nogap : " << timeMin << " " << timeMax << " & " <<
         m_previousRunEndTime.value()  << endl;
    m_previousRunEndTime.emplace(timeMax);
    return false;
  }
}
