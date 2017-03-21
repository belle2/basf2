/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxCosineAlgorithm.h>
#include <TF1.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxCosineAlgorithm::CDCDedxCosineAlgorithm() : CalibrationAlgorithm("CDCDedxCosineCollector")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx electron cos(theta) dependence");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedxCosineAlgorithm::calibrate()
{

  // Get data objects
  auto& ttree = getObject<TTree>("tree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree.GetEntries() < 100)
    return c_NotEnoughData;

  double dedx, costh;
  ttree.SetBranchAddress("dedx", &dedx);
  ttree.SetBranchAddress("costh", &costh);

  const int nbins = 101;
  TH1F* dedxcosth[nbins];
  for (int i = 0; i < nbins; ++i) {
    dedxcosth[i] = new TH1F(TString::Format("dedxcosth%d", i), "dE/dx in bins of cosine", 100, 20, 70);
  }

  for (int i = 0; i < ttree.GetEntries(); ++i) {
    ttree.GetEvent(i);
    if (costh < -1.0 || costh > 1.0) continue;
    int bin = (int)((costh + 1.0) / 2.0 * nbins);
    dedxcosth[bin]->Fill(dedx);
  }

  B2INFO("dE/dx Calibration for CDC dE/dx electron saturation");

  for (int i = 0; i < nbins; ++i) {
    B2INFO("Bin " << i + 1 << ": " << dedxcosth[i]->Integral());
  }

  TH1F* means = new TH1F("cosine", "CDC dE/dx cosine correction", nbins, -1.0, 1.0);
  for (int i = 0; i < nbins; ++i) {
    if (dedxcosth[i]->Integral() < 100) {
      means->SetBinContent(i + 1, 0);
      continue;
    }
    dedxcosth[i]->Fit("gaus");
    double mean = dedxcosth[i]->GetFunction("gaus")->GetParameter(1);
    means->SetBinContent(i + 1, mean);
    B2INFO("cos(theta) = " << means->GetBinCenter(i + 1));
  }

  for (int i = 0; i < nbins; ++i) {
    B2INFO("Bin " << i + 1 << ": " << means->GetBinContent(i + 1));
  }

  saveCalibration(means, getPrefix());

  return c_OK;
}
