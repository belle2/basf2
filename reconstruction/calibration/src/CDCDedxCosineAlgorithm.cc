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

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxCosineAlgorithm::CDCDedxCosineAlgorithm() : CalibrationAlgorithm("CDCDedxElectronCollector")
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

  // make histograms to store dE/dx values in bins of cos(theta)
  // bin size can be arbitrary, for now just make uniform bins
  const int nbins = 40;
  std::vector<double> binedges;
  TH1F* dedxcosth[nbins];
  for (unsigned int i = 0; i < nbins; ++i) {
    binedges.push_back(-1.0 + 2.0 / nbins * i);
    dedxcosth[i] = new TH1F(TString::Format("dedxcosth%d", i), "dE/dx in bins of cosine", 100, 0, 2);
  }

  // fill histograms, bin size may be arbitrary
  for (int i = 0; i < ttree.GetEntries(); ++i) {
    ttree.GetEvent(i);
    if (costh < -1.0 || costh > 1.0) continue;
    auto it = std::lower_bound(binedges.begin(), binedges.end(), costh);
    int bin = std::distance(binedges.begin(), it) - 1;
    if (bin >= nbins) continue;
    dedxcosth[bin]->Fill(dedx);
  }

  // fit histograms to get gains in bins of cos(theta)
  std::map<double, double> cosine;
  for (unsigned int i = 0; i < nbins; ++i) {
    if (dedxcosth[i]->Integral() < 50)
      cosine[binedges[i]] = 1.0; // FIXME! --> should return not enough data
    else {
      int status = dedxcosth[i]->Fit("gaus");
      if (status != 0) {
        cosine[binedges[i]] = 1.0;
      } else {
        float mean = dedxcosth[i]->GetFunction("gaus")->GetParameter(1);
        cosine[binedges[i]] = mean;
      }
    }
  }

  B2INFO("dE/dx Calibration done for CDC dE/dx electron saturation");

  CDCDedxCosine* gain = new CDCDedxCosine(cosine);
  saveCalibration(gain, "CDCDedxCosine");

  return c_OK;
}
