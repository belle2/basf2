/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedx1DCleanupAlgorithm.h>
#include <TF1.h>
#include <TH1F.h>
#include <TTree.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedx1DCleanupAlgorithm::CDCDedx1DCleanupAlgorithm() : CalibrationAlgorithm("CDCDedxElectronCollector")
{
  // Set module properties
  setDescription("A calibration algorithm for the CDC dE/dx entrance angle cleanup correction");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedx1DCleanupAlgorithm::calibrate()
{
  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree->GetEntries() < 100)
    return c_NotEnoughData;

  std::vector<double>* dedx = 0, *enta = 0;
  TBranch* bdedx = 0, *benta = 0;

  ttree->SetBranchAddress("dedxhit", &dedx, &bdedx);
  ttree->SetBranchAddress("enta", &enta, &benta);

  // make histograms to store dE/dx values in bins of entrance angle
  const int nbins = 20;
  double binsize = 2.0 / nbins;
  TH1F dedxenta[nbins];
  for (unsigned int i = 0; i < nbins; ++i) {
    dedxenta[i] = TH1F(TString::Format("dedxenta%d", i), "dE/dx in bins of cosine", 100, 0, 2);
  }

  // fill histograms
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < dedx->size(); ++j) {
      double myenta = enta->at(j);

      // assume rotational symmetry
      if (myenta < -3.1416 / 2.0) myenta += 3.1416 / 2.0;
      if (myenta > 3.1416 / 2.0) myenta -= 3.1416 / 2.0;

      int bin = std::floor(sin(myenta) / binsize);
      if (bin < 0 || bin >= nbins) continue;
      dedxenta[bin].Fill(dedx->at(j));
    }
  }

  // fit histograms to get gains in bins of entrance angle
  std::vector<double> onedcor;
  for (unsigned int i = 0; i < nbins; ++i) {
    if (dedxenta[i].Integral() < 50)
      onedcor.push_back(1.0); // FIXME! --> should return not enough data
    else {
      int status = dedxenta[i].Fit("gaus");
      if (status != 0) {
        onedcor.push_back(1.0); // FIXME! --> should return not enough data
      } else {
        float mean = dedxenta[i].GetFunction("gaus")->GetParameter(1);
        onedcor.push_back(mean);
      }
    }
  }

  std::vector<std::vector<double>> onedcors;
  onedcors.push_back(onedcor);

  B2INFO("dE/dx Calibration done for 1D cleanup correction");

  CDCDedx1DCleanup* gain = new CDCDedx1DCleanup(0, nbins, onedcors);
  saveCalibration(gain, "CDCDedx1DCleanup");

  return c_OK;
}
