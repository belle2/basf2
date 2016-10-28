/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <fcntl.h>
#include <unistd.h>

/* External headers. */
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>

/* Belle2 headers. */
#include <eklm/calibration/EKLMTimeCalibrationAlgorithm.h>
#include <eklm/dbobjects/EKLMTimeCalibration.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

static double CrystalBall(double* x, double* par)
{
  double d1, d2, t, f;
  if (*x < par[1]) {
    d1 = *x - par[1];
    f = exp(-0.5 * d1 * d1 / par[2] / par[2]);
  } else if (*x < par[4]) {
    d1 = *x - par[1];
    f = exp(-0.5 * d1 * d1 / par[3] / par[3]);
  } else {
    d1 = par[4] - par[1];
    d2 = *x - par[4];
    t = par[5] * par[3] * par[3] / d1;
    f = exp(-0.5 * d1 * d1 / par[3] / par[3]) *
        pow(t / (d2 + t), par[5]);
  }
  return par[0] * f;
}

EKLMTimeCalibrationAlgorithm::EKLMTimeCalibrationAlgorithm() :
  CalibrationAlgorithm("EKLMTimeCalibrationCollector")
{
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_maxStrip = m_GeoDat->getMaximalStripGlobalNumber();
  m_Debug = false;
}

EKLMTimeCalibrationAlgorithm::~EKLMTimeCalibrationAlgorithm()
{
}

CalibrationAlgorithm::EResult EKLMTimeCalibrationAlgorithm::calibrate()
{
  int i, n, strip;
  double s1, s2, k1, k2, dt, dl, effectiveLightSpeed;
  double* averageDist, *averageTime, *timeShift, timeShift0;
  struct Event ev;
  std::vector<struct Event>* stripEvents;
  std::vector<struct Event>::iterator it;
  EKLMTimeCalibration* calibration = new EKLMTimeCalibration();
  EKLMTimeCalibrationData calibrationData;
  bool* calibrateStrip;
  TH1F* h, *h2;
  TF1* fcn;
  TTree* t;
  TCanvas* c1 = NULL;
  if (m_Debug)
    c1 = new TCanvas();
  fcn = new TF1("fcn", CrystalBall, 0, 10, 6);
  stripEvents = new std::vector<struct Event>[m_maxStrip];
  averageDist = new double[m_maxStrip];
  averageTime = new double[m_maxStrip];
  timeShift = new double[m_maxStrip];
  calibrateStrip = new bool[m_maxStrip];
  t = &getObject<TTree>("calibration_data");
  t->SetBranchAddress("time", &ev.time);
  t->SetBranchAddress("dist", &ev.dist);
  t->SetBranchAddress("strip", &strip);
  n = t->GetEntries();
  for (i = 0; i < n; i++) {
    t->GetEntry(i);
    stripEvents[strip - 1].push_back(ev);
  }
  k1 = 0;
  k2 = 0;
  for (i = 0; i < m_maxStrip; i++) {
    n = stripEvents[i].size();
    if (n < 2) {
      B2WARNING("Not enough calibration data collected for strip "
                << i + 1 << ".");
      delete fcn;
      delete[] stripEvents;
      delete[] averageDist;
      delete[] averageTime;
      delete[] timeShift;
      delete[] calibrateStrip;
      return CalibrationAlgorithm::c_NotEnoughData;
    }
    calibrateStrip[i] = true;
    averageDist[i] = 0;
    averageTime[i] = 0;
    for (it = stripEvents[i].begin(); it != stripEvents[i].end(); ++it) {
      averageDist[i] = averageDist[i] + it->dist;
      averageTime[i] = averageTime[i] + it->time;
    }
    averageDist[i] = averageDist[i] / n;
    averageTime[i] = averageTime[i] / n;
    s1 = 0;
    s2 = 0;
    for (it = stripEvents[i].begin(); it != stripEvents[i].end(); ++it) {
      dt = it->time - averageTime[i];
      dl = it->dist - averageDist[i];
      s1 = s1 + dt * dl;
      s2 = s2 + dt * dt;
    }
    k1 = k1 + (n - 1) * s1;
    k2 = k2 + (n - 1) * s2;
  }
  h = new TH1F("h", "", 200, -10., 10.);
  h2 = new TH1F("h2", "", 200, -10., 10.);
  effectiveLightSpeed = k1 / k2;
  calibration->setEffectiveLightSpeed(effectiveLightSpeed);
  for (i = 0; i < m_maxStrip; i++) {
    if (!calibrateStrip[i])
      continue;
    timeShift[i] = averageTime[i] - averageDist[i] / effectiveLightSpeed;
    for (it = stripEvents[i].begin(); it != stripEvents[i].end(); ++it) {
      h->Fill(it->time - (timeShift[i] + it->dist / effectiveLightSpeed));
    }
  }
  fcn->SetParameter(0, h->Integral());
  fcn->SetParameter(1, h->GetMean());
  fcn->SetParameter(2, h->GetRMS());
  fcn->SetParameter(3, h->GetRMS());
  fcn->FixParameter(4, h->GetMean() + 1.0);
  fcn->FixParameter(5, 1.0);
  if (m_Debug)
    h->Fit("fcn");
  else
    h->Fit("fcn", "n");
  fcn->ReleaseParameter(4);
  fcn->ReleaseParameter(5);
  if (m_Debug)
    h->Fit("fcn");
  else
    h->Fit("fcn", "n");
  timeShift0 = fcn->GetParameter(1);
  if (m_Debug) {
    h->Draw();
    c1->Print("corrtime.eps");
  }
  for (i = 0; i < m_maxStrip; i++) {
    if (!calibrateStrip[i])
      continue;
    timeShift[i] = timeShift[i] + timeShift0;
    calibrationData.setTimeShift(timeShift[i]);
    calibration->setTimeCalibrationData(i + 1, &calibrationData);
    for (it = stripEvents[i].begin(); it != stripEvents[i].end(); ++it) {
      h2->Fill(it->time - (timeShift[i] + it->dist / effectiveLightSpeed));
    }
  }
  if (m_Debug) {
    h2->Draw();
    c1->Print("corrtime2.eps");
  }
  delete fcn;
  delete[] stripEvents;
  delete[] averageDist;
  delete[] averageTime;
  delete[] timeShift;
  delete[] calibrateStrip;
  saveCalibration(calibration, "EKLMTimeCalibration", getIovFromData());
  return CalibrationAlgorithm::c_OK;
}

void EKLMTimeCalibrationAlgorithm::setDebug()
{
  m_Debug = true;
}

