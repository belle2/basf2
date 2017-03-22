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
#include <TNtuple.h>
#include <TF1.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxRunGainAlgorithm::CDCDedxRunGainAlgorithm() : CalibrationAlgorithm("CDCDedxRunGainCollector")
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
  auto& means = getObject<TH1F>("means");
  auto& ttree = getObject<TTree>("tree");

  // require at least 100 tracks (arbitrary for now)
  if (ttree.GetEntries() < 100)
    return c_NotEnoughData;

  int run;
  ttree.SetBranchAddress("run", &run);

  int lastrun = -1;
  for (int i = 0; i < ttree.GetEntries(); ++i) {
    ttree.GetEvent(i);
    if (lastrun == -1) lastrun = run;
    else if (run != lastrun) {
      B2WARNING("dE/dx run gain calibration failing - multiple runs included!");
      return c_Failure;
    }
  }

  means.Fit("gaus");
  float rungain = means.GetFunction("gaus")->GetParameter(1);

  //  TNtuple* gains = new TNtuple("runGains","CDC dE/dx run gains","run:gain");
  //  gains->Fill(lastrun,rungain);

  TH1F* gains = new TH1F("runGains", "CDC dE/dx run gains", 2, -0.5, 1.5);
  gains->SetBinContent(1, lastrun);
  gains->SetBinContent(2, rungain);

  B2INFO("dE/dx Calibration done for run " << lastrun << ": " << rungain);

  saveCalibration(gains, getPrefix());

  // Iterate
  //  B2INFO("mean: " << mean);
  //  if (mean - 42. >= 1.)
  //    return c_Iterate;

  return c_OK;
}
