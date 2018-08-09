/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <algorithm>
#include <iostream>

#include <TLine.h>
#include <TCanvas.h>

#include <reconstruction/calibration/CDCDedx2DCellAlgorithm.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedx2DCellAlgorithm::CDCDedx2DCellAlgorithm() : CalibrationAlgorithm("CDCDedxElectronCollector")
{
  // Set module properties
  setDescription("A calibration algorithm for the CDC dE/dx two dimensional correction");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedx2DCellAlgorithm::calibrate()
{
  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree->GetEntries() < 100)
    return c_NotEnoughData;

  std::vector<double>* dedxhit = 0, *doca = 0, *enta = 0, *layer = 0;

  ttree->SetBranchAddress("dedxhit", &dedxhit);
  ttree->SetBranchAddress("layer", &layer);
  ttree->SetBranchAddress("doca", &doca);
  ttree->SetBranchAddress("enta", &enta);

  // binning is defined by the version number
  short version = 1;
  const int ndbins = 50, nebins = 50;
  double sentamax = 1.0;
  double docamax = 1.42;
  double ebinsize = 2.0 * sentamax / nebins;
  double dbinsize = 2.0 * docamax / ndbins;

  // fill containers for calibration
  std::vector<std::vector<double>> twoddedxcellinner(ndbins * nebins, std::vector<double>());
  std::vector<std::vector<double>> twoddedxcellouter(ndbins * nebins, std::vector<double>());
  TH2F* docaent = new TH2F("docaent", "", 100, -1 * docamax, 1 * docamax, 100, -1 * sentamax, 1 * sentamax);
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    int ebin, dbin;
    for (unsigned int j = 0; j < dedxhit->size(); ++j) {
      if (dedxhit->at(j) == 0) continue;
      double myenta = enta->at(j);

      // assume rotational symmetry
      if (myenta < -3.1416 / 2.0) myenta += 3.1416 / 2.0;
      else if (myenta > 3.1416 / 2.0) myenta -= 3.1416 / 2.0;
      if (abs(myenta) > 3.1416 / 2) continue;

      ebin = std::floor((std::sin(myenta) + sentamax) / ebinsize);
      dbin = std::floor((doca->at(j) + docamax) / dbinsize);
      if (std::abs(doca->at(j)) > docamax) continue;

      if (layer->at(j) < 8)
        twoddedxcellinner[nebins * dbin + ebin].push_back(dedxhit->at(j));
      else
        twoddedxcellouter[nebins * dbin + ebin].push_back(dedxhit->at(j));

      docaent->Fill(doca->at(j), std::sin(myenta));
    }
  }

  TCanvas* ctmpde = new TCanvas("tmpde", "tmpde", 900, 900);
  ctmpde->cd();
  docaent->Draw("colz");
  ctmpde->SaveAs("docaent.ps");

  // Print the histograms for quality control
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  ctmp->Divide(3, 3);
  std::stringstream psname; psname << "dedx_2dcell.ps[";
  ctmp->Print(psname.str().c_str());
  psname.str(""); psname << "dedx_2dcell.ps";

  TH1F* base = new TH1F("base", "", 250, 0, 5);
  TLine* tl = new TLine();

  // fill with ones (no corrections)
  std::vector<TH2F> ones;
  TH2F hones = TH2F("ones", "dE/dx in bins of DOCA/Enta;DOCA;Entrance Angle", ndbins, -1 * docamax, 1 * docamax, nebins,
                    -1 * sentamax, 1 * sentamax);
  // include 1s in underflow and overflow bins
  for (unsigned int i = 0; i <= ndbins + 1; ++i) {
    for (unsigned int j = 0; j <= nebins + 1; ++j) {
      hones.SetBinContent(i, j, 1);
    }
  }
  ones.push_back(hones);
  ones.push_back(hones);

  // vector of TH2F for calibration constants
  std::vector<TH2F> twodcors;

  // fit histograms to get gains in bins of DOCA and entrance angle
  TH2F twodcor = TH2F("twodcorrection", "dE/dx in bins of DOCA/Enta;DOCA;Entrance Angle", ndbins, -1 * docamax, 1 * docamax, nebins,
                      -1 * sentamax, 1 * sentamax);
  for (unsigned int i = 0; i < twoddedxcellinner.size(); ++i) {

    // remember to iterate bin number because ROOT histograms start with bin 1
    int ndbin = std::floor(i / nebins) + 1;
    int nebin = i % nebins + 1;
    ctmp->cd(i % 9 + 1);
    for (unsigned int j = 0; j < twoddedxcellinner[i].size(); ++j) {
      base->Fill(twoddedxcellinner[i][j]);
    }
    base->DrawCopy("hist");

    double mean = 1.0;
    if (twoddedxcellinner[i].size() < 10) {
      twodcor.SetBinContent(ndbin, nebin, mean); // <-- FIX ME, should return not enough data
    } else {
      mean *= calculateMean(twoddedxcellinner[i], 0.05, 0.25);
      twodcor.SetBinContent(ndbin, nebin, mean);
    }

    tl->SetX1(mean); tl->SetX2(mean);
    tl->SetY1(0); tl->SetY2(base->GetMaximum());
    tl->DrawClone("same");

    base->Reset();
    if ((i + 1) % 9 == 0 || i + 1 == twoddedxcellinner.size())
      ctmp->Print(psname.str().c_str());
  }
  twodcor.SetTitle("2D cell correction");
  twodcors.push_back(twodcor);

  twodcor.Reset();
  for (unsigned int i = 0; i < twoddedxcellouter.size(); ++i) {

    // remember to iterate bin number because ROOT histograms start with bin 1
    int ndbin = std::floor(i / nebins) + 1;
    int nebin = i % nebins + 1;
    ctmp->cd(i % 9 + 1);
    for (unsigned int j = 0; j < twoddedxcellouter[i].size(); ++j) {
      base->Fill(twoddedxcellouter[i][j]);
    }
    base->DrawCopy("hist");

    double mean = 1.0;
    if (twoddedxcellouter[i].size() < 10) {
      twodcor.SetBinContent(ndbin, nebin, mean); // <-- FIX ME, should return not enough data
    } else {
      mean *= calculateMean(twoddedxcellouter[i], 0.05, 0.25);
      twodcor.SetBinContent(ndbin, nebin, mean);
    }

    tl->SetX1(mean); tl->SetX2(mean);
    tl->SetY1(0); tl->SetY2(base->GetMaximum());
    tl->DrawClone("same");

    base->Reset();
    if ((i + 1) % 9 == 0 || i + 1 == twoddedxcellouter.size())
      ctmp->Print(psname.str().c_str());
  }
  twodcor.SetTitle("2D cell correction");
  twodcors.push_back(twodcor);

  psname.str(""); psname << "dedx_2dcell.ps]";
  ctmp->Print(psname.str().c_str());

  B2INFO("dE/dx calibration done for 2D correction");

  CDCDedx2DCell* gain = new CDCDedx2DCell(version, twodcors);
  //  CDCDedx2DCell* gain = new CDCDedx2DCell(version, ones);
  saveCalibration(gain, "CDCDedx2DCell");

  delete ctmpde;
  delete ctmp;
  delete docaent;
  delete base;
  delete tl;

  return c_OK;
}

