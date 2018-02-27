/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDClusterPositionCalibrationAlgorithm.h>

#include <string>


using namespace std;
using namespace Belle2;


PXDClusterPositionCalibrationAlgorithm::PXDClusterPositionCalibrationAlgorithm():
  CalibrationAlgorithm("PXDClusterPositionCollector"),
  minClusterForShapeLikelyhood(500), minClusterForPositionOffset(2000)
{
  setDescription(
    " -------------------------- PXDClusterPositionCalibrationAlgorithm ----------------------\n"
    "                                                                                         \n"
    "  Algorithm for estimating cluster position offsets and shape likelyhoods.               \n"
    " ----------------------------------------------------------------------------------------\n"
  );
}

CalibrationAlgorithm::EResult PXDClusterPositionCalibrationAlgorithm::calibrate()
{


  // Save the hot pixel mask to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  //PXDMaskedPixelPar* maskedPixelsPar = new PXDMaskedPixelPar();
  //saveCalibration(maskedPixelsPar, "PXDMaskedPixelPar");

  if (getIteration() < 1) {
    B2INFO("Calibration called for iteration");
    return c_Iterate;
  }

  B2INFO("PXDClusterPosition Calibration Successful");
  return c_OK;
}
