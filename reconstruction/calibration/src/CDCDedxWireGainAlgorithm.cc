/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxWireGainAlgorithm.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxWireGainAlgorithm::CDCDedxWireGainAlgorithm() : CalibrationAlgorithm("CDCDedxWireGainCollector")
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
  auto& ttree = getObject<TTree>("tree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree.GetEntries() < 100)
    return c_NotEnoughData;

  int nhits;
  int wire[100];
  int layer[100];
  double dedxhit[100];
  ttree.SetBranchAddress("nhits", &nhits);
  ttree.SetBranchAddress("wire", wire);
  ttree.SetBranchAddress("layer", layer);
  ttree.SetBranchAddress("dedxhit", dedxhit);

  std::map<int, std::vector<double> > wirededx;
  for (int i = 0; i < ttree.GetEntries(); ++i) {
    ttree.GetEvent(i);
    for (int j = 0; j < nhits; ++j) {
      int wireid = WireID(layer[j], wire[j]).getEWire();
      wirededx[wireid].push_back(dedxhit[j]);
    }
  }

  B2INFO("dE/dx Calibration done for " << wirededx.size() << " CDC wires");

  //  if( wirededx.size() < 14336 ) return c_Failure;

  TH2F* gains = new TH2F("wireGains", "CDC dE/dx wire gains", 14337, -0.5, 14336.5, 2, -0.5, 1.5);
  int counter = 1;
  for (auto const& awire : wirededx) {
    B2INFO("Wire " << awire.first << ": " << calculateMean(awire.second, 0.05, 0.25));
    gains->SetBinContent(counter, 1, awire.first);
    gains->SetBinContent(counter, 2, calculateMean(awire.second, 0.05, 0.25));
    counter++;
  }
  saveCalibration(gains, getPrefix());

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
