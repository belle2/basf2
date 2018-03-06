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
#include <vector>
#include <map>
#include "TH1I.h"

using namespace std;
using boost::format;
using namespace Belle2;


PXDHotPixelMaskCalibrationAlgorithm::PXDHotPixelMaskCalibrationAlgorithm(): CalibrationAlgorithm("PXDHotPixelMaskCollector"),
  minEvents(10000), minHits(5), maxOccupancy(0.00001), maskUCells(false), minHitsU(200), maxOccupancyU(0.00001), maskVCells(false),
  minHitsV(50), maxOccupancyV(0.00001)
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

  // This is the masking payload for conditions DB
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

    vector<float> unmaskedHitsAlongU(250, 0);
    vector<int> unmaskedCellsAlongU(250, 0);

    vector<float> unmaskedHitsAlongV(768, 0);
    vector<int> unmaskedCellsAlongV(768, 0);

    // Mask all hot pixel for this sensor
    for (auto bin = 1; bin <= collector_pxdhitmap->GetXaxis()->GetNbins(); bin++) {
      // Find the current pixel cell
      int pixID = bin - 1;
      int uCell = pixID / 768;
      int vCell = pixID % 768;
      // First, we mask single pixels exceeding occupancy threshold
      float nhits = (float) collector_pxdhitmap->GetBinContent(bin);
      bool masked = false;
      if (nhits > minHits) {
        if (nhits / nevents > maxOccupancy) {
          // This pixel is hot, we have to mask it
          maskedPixelsPar->maskSinglePixel(id.getID(), pixID);
          masked = true;
          B2RESULT("Masking single pixel with ucell=" << uCell << ", vcell=" << vCell << " on sensor " << id);
        }
      }
      // Then we accumulate hits along u and v direction for unmasked
      // pixels
      if (not masked) {
        ++unmaskedCellsAlongU[uCell];
        unmaskedHitsAlongU[uCell] += nhits;
        ++unmaskedCellsAlongV[vCell];
        unmaskedHitsAlongV[vCell] += nhits;
      }
    }

    if (maskUCells) {
      for (auto uCell = 0; uCell < 250; uCell++) {
        if (unmaskedHitsAlongU[uCell] > minHitsU && unmaskedCellsAlongU[uCell] > 0) {
          // Compute average occupancy per uCell
          float occupancy = unmaskedHitsAlongU[uCell] / unmaskedCellsAlongU[uCell];
          // Mask residual hot uCell
          if (occupancy > maxOccupancyU) {
            for (auto vCell = 0; vCell < 768; vCell++)
              maskedPixelsPar->maskSinglePixel(id.getID(),  uCell * 768 + vCell);

            B2RESULT("Masking complete ucell=" << uCell << " on sensor " << id);
          }
        }
      }
    }

    if (maskVCells) {
      for (auto vCell = 0; vCell < 768; vCell++) {
        if (unmaskedHitsAlongV[vCell] > minHitsV && unmaskedCellsAlongV[vCell] > 0) {
          // Compute average occupancy per vCell
          float occupancy = unmaskedHitsAlongV[vCell] / unmaskedCellsAlongV[vCell];
          // Mask residual hot vCell
          if (occupancy > maxOccupancyV) {
            for (auto uCell = 0; uCell < 250; uCell++)
              maskedPixelsPar->maskSinglePixel(id.getID(),  uCell * 768 + vCell);

            B2RESULT("Masking complete vCell=" << vCell << " on sensor " << id);
          }
        }
      }
    }
  }

  // After the masking is done, we compute the fraction of
  // masked pixels per sensorID

  for (auto elem : maskedPixelsPar->getMaskedPixelMap()) {
    auto id = elem.first;
    auto singles = elem.second;
    B2RESULT("SensorID " << VxdID(id) << " has fraction of masked pixels of " << (float)singles.size() / (768 * 250));
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
