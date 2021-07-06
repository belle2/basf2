/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <calibration/example_caf_lib/TestBoundarySettingAlgorithm.h>

#include <memory>

#include <TH1F.h>

#include <calibration/dbobjects/TestCalibMean.h>

using namespace Belle2;
using namespace Calibration;

TestBoundarySettingAlgorithm::TestBoundarySettingAlgorithm(): CalibrationAlgorithm("CaTest")
{
  setDescription(
    " -------------------------- Test Calibration Algoritm -------------------------\n"
    "                                                                               \n"
    "  Testing algorithm which just gets mean of a test histogram collected by      \n"
    "  CaTest module and provides a DB object with another histogram with one       \n"
    "  entry at calibrated value.                                                   \n"
    " ------------------------------------------------------------------------------\n"
  );
}

CalibrationAlgorithm::EResult TestBoundarySettingAlgorithm::calibrate()
{
  // Pulling in data from collector output. It now returns shared_ptr<T> so the underlying pointer
  // will delete itself automatically at the end of this scope unless you do something
  auto hist = getObjectPtr<TH1F>("MyHisto");
  if (!hist) return c_Failure;
  B2INFO("Number of Entries in MyHisto was " << hist->GetEntries());

  float mean = hist->GetMean();
  float meanError = hist->GetMeanError();

  B2INFO("Mean of MyHisto was " << mean);
  B2INFO("Mean Error of MyHisto was " << meanError);

  if (hist->GetEntries() < 100)
    return c_NotEnoughData;

  // Example of saving a DBObject.
  TestCalibMean* correction = new TestCalibMean(mean, meanError);
  saveCalibration(correction, "TestCalibMean");

  // Decide if we need to iterate or if this value was fine
  if (mean - 42. >= 1.) {
    return c_Iterate;
  } else {
    return c_OK;
  }
}


bool TestBoundarySettingAlgorithm::isBoundaryRequired(const ExpRun& /*currentRun*/)
{
  // First run in data as we iterate, but our boundaries weren't set manually already?
  // Just set the first run to be a boundary and we are done.
  if (m_boundaries.empty()) {
    B2INFO("This is the first run encountered, let's say it is a boundary.");
    return true;
  } else {
    return false;
  }
}