double CDCDedx2DCellAlgorithm::calculateMean(const std::vector<double>& dedx,
                                             double removeLowest, double removeHighest) const
{
  // Calculate the truncated average by skipping the lowest & highest
  // events in the array of dE/dx values
  std::vector<double> sortedDedx = dedx;
  std::sort(sortedDedx.begin(), sortedDedx.end());

  double truncatedMean = 0.0;
  double mean = 0.0;
  double sumOfSquares = 0.0;
  int numValuesTrunc = 0;
  const int numDedx = sortedDedx.size();

  // add a factor of 0.5 here to make sure we are rounding appropriately...
  const int lowEdgeTrunc = int(numDedx * removeLowest + 0.5);
  const int highEdgeTrunc = int(numDedx * (1 - removeHighest) + 0.5);
  for (int i = 0; i < numDedx; i++) {
    mean += sortedDedx[i];
    if (i >= lowEdgeTrunc and i < highEdgeTrunc) {
      truncatedMean += sortedDedx[i];
      sumOfSquares += sortedDedx[i] * sortedDedx[i];
      numValuesTrunc++;
    }
  }

  if (numDedx != 0) mean /= numDedx;

  if (numValuesTrunc != 0) truncatedMean /= numValuesTrunc;
  else truncatedMean = mean;

  return truncatedMean;
}
