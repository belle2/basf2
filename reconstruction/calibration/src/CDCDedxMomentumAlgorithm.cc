/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxMomentumAlgorithm.h>

#include <TF1.h>
#include <TH1F.h>
#include <TCanvas.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxMomentumAlgorithm::CDCDedxMomentumAlgorithm() : CalibrationAlgorithm("CDCDedxElectronCollector")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx electron cos(theta) dependence");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedxMomentumAlgorithm::calibrate()
{
  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree->GetEntries() < 100)
    return c_NotEnoughData;

  double dedx, p;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("p", &p);

  // make histograms to store dE/dx values in bins of cos(theta)
  // bin size can be arbitrary, for now just make uniform bins
  const int nbins = 100;
  TH1F dedxp[nbins];
  for (unsigned int i = 0; i < nbins; ++i) {
    dedxp[i] = TH1F(TString::Format("dedxp%d", i), "dE/dx in bins of momentum", 100, 0, 2);
  }

  // fill histograms, bin size may be arbitrary
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    if (p <= 0.0 || p > 10.0) continue;
    int bin = std::floor(p / (10.0 / nbins));
    if (bin < 0 || bin >= nbins) continue;
    dedxp[bin].Fill(dedx);
  }

  // Print the histograms for quality control
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  ctmp->Divide(3, 3);
  std::stringstream psname; psname << "dedx_momentum.ps[";
  ctmp->Print(psname.str().c_str());
  psname.str(""); psname << "dedx_momentum.ps";

  // fit histograms to get gains in bins of cos(theta)
  std::vector<double> momentum;
  for (unsigned int i = 0; i < nbins; ++i) {
    ctmp->cd(i % 9 + 1); // each canvas is 9x9
    dedxp[i].DrawCopy("hist");

    if (dedxp[i].Integral() < 10)
      momentum.push_back(1.0); // FIXME! --> should return not enough data
    else {
      int status = dedxp[i].Fit("gaus");
      if (status != 0) {
        momentum.push_back(1.0); // FIXME! --> should return not enough data
      } else {
        float mean = dedxp[i].GetFunction("gaus")->GetParameter(1);
        momentum.push_back(mean);
      }
    }
    if ((i + 1) % 9 == 0)
      ctmp->Print(psname.str().c_str());
  }

  psname.str(""); psname << "dedx_momentum.ps]";
  ctmp->Print(psname.str().c_str());
  delete ctmp;

  B2INFO("dE/dx Calibration done for CDC dE/dx momentum correction");

  CDCDedxMomentumCor* gain = new CDCDedxMomentumCor(nbins, momentum);
  saveCalibration(gain, "CDCDedxMomentumCor");

  return c_OK;
}
