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

#include <TH1F.h>
#include <TLine.h>
#include <TCanvas.h>

#include <reconstruction/calibration/CDCDedxWireGainAlgorithm.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxWireGainAlgorithm::CDCDedxWireGainAlgorithm() : CalibrationAlgorithm("CDCDedxElectronCollector")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx wire gains");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedxWireGainAlgorithm::calibrate()
{
  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree->GetEntries() < 100)
    return c_NotEnoughData;

  // HAVE to set these pointers to 0!
  std::vector<int>* wire = 0;
  std::vector<double>* dedxhit = 0;

  ttree->SetBranchAddress("wire", &wire);
  ttree->SetBranchAddress("dedxhit", &dedxhit);

  // make vectors to store dE/dx values for each wire
  std::vector<std::vector<double>> wirededx(14336, std::vector<double>());
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < wire->size(); ++j) {
      if (dedxhit->at(j) == 0) continue;
      wirededx[wire->at(j)].push_back(dedxhit->at(j));
    }
  }

  // Print the histograms for quality control
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  ctmp->Divide(3, 3);
  std::stringstream psname; psname << "dedx_wiregains.ps[";
  ctmp->Print(psname.str().c_str());
  psname.str(""); psname << "dedx_wiregains.ps";

  TH1F* base = new TH1F("base", "", 250, 0, 5);
  TLine* tl = new TLine();
  unsigned int outermin = 8 * 160;
  double outeravg = 0; int nouterwires = 0;

  std::vector<double> means;
  for (unsigned int i = 0; i < wirededx.size(); ++i) {
    ctmp->cd(i % 9 + 1); // each canvas is 9x9
    for (unsigned int j = 0; j < wirededx[i].size(); ++j) {
      base->Fill(wirededx[i][j]);
    }
    base->DrawCopy("hist");

    double mean = 1.0;
    if (wirededx[i].size() < 10) {
      means.push_back(mean); // <-- FIX ME, should return not enough data
    } else {
      mean *= calculateMean(wirededx[i], 0.05, 0.25);
      means.push_back(mean);
      if (i >= outermin) {
        outeravg += mean;
        nouterwires++;
      }
    }

    tl->SetX1(mean); tl->SetX2(mean);
    tl->SetY1(0); tl->SetY2(base->GetMaximum());
    tl->Draw("same");

    base->Reset();
    if ((i + 1) % 9 == 0)
      ctmp->Print(psname.str().c_str());
  }

  psname.str(""); psname << "dedx_wiregains.ps]";
  ctmp->Print(psname.str().c_str());
  delete ctmp;
  delete base;
  delete tl;

  // Normalize the outer layers to 1
  outeravg /= nouterwires;
  for (unsigned int i = 0; i < 14336; ++i) {
    means[i] /= outeravg;
  }

  B2INFO("dE/dx Calibration done for " << means.size() << " CDC wires");

  CDCDedxWireGain* gains = new CDCDedxWireGain(means);
  saveCalibration(gains, "CDCDedxWireGain");

  return c_OK;
}

double CDCDedxWireGainAlgorithm::calculateMean(const std::vector<double>& dedx,
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
