/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxRunGainAlgorithm.h>

#include <TF1.h>
#include <TH1F.h>
#include <TCanvas.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxRunGainAlgorithm::CDCDedxRunGainAlgorithm() : CalibrationAlgorithm("CDCDedxElectronCollector")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx run gains");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedxRunGainAlgorithm::calibrate()
{

  // Get data objects
  auto means = getObjectPtr<TH1F>("means");
  auto dbtree = getObjectPtr<TTree>("dbtree");

  if (means->GetEntries() < 100)
    return c_NotEnoughData;

  // Gets the current vector of ExpRun<int,int> and checks that not more than one was passed
  if (getRunList().size() > 1) {
    B2ERROR("More than one run executed in CDCDedxRunGainAlgorithm. This is not valid!");
    return c_Failure;
  }

  // get the existing constants (should only be one set)
  CDCDedxRunGain* dbRunGain = 0;
  dbtree->SetBranchAddress("runGain", &dbRunGain);
  if (dbtree->GetEntries() != 0) {
    dbtree->GetEvent(0);
  }

  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  double rungain = (dbRunGain) ? dbRunGain->getRunGain() : 1.0;
  if (means->Fit("gaus")) {
    rungain *= means->GetFunction("gaus")->GetParameter(1);
    ctmp->Print("dedx_rungains.ps");
  }

  B2INFO("dE/dx run gains done: " << rungain);

  CDCDedxRunGain* gain = new CDCDedxRunGain(rungain);
  saveCalibration(gain, "CDCDedxRunGain");

  return c_OK;
}
