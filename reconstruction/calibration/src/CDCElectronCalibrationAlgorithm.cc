/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/calibration/CDCElectronCalibrationAlgorithm.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCElectronCalibrationAlgorithm::CDCElectronCalibrationAlgorithm() : CalibrationAlgorithm("CDCElectronCollector")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx electron calibration");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCElectronCalibrationAlgorithm::calibrate()
{

  // Get data objects
  auto& gains = getObject<TH1F>("gains");
  auto& ttree = getObject<TTree>("tree");

  // require at least 20 tracks (arbitrary)
  if (gains.GetEntries() < 20 || ttree.GetEntries() < 20)
    return c_NotEnoughData;

  Double_t mean = gains.GetMean();
  Double_t meanerror = gains.GetMeanError();

  if (meanerror <= 0)
    return c_Failure;

  static int nameDistinguisher(0);
  TH1I* correction = new TH1I(TString::Format("constant-in-histo%d", nameDistinguisher),
                              "Mean value of calibration test histo", 200, 0, 200);
  nameDistinguisher++;
  correction->Fill((int)mean);

  saveCalibration(correction, getPrefix());

  // Iterate
  //  B2INFO("mean: " << mean);
  //  if (mean - 42. >= 1.)
  //    return c_Iterate;

  return c_OK;
}
