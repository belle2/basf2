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
#include <TH1F.h>
#include <TCanvas.h>

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

  B2INFO("Preparing dE/dx calibration done for CDC dE/dx electron saturation");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  auto dbtree = getObjectPtr<TTree>("dbtree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree->GetEntries() < 100)
    return c_NotEnoughData;

  double dedx, costh;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("costh", &costh);

  // Gets the current vector of ExpRun<int,int> and checks that not more than one was passed
  if (getRunList().size() > 1) {
    B2ERROR("More than one run executed in CDCDedxCosineAlgorithm. This is not valid!");
    return c_Failure;
  }

  // get the existing constants (should only be one set)
  CDCDedxCosineCor* dbCosineCor = 0;
  dbtree->SetBranchAddress("cosineCor", &dbCosineCor);
  int size = 0;
  if (dbtree->GetEntries() != 0) {
    dbtree->GetEvent(0);
    size = dbCosineCor->getSize();
  }

  // make histograms to store dE/dx values in bins of cos(theta)
  // bin size can be arbitrary, for now just make uniform bins
  const int nbins = 100;
  TH1F dedxcosth[nbins];
  for (unsigned int i = 0; i < nbins; ++i) {
    dedxcosth[i] = TH1F(TString::Format("dedxcosth%d", i), "dE/dx in bins of cosine", 100, 0, 2);
  }

  // fill histograms, bin size may be arbitrary
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    if (dedx == 0) continue;
    if (costh < -1.0 || costh > 1.0) continue;
    int bin = (costh + 1.0) / (2.0 / nbins);
    if (bin < 0 || bin >= nbins) continue;
    dedxcosth[bin].Fill(dedx);
  }

  // Print the histograms for quality control
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  ctmp->Divide(3, 3);
  std::stringstream psname; psname << "dedx_cosine.ps[";
  ctmp->Print(psname.str().c_str());
  psname.str(""); psname << "dedx_cosine.ps";

  // fit histograms to get gains in bins of cos(theta)
  std::vector<double> cosine;
  B2WARNING(size << "\t" << nbins);
  for (unsigned int i = 0; i < nbins; ++i) {
    ctmp->cd(i % 9 + 1); // each canvas is 9x9
    dedxcosth[i].DrawCopy("hist");

    double mean = (nbins == size) ? dbCosineCor->getMean(i) : 1.0;
    B2WARNING(mean << "\t" << dbCosineCor->getMean(i));
    if (dedxcosth[i].Integral() < 10)
      cosine.push_back(mean); // FIXME! --> should return not enough data
    else {
      if (dedxcosth[i].Fit("gaus")) {
        cosine.push_back(mean); // FIXME! --> should return not enough data
      } else {
        mean *= dedxcosth[i].GetFunction("gaus")->GetParameter(1);
        cosine.push_back(mean);
      }
    }
    if ((i + 1) % 9 == 0)
      ctmp->Print(psname.str().c_str());
  }

  psname.str(""); psname << "dedx_cosine.ps]";
  ctmp->Print(psname.str().c_str());
  delete ctmp;

  B2INFO("dE/dx calibration done for CDC dE/dx electron saturation");

  CDCDedxCosineCor* gain = new CDCDedxCosineCor(cosine);
  saveCalibration(gain, "CDCDedxCosineCor");

  return c_OK;
}
