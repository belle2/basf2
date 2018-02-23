/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dataobjects/VxdID.h>
#include <pxd/calibration/PXDHotPixelMaskCalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDMaskedPixelPar.h>

#include <boost/format.hpp>
#include <string>
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

  auto collector_pxdhits = getObjectPtr<TH1I>("PXDHits");
  auto nevents = collector_pxdhits->GetEntries();
  if (nevents < minEvents) {
    B2INFO("Not Enough Data");
    return c_NotEnoughData;
  }

  PXDMaskedPixelPar* maskedPixelsPar = new PXDMaskedPixelPar();

  // Loop over all sensor from collector
  auto collector_pxdhitcounts = getObjectPtr<TH1I>("PXDHitCounts");
  for (auto sensBin = 1; sensBin <= collector_pxdhitcounts->GetXaxis()->GetNbins(); sensBin++) {
    // The bin label is assumed to be a string representation of VxdID
    string sensorDescr =  collector_pxdhitcounts->GetXaxis()->GetBinLabel(sensBin);
    VxdID id(sensorDescr);

    // Get hitmap from collector
    string name = str(format("PXD_%1%_PixelHitmap") % id.getID());
    auto collector_pxdhitmap =  getObjectPtr<TH1I>(name.c_str());

    // Check if there was data collected for this sensor
    if (collector_pxdhitmap == nullptr) continue;

    // Mask all hot pixel for this sensor
    for (auto bin = 1; bin <= collector_pxdhitmap->GetXaxis()->GetNbins(); bin++) {
      float nhits = (float) collector_pxdhitmap->GetBinContent(bin);
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
