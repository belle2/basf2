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

#include <reconstruction/calibration/CDCDedx2DCorrectionAlgorithm.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedx2DCorrectionAlgorithm::CDCDedx2DCorrectionAlgorithm() : CalibrationAlgorithm("CDCDedxElectronCollector")
{
  // Set module properties
  setDescription("A calibration algorithm for the CDC dE/dx two dimensional correction");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedx2DCorrectionAlgorithm::calibrate()
{
  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree->GetEntries() < 100)
    return c_NotEnoughData;

  std::vector<double>* dedxhit = 0, *doca = 0, *enta = 0;

  ttree->SetBranchAddress("dedxhit", &dedxhit);
  ttree->SetBranchAddress("doca", &doca);
  ttree->SetBranchAddress("enta", &enta);

  const int ndbins = 10, nebins = 10;
  double dbinsize = 2.0 / ndbins, ebinsize = 2.0 / nebins;
  std::vector<std::vector<double>> twoddedx(ndbins * nebins, std::vector<double>());
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < dedxhit->size(); ++j) {
      double myenta = enta->at(j);

      // assume rotational symmetry
      if (myenta < -3.1416 / 2.0) myenta += 3.1416 / 2.0;
      else if (myenta > 3.1416 / 2.0) myenta -= 3.1416 / 2.0;
      if (abs(myenta) > 3.1416 / 2) continue;

      int ebin = std::floor((std::sin(myenta) + 1.0) / ebinsize);
      int dbin = std::floor((doca->at(j) + 1.0) / dbinsize);
      if (doca->at(j) > 1.0) dbin = ndbins - 1;
      else if (doca->at(j) < -1.0) dbin = 0;

      twoddedx[nebins * dbin + ebin].push_back(dedxhit->at(j));
    }
  }

  // Print the histograms for quality control
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  ctmp->Divide(3, 3);
  std::stringstream psname; psname << "dedx_2dcor.ps[";
  ctmp->Print(psname.str().c_str());
  psname.str(""); psname << "dedx_2dcor.ps";

  TH1F* base = new TH1F("base", "", 250, 0, 5);
  TLine* tl = new TLine();

  // fit histograms to get gains in bins of DOCA and entrance angle
  TH2F twodcor = TH2F("twodcorrection", "dE/dx in bins of DOCA/Enta", ndbins, -1.0, 1.0, nebins, -1, 1);
  for (unsigned int i = 0; i < twoddedx.size(); ++i) {
    int ndbin = std::floor(i / nebins) + 1;
    int nebin = i % nebins + 1;
    ctmp->cd(i % 9 + 1);
    for (unsigned int j = 0; j < twoddedx[i].size(); ++j) {
      base->Fill(twoddedx[i][j]);
    }
    base->DrawCopy("hist");

    double mean = 1.0;
    if (twoddedx[i].size() < 10) {
      twodcor.SetBinContent(ndbin, nebin, mean); // <-- FIX ME, should return not enough data
    } else {
      mean = calculateMean(twoddedx[i], 0.05, 0.25);
      twodcor.SetBinContent(ndbin, nebin, mean); // <-- FIX ME, should return not enough data
    }

    std::cout << i << "\t" << ndbin << "\t" << nebin << "\t" << mean << std::endl;

    tl->SetX1(mean); tl->SetX2(mean);
    tl->SetY1(0); tl->SetY2(base->GetMaximum());
    tl->Draw("same");

    base->Reset();
    if ((i + 1) % 9 == 0 || i + 1 == twoddedx.size())
      ctmp->Print(psname.str().c_str());
  }

  psname.str(""); psname << "dedx_2dcor.ps]";
  ctmp->Print(psname.str().c_str());
  delete ctmp;
  delete tl;

  B2INFO("dE/dx Calibration done for 2D correction");

  std::vector<TH2F> twodcors;
  twodcors.push_back(twodcor);

  CDCDedx2DCor* gain = new CDCDedx2DCor(0, twodcors);
  saveCalibration(gain, "CDCDedx2DCor");

  return c_OK;
}

double CDCDedx2DCorrectionAlgorithm::calculateMean(const std::vector<double>& dedx,
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
