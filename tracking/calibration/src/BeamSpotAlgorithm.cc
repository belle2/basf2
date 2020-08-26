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


#include <TH1F.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

BeamSpotAlgorithm::BeamSpotAlgorithm() : CalibrationAlgorithm("BeamSpotCollector")
{
  cout << "Radek is here " << __LINE__ << endl;
  setDescription("BeamSpot calibration algorithm");
}

double getMaxGap(map<pair<int, int>, pair<double, double>> runsInfo);

std::map<std::pair<int, int>, std::pair<double, double>> getRunInfo(const std::vector<event>& evts);
std::pair<double, double> getMinMaxTime(std::map<std::pair<int, int>, std::pair<double, double>> runsInfo);
pair<double, double> getMinMaxTime(const vector<event>& evts);


void encodeStartEndTime(TMatrixDSym& vtxErrMat, pair<double, double> StartEnd, int nDiv, int i)
{
  double start = StartEnd.first + (StartEnd.second - StartEnd.first) / nDiv  * i;
  double end   = StartEnd.first + (StartEnd.second - StartEnd.first) / nDiv  * (i + 1);

  vtxErrMat(0, 1) = start / 1e20;
  vtxErrMat(0, 2) = end / 1e20;
}


CalibrationAlgorithm::EResult BeamSpotAlgorithm::calibrate()
{
  cout << "Radek is here " << __LINE__ << endl;
  /*
  int minVertices = 300;
  int nSigmacut = 6;

  auto hVertexX = getObjectPtr<TH1F>("Y4S_Vertex.X");
  hVertexX->GetXaxis()->UnZoom();

  int entries = hVertexX->GetEntries();
  if (entries < minVertices) {
    return c_NotEnoughData;
  }


  auto hVertexY = getObjectPtr<TH1F>("Y4S_Vertex.Y");
  hVertexY->GetXaxis()->UnZoom();
  auto hVertexZ = getObjectPtr<TH1F>("Y4S_Vertex.Z");
  hVertexZ->GetXaxis()->UnZoom();

  auto hVarX = getObjectPtr<TH1F>("Var.X");
  hVarX->GetXaxis()->UnZoom();
  auto hVarY = getObjectPtr<TH1F>("Var.Y");
  hVarY->GetXaxis()->UnZoom();
  auto hVarZ = getObjectPtr<TH1F>("Var.Z");
  hVarZ->GetXaxis()->UnZoom();

  Double_t medianX[1] = {0.};
  Double_t medianY[1] = {0.};
  Double_t medianZ[1] = {0.};

  Double_t q[1] = {0.5};

  hVertexX->GetQuantiles(1, medianX, q);
  hVertexY->GetQuantiles(1, medianY, q);
  hVertexZ->GetQuantiles(1, medianZ, q);

  auto vertexPos = TVector3(medianX[0], medianY[0], medianZ[0]);
  auto vertexSize = TMatrixDSym(3);
  auto vertexCov = TMatrixDSym(3);

  auto xRMS = hVertexX->GetRMS();
  auto yRMS = hVertexY->GetRMS();
  auto zRMS = hVertexZ->GetRMS();

  hVertexX->SetAxisRange(medianX[0] - nSigmacut * xRMS, medianX[0] + nSigmacut * xRMS, "X");
  hVertexY->SetAxisRange(medianY[0] - nSigmacut * yRMS, medianY[0] + nSigmacut * yRMS, "X");
  hVertexZ->SetAxisRange(medianZ[0] - nSigmacut * zRMS, medianZ[0] + nSigmacut * zRMS, "X");

  vertexCov[0][1] = vertexCov[1][0] = 0;
  vertexCov[0][2] = vertexCov[2][0] = 0;
  vertexCov[1][2] = vertexCov[2][1] = 0;
  vertexCov[0][0] = hVertexX->GetRMS() * hVertexX->GetRMS() / double(entries);
  vertexCov[1][1] = hVertexY->GetRMS() * hVertexY->GetRMS() / double(entries);
  vertexCov[2][2] = hVertexZ->GetRMS() * hVertexZ->GetRMS() / double(entries);

  vertexSize[0][1] = vertexSize[1][0] = 0;
  vertexSize[0][2] = vertexSize[2][0] = 0;
  vertexSize[1][2] = vertexSize[2][1] = 0;
  vertexSize[0][0] = hVertexX->GetRMS() * hVertexX->GetRMS() - hVarX->GetMean();
  vertexSize[1][1] = hVertexY->GetRMS() * hVertexY->GetRMS() - hVarY->GetMean();
  vertexSize[2][2] = hVertexZ->GetRMS() * hVertexZ->GetRMS() - hVarZ->GetMean();

  // if size y NAN, set it to expected size for beta_star_y = 1 mm:
  if (vertexSize[1][1] < 0) vertexSize[1][1] = 0.3 * 1e-4 * 0.3 * 1e-4;
  */


  //IP position
  auto vertexPos = TVector3(0.1, -0.2, 0.3);
  auto vertexPos2 = TVector3(0.3, -0.3, 0.3);


  //cout << "Radek is here " << __LINE__ << endl;

  //IP error
  double ipXerr2 = 0;
  double ipYerr2 = 0;
  double ipZerr2 = 0;

  auto vertexCov = TMatrixDSym(3);

  vertexCov[0][1] = vertexCov[1][0] = 0;
  vertexCov[0][2] = vertexCov[2][0] = 0;
  vertexCov[1][2] = vertexCov[2][1] = 0;
  vertexCov[0][0] = ipXerr2;
  vertexCov[1][1] = ipYerr2;
  vertexCov[2][2] = ipZerr2;

  //cout << "Radek is here " << __LINE__ << endl;
  //BeamSpot size
  double sizeX2 = pow(10e-4, 2);
  double sizeY2 = pow(1e-4, 2);
  double sizeZ2 = pow(200e-4, 2);


  auto vertexSize = TMatrixDSym(3);
  vertexSize[0][1] = vertexSize[1][0] = 0;
  vertexSize[0][2] = vertexSize[2][0] = 0;
  vertexSize[1][2] = vertexSize[2][1] = 0;
  vertexSize[0][0] = sizeX2;
  vertexSize[1][1] = sizeY2;
  vertexSize[2][2] = sizeZ2;

  //cout << "Radek is here " << __LINE__ << endl;

  /*
  auto payload = new BeamSpot();
  payload->setIP(vertexPos, vertexCov);
  payload->setSizeCovMatrix(vertexSize);


  auto payload2 = new BeamSpot();
  payload2->setIP(vertexPos2, vertexCov);
  payload2->setSizeCovMatrix(vertexSize);
  */

  //cout << "Radek is here " << __LINE__ << endl;
  auto tracks = getObjectPtr<TTree>("tracks");
  if (!tracks || tracks->GetEntries() < 15) {
    if (tracks)
      cout << "Too few data : " << tracks->GetEntries() <<  endl;
    return c_NotEnoughData;
  }
  cout << "Tracks size " << tracks->GetEntries() << endl;


  vector<event> evts = getEvents(tracks.get());


  cout << "Radek is here " << __LINE__ << endl;
  map<pair<int, int>, pair<double, double>> runsInfo = getRunInfo(evts);
  cout << "Radek is here " << __LINE__ << endl;
  double maxGap = getMaxGap(runsInfo);
  cout << "Radek is here " << __LINE__ << endl;
  auto tMinMax =  getMinMaxTime(runsInfo);
  cout << "Radecek is here " << __LINE__ << endl;


  const double tDiv = 1800; //in seconds
  auto StartEnd = getMinMaxTime(evts);
  double nSplits = max(1, static_cast<int>(round((StartEnd.second - StartEnd.first) / tDiv)));

  vector<double> splitPoints;
  for (int i = 1; i < nSplits; ++i) {
    splitPoints.push_back((i * 1.) / nSplits);
  }

  //vector<double> splitPoints = {0.7};
  vector<ExpRunEvt> breakPoints = convertSplitPoints(evts, splitPoints);
  cout << "Radecek is here " << __LINE__ << endl;


  vector<TVector3> ipVec;
  vector<TMatrixDSym> ipUncVec;
  TMatrixDSym  sizeMat(3);

  tie(ipVec, ipUncVec, sizeMat) = runBeamSpotAnalysis(evts, splitPoints);
  cout << "Radecek is here " << __LINE__ << endl;


  // hack to store the time into the payload
  for (unsigned i = 0; i < ipUncVec.size(); ++i)
    encodeStartEndTime(ipUncVec.at(i), StartEnd, ipUncVec.size(), i);

  //Loop over all runs
  int payloadID = 0;
  for (auto el : runsInfo) {
    pair<int, int> iov = el.first;

    //vertexPos(0) = maxGap;
    //vertexPos(1) = tMinMax.first;
    //vertexPos(2) = tMinMax.second;
    //vertexCov(0, 0) = iov.second;
    //vertexCov(1, 1) = el.second.first;
    //vertexCov(2, 2) = el.second.second;

    auto payload = new BeamSpot();

    payload->setIP(ipVec.at(payloadID), ipUncVec.at(payloadID));
    payload->setSizeCovMatrix(sizeMat);
    TObject* obj  = static_cast<TObject*>(payload); //TODO try withou static_cast

    cout << "Radecek is here " << __LINE__ << endl;
    EventDependency intraRun(obj);

    cout << "Radek inside " << iov.second << " : " << payloadID <<  endl;
    cout << "Radecek is here " << __LINE__ << endl;
    while (payloadID < breakPoints.size() &&  breakPoints.at(payloadID).exp == iov.first
           && breakPoints.at(payloadID).run == iov.second) {
      ++payloadID;
      auto payloadNow = new BeamSpot();
      payloadNow->setIP(ipVec.at(payloadID), ipUncVec.at(payloadID));
      payloadNow->setSizeCovMatrix(sizeMat);
      TObject* objNow  = static_cast<TObject*>(payloadNow); //TODO try withou static_cast

      cout << "Radek inside inside : " << payloadID << endl;
      cout << "Radecek is here " << __LINE__ << endl;
      cout <<  "Radek " << breakPoints.size() << endl;
      cout << breakPoints.at(payloadID - 1).evt << endl;
      intraRun.add(breakPoints.at(payloadID - 1).evt, objNow); // valid from event number 500

      cout << "Radecek is here " << __LINE__ << endl;
    }

    cout << "Radecek is here " << __LINE__ << endl;
    auto m_iov = IntervalOfValidity(iov.first, iov.second, iov.first, iov.second);
    Database::Instance().storeData("BeamSpot", &intraRun, m_iov);
    cout << "Radecek is here " << __LINE__ << endl;
  }

  /*
  TObject* obj  = static_cast<TObject*>(payload);
  TObject* obj2 = static_cast<TObject*>(payload2);

  // add objects with different validity
  EventDependency intraRun(obj);
  intraRun.add(500, obj2);   // valid from event number 500
  //intraRun.add(1000, agelObj[2]);  // valid from event number 1000

  // store under user defined name
  auto m_iov = IntervalOfValidity(0, 0, -1, -1);
  cout << "Saving BeamSpotEventDep " << endl;
  Database::Instance().storeData("BeamSpotEventDep", &intraRun, m_iov);
  auto m_iov2 = IntervalOfValidity(12, 1797, 12, 1797);
  Database::Instance().storeData("BeamSpotEventDep", &intraRun, m_iov2);
  */


  // probably not needed - would trigger re-doing the collection
  //if ( ... too large corrections ... ) return c_Iterate;

  return c_OK;
}

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


