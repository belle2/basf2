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

  if (means->GetEntries() < 100)
    return c_NotEnoughData;

  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  means->Fit("gaus");
  double rungain = means->GetFunction("gaus")->GetParameter(1);
  ctmp->Print("dedx_rungains.ps");

  B2INFO("dE/dx run gains done: " << rungain);

  CDCDedxRunGain* gain = new CDCDedxRunGain(rungain);
  saveCalibration(gain, "CDCDedxRunGain");

  return c_OK;
}
