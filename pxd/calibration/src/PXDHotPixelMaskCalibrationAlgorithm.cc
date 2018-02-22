/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//#include <framework/database/DBObjPtr.h>
//#include <framework/database/DBImportObjPtr.h>
//#include <framework/database/IntervalOfValidity.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/calibration/PXDHotPixelMaskCalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDMaskedPixelPar.h>

#include <vxd/geometry/GeoCache.h>
#include <boost/format.hpp>
#include "TH1I.h"

using namespace std;
using boost::format;
using namespace Belle2;


PXDHotPixelMaskCalibrationAlgorithm::PXDHotPixelMaskCalibrationAlgorithm(): CalibrationAlgorithm("PXDHotPixelMaskCollector"),
  minEvents(1000000), minHits(10), maxOccupancy(0.00001)
{
  setDescription(
    " -------------------------- PXDHotPixelMak Calibration Algorithm ------------------------\n"
    "                                                                                         \n"
    "  Algorithm which masks all single pixels with too large occupancy.                      \n"
    " ----------------------------------------------------------------------------------------\n"
  );
}

CalibrationAlgorithm::EResult PXDHotPixelMaskCalibrationAlgorithm::calibrate()
{

  auto hist_nevents = getObjectPtr<TH1I>("nevents");
  auto nevents = hist_nevents->GetEntries();
  if (nevents < minEvents) {
    B2INFO("Not Enough Data");
    return c_NotEnoughData;
  }

  PXDMaskedPixelPar* maskedPixelsPar = new PXDMaskedPixelPar();

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();

  if (gTools->getNumberOfPXDLayers() == 0) {
    B2WARNING("Missing geometry for PXD, PXD-masking is skiped.");
  }

  // Mask all hot pixels for PXD
  int nPXDSensors = gTools->getNumberOfPXDSensors();
  for (int i = 0; i < nPXDSensors; i++) {
    // Get hitmap for PXD sensor
    string name = str(format("PXD_%1%_PixelHitmap") % i);
    auto hpxdhitmap =  getObjectPtr<TH1I>(name.c_str());


    // Mask all hot pixel for this sensor
    VxdID id = gTools->getSensorIDFromPXDIndex(i);
    for (auto bin = 1; bin <= hpxdhitmap->GetXaxis()->GetNbins(); bin++) {
      float nhits = (float) hpxdhitmap->GetBinContent(bin);

      if (nhits > minHits) {
        if (nhits / nevents > maxOccupancy) {
          // This pixel is hot, we have to mask it
          maskedPixelsPar->maskSinglePixel(id.getID(), bin - 1);
        }
      }
    }
  }
  // Save the hot pixel mask to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(maskedPixelsPar, "PXDMaskedPixelPar");

  if (getIteration() < 1) {
    B2INFO("Calibration called for iteration");
    return c_Iterate;
  }

  B2INFO("PXDHotPixelMask Calibration Successful");
  return c_OK;
}
