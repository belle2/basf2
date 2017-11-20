/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

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

  std::vector<double>* dedx = 0, *doca = 0, *enta = 0;
  TBranch* bdedx = 0, *bdoca = 0, *benta = 0;

  ttree->SetBranchAddress("dedxhit", &dedx, &bdedx);
  ttree->SetBranchAddress("doca", &doca, &bdoca);
  ttree->SetBranchAddress("enta", &enta, &benta);

  const int ndbins = 10, nebins = 10;
  TH1F twodbin[ndbins][nebins];
  for (unsigned int i = 0; i < ndbins; ++i) {
    for (unsigned int j = 0; j < nebins; ++j) {
      twodbin[i][j] = TH1F(TString::Format("twodbin%d%d", i, j), "dE/dx in bins of DOCA/Enta", 100, 0, 2);
    }
  }

  // fill histograms
  for (unsigned int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < dedx->size(); ++j) {
      double myenta = enta->at(j);

      // assume rotational symmetry
      if (myenta < -3.1416 / 2.0) myenta += 3.1416 / 2.0;
      if (myenta > 3.1416 / 2.0) myenta -= 3.1416 / 2.0;
      if (abs(myenta) > 3.1416) continue;
      int ebin = (std::sin(myenta) + 1.0) / nebins;

      int dbin;
      if (abs(doca->at(j)) > 1.5) dbin = ndbins - 1;
      else dbin = (doca->at(j) + 1.5) / ndbins;

      twodbin[dbin][ebin].Fill(dedx->at(j));
    }
  }

  // fit histograms to get gains in bins of DOCA and entrance angle
  TH2F twodcor = TH2F("twodcorrection", "dE/dx in bins of DOCA/Enta", ndbins, -1.5, 1.5, nebins, -1, 1);
  for (unsigned int i = 0; i < ndbins; ++i) {
    for (unsigned int j = 0; j < nebins; ++j) {
      if (twodbin[i][j].Integral() < 50) {
        twodcor.SetBinContent(i + 1, j + 1, 1.0); // <-- FIX ME, should return not enough data
        continue;
      }
      int status = twodbin[i][j].Fit("gaus");
      if (status != 0) {
        twodcor.SetBinContent(i + 1, j + 1, 1.0); // <-- FIX ME, should return not enough data
      } else {
        float mean = twodbin[i][j].GetFunction("gaus")->GetParameter(1);
        twodcor.SetBinContent(i + 1, j + 1, mean);
      }
    }
  }

  B2INFO("dE/dx Calibration done for 2D correction");

  std::vector<TH2F> twodcors;
  twodcors.push_back(twodcor);

  CDCDedx2DCor* gain = new CDCDedx2DCor(0, twodcors);
  saveCalibration(gain, "CDCDedx2DCor");

  return c_OK;
}
