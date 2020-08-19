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



#include <TH1F.h>
#include <iostream>

using namespace std;
using namespace Belle2;

BeamSpotAlgorithm::BeamSpotAlgorithm() : CalibrationAlgorithm("BeamSpotCollector")
{
  cout << "Radek is here " << __LINE__ << endl;
  setDescription("BeamSpot calibration algorithm");
}

double getMaxGap(map<pair<int, int>, pair<double, double>> runsInfo);
map<pair<int, int>, pair<double, double>> getRunInfo();
pair<double, double> getMinMaxTime();



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

  auto tracks = getObjectPtr<TTree>("tracks");
  cout << "Tracks size " << tracks->GetEntries() << endl;

  //IP position
  auto vertexPos = TVector3(0.1, -0.2, 0.3);
  auto vertexPos2 = TVector3(0.3, -0.3, 0.3);



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


  /*
  auto payload = new BeamSpot();
  payload->setIP(vertexPos, vertexCov);
  payload->setSizeCovMatrix(vertexSize);


  auto payload2 = new BeamSpot();
  payload2->setIP(vertexPos2, vertexCov);
  payload2->setSizeCovMatrix(vertexSize);
  */


  //saveCalibration(payload);

  map<pair<int, int>, pair<double, double>> runsInfo = getRunInfo();
  double maxGap = getMaxGap(runsInfo);
  auto tMinMax =  getMinMaxTime();

  for (auto el : runsInfo) {
    pair<int, int> iov = el.first;

    auto payload = new BeamSpot();
    vertexPos(0) = maxGap;
    vertexPos(1) = tMinMax.first;
    vertexPos(2) = tMinMax.second;
    payload->setIP(vertexPos, vertexCov);
    payload->setSizeCovMatrix(vertexSize);

    TObject* obj  = static_cast<TObject*>(payload);

    EventDependency intraRun(obj);
    auto m_iov = IntervalOfValidity(iov.first, iov.second, iov.first, iov.second);
    Database::Instance().storeData("BeamSpotEventDep", &intraRun, m_iov);
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
  auto f = begin(runsInfo);
  auto s = f;
  ++s;
  if (s == end(runsInfo))
    return 0;

  double maxGap = 0;
  while (s != end(runsInfo)) {
    double t1End   = f->second.second;
    double t2Start = f->second.first;

    maxGap = max(maxGap, t2Start - t1End);
  }

  return maxGap;
}


map<pair<int, int>, pair<double, double>> BeamSpotAlgorithm::getRunInfo()
{
  auto tracks = getObjectPtr<TTree>("tracks");
  cout << "Tracks size " << tracks->GetEntries() << endl;

  double time;
  int run, exp;
  tracks->SetBranchAddress("time", &time);
  tracks->SetBranchAddress("run", &run);
  tracks->SetBranchAddress("exp", &exp);

  map<pair<int, int>, pair<double, double>> runsInfo;

  for (int i = 0; i < tracks->GetEntries(); ++i) {
    tracks->GetEntry(i);
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


pair<double, double> BeamSpotAlgorithm::getMinMaxTime()
{
  auto tracks = getObjectPtr<TTree>("tracks");
  cout << "Tracks size " << tracks->GetEntries() << endl;

  double time;
  tracks->SetBranchAddress("time", &time);

  double timeMin = 1e50, timeMax = -1e50;
  for (int i = 0; i < tracks->GetEntries(); ++i) {
    tracks->GetEntry(i);
    timeMin = min(timeMin, time);
    timeMax = max(timeMax, time);
  }
  return {timeMin, timeMax};
}






bool BeamSpotAlgorithm::isBoundaryRequired(const Calibration::ExpRun& /*currentRun*/)
{
  const double maxTimeGap = 3600; //maximal time gap in seconds

  double timeMin, timeMax;
  tie(timeMin, timeMax) = getMinMaxTime();

  if (!m_previousRunEndTime) {
    B2INFO("This is the first run encountered, let's say it is a boundary.");
    B2INFO("Initial timeMax was " << timeMax);
    m_previousRunEndTime.emplace(timeMax);
    return true;
  } else if (timeMin - m_previousRunEndTime.value() > maxTimeGap) {
    B2INFO("time gap " << timeMin - m_previousRunEndTime.value() << " is more than allowed");
    m_previousRunEndTime.emplace(timeMax);
    return true;
  } else {
    return false;
  }
}
