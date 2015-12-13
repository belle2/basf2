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
#include <framework/core/ModuleManager.h>
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

static double Pol1(double* x, double* par)
{
  return *x * par[0] + par[1];
}

EKLMTimeCalibrationAlgorithm::EKLMTimeCalibrationAlgorithm() :
  CalibrationAlgorithm("EKLMTimeCalibrationCollector")
{
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_nStripDifferent = m_GeoDat->getNStripsDifferentLength();
}

EKLMTimeCalibrationAlgorithm::~EKLMTimeCalibrationAlgorithm()
{
}

CalibrationAlgorithm::EResult EKLMTimeCalibrationAlgorithm::calibrate()
{
  TFile* fOut = new TFile("TimeCalibration.root", "recreate");
  TTree* tOut = new TTree("t_calibration", "");
  const int maxbins = 40;
  const int nbins = 25;
  int i, j, n2, bin;
  float mint[maxbins], time, dist, *timeArray, *distArray;
  float p0, p1, maxval;
  char str[128];
  double len;
  TH1F* h[50], *h2[maxbins], *h3;
  TF1* fcn, *fcn2;
  TTree* t2;
  TCanvas* c1 = new TCanvas();
  tOut->Branch("p0", &p0, "p0/F");
  tOut->Branch("p1", &p1, "p1/F");
  fcn = new TF1("fcn", CrystalBall, 0, 10, 6);
  fcn2 = new TF1("fcn2", Pol1, 0, 10, 2);
  for (i = 0; i < m_nStripDifferent; i++) {
    len = m_GeoDat->getStripLength(m_GeoDat->getStripLengthIndex(i) + 1);
    for (j = 0; j < nbins; j++) {
      mint[j] = 200;
    }
    h[i] = new TH1F("h", "", nbins, 0, len);
    snprintf(str, 128, "t%d", i);
    t2 = &getObject<TTree>(str);;
    t2->SetBranchAddress("time", &time);
    t2->SetBranchAddress("dist", &dist);
    n2 = t2->GetEntries();
    timeArray = new float[n2];
    distArray = new float[n2];
    for (j = 0; j < n2; j++) {
      t2->GetEntry(j);
      timeArray[j] = time;
      distArray[j] = dist;
    }
    for (j = 0; j < n2; j++) {
      if (distArray[j] >= len || distArray[j] < 0)
        continue;
      bin = floor(distArray[j] / len * nbins);
      if (mint[bin] > timeArray[j])
        mint[bin] = timeArray[j];
    }
    for (j = 0; j < nbins; j++) {
      snprintf(str, 128, "h2_%d", j);
      h2[j] = new TH1F(str, "", 100, mint[j], mint[j] + 10);
    }
    for (j = 0; j < n2; j++) {
      if (distArray[j] >= len || distArray[j] < 0)
        continue;
      bin = floor(distArray[j] / len * nbins);
      h2[bin]->Fill(timeArray[j]);
    }
    for (j = 0; j < nbins; j++) {
      fcn->SetParameter(0, h2[j]->Integral());
      fcn->SetParameter(1, h2[j]->GetMean());
      fcn->SetParameter(2, h2[j]->GetRMS());
      fcn->SetParameter(3, h2[j]->GetRMS());
      fcn->FixParameter(4, h2[j]->GetMean() + 1.0);
      fcn->FixParameter(5, 1.0);
      h2[j]->Fit("fcn");
      fcn->ReleaseParameter(4);
      fcn->ReleaseParameter(5);
      h2[j]->Fit("fcn");
      h[i]->SetBinContent(j + 1, fcn->GetParameter(1));
      h[i]->SetBinError(j + 1, fcn->GetParError(1));
      h2[j]->Draw();
      c1->Print("fit1.eps");
    }
    h[i]->Fit("fcn2");
    h[i]->Draw();
    c1->Print("fit2.eps");
    p0 = fcn2->GetParameter(0);
    /*
     * Shift of the most probable value is corrected from
     * the distribution for the entire strip.
     */
    h3 = new TH1F("h3", "", 130, -3., 10.);
    for (j = 0; j < n2; j++) {
      if (distArray[j] >= len || distArray[j] < 0)
        continue;
      h3->Fill(timeArray[j] - distArray[j] * p0);
    }
    maxval = -1;
    for (j = 1; j <= 130; j++) {
      if (h3->GetBinContent(j) > maxval) {
        maxval = h3->GetBinContent(j);
        p1 =  h3->GetBinCenter(j);
      }
    }
    tOut->Fill();
    delete timeArray;
    delete distArray;
    snprintf(str, 128, "time - (%g * dist + %g)", p0, p1);
    t2->Draw(str, (std::string(str) + "< 10").c_str());
    snprintf(str, 128, "corrtime%d.eps", i);
    c1->Print(str);
  }
  fOut->cd();
  tOut->Write();
  delete tOut;
  delete fOut;
  return CalibrationAlgorithm::c_OK;
}

