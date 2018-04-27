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

#include <TCanvas.h>
#include <TH1F.h>
#include <TLine.h>
#include <TTree.h>

#include <reconstruction/calibration/CDCDedx1DCellAlgorithm.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedx1DCellAlgorithm::CDCDedx1DCellAlgorithm() : CalibrationAlgorithm("CDCDedxElectronCollector")
{
  // Set module properties
  setDescription("A calibration algorithm for the CDC dE/dx entrance angle cleanup correction");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedx1DCellAlgorithm::calibrate()
{
  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree->GetEntries() < 100)
    return c_NotEnoughData;

  std::vector<double>* dedxhit = 0, *enta = 0;

  ttree->SetBranchAddress("enta", &enta);
  ttree->SetBranchAddress("dedxhit", &dedxhit);

  // make histograms to store dE/dx values in bins of entrance angle
  const int nbins = 20;
  double binsize = 2.0 / nbins;
  std::vector<std::vector<double>> entadedx(nbins, std::vector<double>());
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < dedxhit->size(); ++j) {
      if (dedxhit->at(j) == 0) continue;
      double myenta = enta->at(j);

      // assume rotational symmetry
      if (myenta < -3.1416 / 2.0) myenta += 3.1416 / 2.0;
      else if (myenta > 3.1416 / 2.0) myenta -= 3.1416 / 2.0;
      if (abs(myenta) > 3.1416 / 2) continue;

      int bin = std::floor((sin(myenta) + 1) / binsize);
      if (bin < 0 || bin >= nbins) continue;
      entadedx[bin].push_back(dedxhit->at(j));
    }
  }

  // Print the histograms for quality control
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  ctmp->Divide(3, 3);
  std::stringstream psname; psname << "dedx_1dcell.ps[";
  ctmp->Print(psname.str().c_str());
  psname.str(""); psname << "dedx_1dcell.ps";

  TH1F* base = new TH1F("base", "", 250, 0, 5);
  TLine* tl = new TLine();

  // fit histograms to get gains in bins of entrance angle
  int size = (m_DB1DCell) ? m_DB1DCell->getSize() : 0;
  std::vector<double> onedcor;
  for (unsigned int i = 0; i < entadedx.size(); ++i) {
    ctmp->cd(i % 9 + 1); // each canvas is 9x9
    for (unsigned int j = 0; j < entadedx[i].size(); ++j) {
      base->Fill(entadedx[i][j]);
    }
    base->DrawCopy("hist");

    double mean = (nbins == size) ? m_DB1DCell->getMean(0, i) : 1.0;
    if (entadedx[i].size() < 10) {
      onedcor.push_back(mean); // <-- FIX ME, should return not enough data
    } else {
      mean *= calculateMean(entadedx[i], 0.05, 0.25);
      onedcor.push_back(mean);
    }

    tl->SetX1(mean); tl->SetX2(mean);
    tl->SetY1(0); tl->SetY2(base->GetMaximum());
    tl->DrawClone("same");

    base->Reset();
    if ((i + 1) % 9 == 0 || i + 1 == entadedx.size())
      ctmp->Print(psname.str().c_str());
  }

  psname.str(""); psname << "dedx_1dcell.ps]";
  ctmp->Print(psname.str().c_str());
  delete ctmp;

  std::vector<std::vector<double>> onedcors;
  onedcors.push_back(onedcor);

  B2INFO("dE/dx Calibration done for 1D cleanup correction");

  CDCDedx1DCell* gain = new CDCDedx1DCell(0, onedcors);
  saveCalibration(gain, "CDCDedx1DCell");

  return c_OK;
}

double CDCDedx1DCellAlgorithm::calculateMean(const std::vector<double>& dedx,
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
