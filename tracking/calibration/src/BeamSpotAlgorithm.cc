/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gaetano de Marino, Tadeas Bilka                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/calibration/BeamSpotAlgorithm.h>

#include <mdst/dbobjects/BeamSpot.h>

#include <framework/database/EventDependency.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

#include <tracking/calibration/analyzeTime.h>


#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

BeamSpotAlgorithm::BeamSpotAlgorithm() : CalibrationAlgorithm("BeamSpotCollector")
{
  setDescription("BeamSpot calibration algorithm");
}

//double getMaxGap(map<pair<int, int>, pair<double, double>> runsInfo);

std::map<std::pair<int, int>, std::pair<double, double>> getRunInfo(const std::vector<event>& evts);
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


  map<pair<int, int>, pair<double, double>> runsInfo = getRunInfo(evts);

  //Loop over all runs
  int payloadID = 0;
  for (auto el : runsInfo) {
    pair<int, int> iov = el.first;

    cout << "Radecek is here " << __LINE__ << endl;
    TObject* obj = getBeamSpotObj(ipVec.at(payloadID), ipUncVec.at(payloadID), sizeMat);
    EventDependency intraRun(obj);

    cout << "Radek inside " << iov.second << " : " << payloadID <<  endl;
    cout << "Radecek is here " << __LINE__ << endl;
    while (payloadID < int(breakPoints.size()) &&  breakPoints.at(payloadID).exp == iov.first
           && breakPoints.at(payloadID).run == iov.second) {
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
    auto m_iov = IntervalOfValidity(iov.first, iov.second, iov.first, iov.second);
    Database::Instance().storeData("BeamSpot", &intraRun, m_iov);
    cout << "Radecek is here " << __LINE__ << endl;
  }



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
