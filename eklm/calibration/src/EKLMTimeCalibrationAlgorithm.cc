/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>

/* Belle2 headers. */
#include <eklm/calibration/EKLMTimeCalibrationAlgorithm.h>
#include <eklm/dbobjects/EKLMTimeCalibration.h>

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
  int i, j1, j2, n, strip;
  double s[2][3], k[2][3], dt, dl, dn, effectiveLightSpeed, tau;
  double* averageDist, *averageTime, *averageSqrtN, *timeShift, timeShift0;
  struct Event ev;
  std::vector<struct Event>* stripEvents;
  std::vector<struct Event>::iterator it;
  EKLMTimeCalibration* calibration = new EKLMTimeCalibration();
  EKLMTimeCalibrationData calibrationData;
  bool* calibrateStrip;
  TH1F* h, *h2;
  TF1* fcn;
  std::shared_ptr<TTree> t;
  TCanvas* c1 = NULL;
  if (m_Debug)
    c1 = new TCanvas();
  fcn = new TF1("fcn", CrystalBall, 0, 10, 6);
  stripEvents = new std::vector<struct Event>[m_maxStrip];
  averageDist = new double[m_maxStrip];
  averageTime = new double[m_maxStrip];
  averageSqrtN = new double[m_maxStrip];
  timeShift = new double[m_maxStrip];
  calibrateStrip = new bool[m_maxStrip];
  t = getObjectPtr<TTree>("calibration_data");
  t->SetBranchAddress("time", &ev.time);
  t->SetBranchAddress("dist", &ev.dist);
  t->SetBranchAddress("npe", &ev.npe);
  t->SetBranchAddress("strip", &strip);
  n = t->GetEntries();
  for (i = 0; i < n; i++) {
    t->GetEntry(i);
    stripEvents[strip - 1].push_back(ev);
  }
  for (j1 = 0; j1 < 2; j1++) {
    for (j2 = 0; j2 < 3; j2++)
      k[j1][j2] = 0;
  }
  /*
   * Determination of the effective light speed. For each strip, the variance
   * of time is
   *
   * sigma = \frac{1}{n - 1} \sum (t_i - (t_0 + l_i / c_{eff} +
   * \tau / \sqrt{N_i})^2, (1)
   *
   * where t_i, l_i and N_i are time, distance from SiPM and number of
   * photoelectrons for a hit, respectively, t_0 is the time shift for this
   * strip, c_{eff} is the effective light speed and \tau is signal amplitude
   * dependence time constant. The parameters c_{eff} and \tau are constant
   * for all strips. The variance is approximated by
   *
   * sigma = \frac{1}{n - 1} \sum (t_i - \sum t_i  -
   * (l_i - (\sum l_i))/ c_{eff} + \tau / \sqrt{N_i})^2, (2)
   *
   * and sum of sigmas is minimized similarly to chi^2 minimization by direct
   * calculation. Partial derivatives of Eq. (2) by c_{eff} and \tau are equal
   * to 0 at the minimum.
   */
  for (i = 0; i < m_maxStrip; i++) {
    n = stripEvents[i].size();
    if (n < 2) {
      B2WARNING("Not enough calibration data collected for strip "
                << i + 1 << ".");
      delete fcn;
      delete[] stripEvents;
      delete[] averageDist;
      delete[] averageTime;
      delete[] averageSqrtN;
      delete[] timeShift;
      delete[] calibrateStrip;
      return CalibrationAlgorithm::c_NotEnoughData;
    }
    calibrateStrip[i] = true;
    averageDist[i] = 0;
    averageTime[i] = 0;
    averageSqrtN[i] = 0;
    for (it = stripEvents[i].begin(); it != stripEvents[i].end(); ++it) {
      averageDist[i] = averageDist[i] + it->dist;
      averageTime[i] = averageTime[i] + it->time;
      averageSqrtN[i] = averageSqrtN[i] + 1.0 / sqrt(it->npe);
    }
    averageDist[i] = averageDist[i] / n;
    averageTime[i] = averageTime[i] / n;
    averageSqrtN[i] = averageSqrtN[i] / n;
    for (j1 = 0; j1 < 2; j1++) {
      for (j2 = 0; j2 < 3; j2++)
        s[j1][j2] = 0;
    }
    for (it = stripEvents[i].begin(); it != stripEvents[i].end(); ++it) {
      dt = it->time - averageTime[i];
      dl = it->dist - averageDist[i];
      dn = 1.0 / sqrt(it->npe);
      s[0][0] += dl * dl;
      s[0][1] += dl * dn;
      s[0][2] += dl * dt;
      s[1][0] += dn * dl;
      s[1][1] += dn * dn;
      s[1][2] += dn * dt;
    }
    for (j1 = 0; j1 < 2; j1++) {
      for (j2 = 0; j2 < 3; j2++)
        k[j1][j2] += (n - 1) * s[j1][j2];
    }
  }
  /*
   * Now, the system of equations is
   *
   * k[0][0] / c_{eff} + k[0][1] \tau = k[0][2],
   * k[1][0] / c_{eff} + k[1][1] \tau = k[1][2].
   */
  h = new TH1F("h", "", 200, -10., 10.);
  h2 = new TH1F("h2", "", 200, -10., 10.);
  effectiveLightSpeed = (k[0][0] * k[1][1] - k[1][0] * k[0][1]) /
                        (k[0][2] * k[1][1] - k[1][2] * k[0][1]);
  tau = (k[0][0] * k[1][2] - k[1][0] * k[0][2]) /
        (k[0][0] * k[1][1] - k[1][0] * k[0][1]);
  B2INFO("Effective light speed = " << effectiveLightSpeed << " cm / ns");
  B2INFO("Amplitude time constant = " << tau << " ns");
  calibration->setEffectiveLightSpeed(effectiveLightSpeed);
  calibration->setAmplitudeTimeConstant(tau);
  for (i = 0; i < m_maxStrip; i++) {
    if (!calibrateStrip[i])
      continue;
    timeShift[i] = averageTime[i] - averageDist[i] / effectiveLightSpeed -
                   averageSqrtN[i] * tau;
    for (it = stripEvents[i].begin(); it != stripEvents[i].end(); ++it) {
      h->Fill(it->time - (timeShift[i] + it->dist / effectiveLightSpeed +
                          tau / sqrt(it->npe)));
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
      h2->Fill(it->time - (timeShift[i] + it->dist / effectiveLightSpeed +
                           tau / sqrt(it->npe)));
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
  delete[] averageSqrtN;
  delete[] timeShift;
  delete[] calibrateStrip;
  saveCalibration(calibration, "EKLMTimeCalibration");
  return CalibrationAlgorithm::c_OK;
}

void EKLMTimeCalibrationAlgorithm::setDebug()
{
  m_Debug = true;
}

