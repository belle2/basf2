/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/calibration/PXDHotPixelMaskCalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDMaskedPixelPar.h>


#include "TH1I.h"

using namespace std;
using namespace Belle2;

PXDHotPixelMaskCalibrationAlgorithm::PXDHotPixelMaskCalibrationAlgorithm(): CalibrationAlgorithm("PXDHotPixelMaskCollector"),
  minEvents(20000), minHits(10), maxOccupancy(0.01)
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

  PXDMaskedPixelPar* maskedPixelsPar = new PXDMaskedPixelPar();

  auto hist_nevents = getObjectPtr<TH1I>("nevents");
  auto nevents = hist_nevents->GetEntries();
  if (nevents < minEvents) {
    B2INFO("Not Enough Data");
    return c_NotEnoughData;
  }

  auto hist_hitmap = getObjectPtr<TH1I>("hitmap");

  //Loop over all bins (=pixels) in hitmap
  for (auto bin = 1; bin <= hist_hitmap->GetXaxis()->GetNbins(); bin++) {
    float nhits = (float) hist_hitmap->GetBinContent(bin);
    unsigned int pixelID = bin - 1;

    if (nhits > minHits) {
      if (nhits / nevents > maxOccupancy) {
        // This pixel is hot, we have to mask it
        maskedPixelsPar->maskSinglePixel(VxdID("1.1.1").getID(), pixelID);
        maskedPixelsPar->maskSinglePixel(VxdID("1.1.2").getID(), pixelID);
        maskedPixelsPar->maskSinglePixel(VxdID("2.1.1").getID(), pixelID);
        maskedPixelsPar->maskSinglePixel(VxdID("2.1.2").getID(), pixelID);
      }
    }
  }

  // Save the TNtuple to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(maskedPixelsPar, "PXDMaskedPixelPar");

  if (getIteration() < 1) {
    B2INFO("Calibration called for iteration");
    return c_Iterate;
  }

  B2INFO("PXDHotPixelMask Calibration Successful");
  return c_OK;
}
