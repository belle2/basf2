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

  std::vector<std::vector<double>> wirededx(14336, std::vector<double>());
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < wire->size(); ++j) {
      wirededx[wire->at(j)].push_back(dedxhit->at(j));
    }
  }

  std::vector<double> means;
  for (unsigned int i = 0; i < 14336; ++i) {
    if (wirededx[i].size() < 50) {
      means.push_back(1.0); // <-- FIX ME, should return not enough data
    } else {
      double mean = calculateMean(wirededx[i], 0.05, 0.25);
      means.push_back(mean);
    }
  }

  B2INFO("dE/dx Calibration done for " << means.size() << " CDC wires");

  CDCDedxWireGain* gains = new CDCDedxWireGain(means);
  saveCalibration(gains, "CDCDedxWireGain");

  // Iterate
  //  B2INFO("mean: " << mean);
  //  if (mean - 42. >= 1.)
  //    return c_Iterate;

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