map<pair<int, int>, pair<double, double>> getRunInfo(const vector<event>& evts)
{
  /*
  //auto tracks = getObjectPtr<TTree>("tracks");
  cout << "Tracks size " << tracks->GetEntries() << endl;

  double time;
  int run, exp;
  tracks->SetBranchAddress("time", &time);
  tracks->SetBranchAddress("run", &run);
  tracks->SetBranchAddress("exp", &exp);
  */

  map<pair<int, int>, pair<double, double>> runsInfo;

  for (auto& evt : evts) {
    int exp = evt.exp;
    int run = evt.run;
    double time = evt.t;
    //tracks->GetEntry(i);
    if (runsInfo.count({exp, run})) {
      double tMin, tMax;
      tie(tMin, tMax) = runsInfo.at({exp, run});
      tMin = min(tMin, time);
      tMax = max(tMax, time);
      runsInfo.at({exp, run}) = {tMin, tMax};
    }
    else {
      runsInfo[ {exp, run}] = {time, time};
    }

  }
  return runsInfo;
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


pair<double, double> getMinMaxTime(const vector<event>& evts)
{
  double tMin = 1e40, tMax = -1e40;
  for (const auto& evt : evts) {
    double time = evt.tAbs;
    tMin = min(tMin, time);
    tMax = max(tMax, time);
  }
  return {tMin, tMax};
}





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
