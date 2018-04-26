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
#include "TMath.h"

using namespace std;
using boost::format;
using namespace Belle2;


PXDHotPixelMaskCalibrationAlgorithm::PXDHotPixelMaskCalibrationAlgorithm(): CalibrationAlgorithm("PXDHotPixelMaskCollector"),
  forceContinueMasking(true), minEvents(10000), minHits(5), pixelMultiplier(10), maskDrains(false), minHitsDrain(50),
  drainMultiplier(10), maskRows(false), minHitsRow(50), rowMultiplier(10)
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
    if (not forceContinueMasking) {
      B2INFO("Not enough data: Only " << nevents << " events were collected!");
      return c_NotEnoughData;
    } else {
      B2WARNING("Not enough data: Only " << nevents << " events were collected! The masking continous but the mask may be empty.");
    }
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

    // Compute the median number of hits to define a robust baseline for judging a channel fires too often
    int nBins = collector_pxdhitmap->GetXaxis()->GetNbins();
    double prob = 0.5;
    vector<double> hitVec(nBins);

    for (auto bin = 1; bin <= nBins; bin++) {
      hitVec[bin - 1] = (double) collector_pxdhitmap->GetBinContent(bin);
    }
    double medianNumberOfHits;
    TMath::Quantiles(nBins, 1, &hitVec[0], &medianNumberOfHits, &prob, kFALSE);

    // We get in trouble when the median is zero
    if (medianNumberOfHits <= 0) medianNumberOfHits = 1;
    B2RESULT("Median of occupancy "  << medianNumberOfHits / nevents << " for sensor " << id);

    // Mask all single pixels exceeding medianNumberOfHits x multiplier
    double pixelHitThr = pixelMultiplier * medianNumberOfHits;
    B2RESULT("Pixel hit threshold is "  << pixelHitThr  << " for sensor " << id);

    // Bookkeeping for masking drains
    vector<float> unmaskedHitsAlongDrain(c_nDrains, 0);
    vector<int> unmaskedCellsAlongDrain(c_nDrains, 0);

    // Bookkeeping for maskign rows
    vector<float> unmaskedHitsAlongRow(c_nVCells, 0);
    vector<int> unmaskedCellsAlongRow(c_nVCells, 0);

    // Mask all hot pixel for this sensor
    for (auto bin = 1; bin <= nBins; bin++) {
      // Find the current pixel cell
      int pixID = bin - 1;
      int uCell = pixID / c_nVCells;
      int vCell = pixID % c_nVCells;
      int drainID = uCell * 4 + vCell % 4;

      // First, we mask single pixels exceeding hit threshold
      float nhits = collector_pxdhitmap->GetBinContent(bin);
      bool masked = false;
      if (nhits > minHits) {
        if (nhits > pixelHitThr) {
          // This pixel is hot, we have to mask it
          maskedPixelsPar->maskSinglePixel(id.getID(), pixID);
          masked = true;
          B2RESULT("Masking single pixel with ucell=" << uCell << ", vcell=" << vCell << " on sensor " << id);
        }
      }
      // Then we accumulate hits along u and v direction for unmasked
      // pixels
      if (not masked) {
        ++unmaskedCellsAlongDrain[drainID];
        unmaskedHitsAlongDrain[drainID] += nhits;
        ++unmaskedCellsAlongRow[vCell];
        unmaskedHitsAlongRow[vCell] += nhits;
      }
    }

    if (maskDrains) {
      double drainHitThr = drainMultiplier * medianNumberOfHits;
      B2RESULT("Drain hit threshold is "  << drainHitThr << " for sensor " << id);

      for (auto drainID = 0; drainID < c_nDrains; drainID++) {
        if (unmaskedHitsAlongDrain[drainID] > minHitsDrain && unmaskedCellsAlongDrain[drainID] > 0) {
          // Compute average number of hits per drain
          float nhits = unmaskedHitsAlongDrain[drainID] / unmaskedCellsAlongDrain[drainID];
          // Mask residual hot drain
          if (nhits > drainHitThr) {
            for (auto iGate = 0; iGate < 192; iGate++) {
              int uCell = drainID / 4;
              int vCell = drainID % 4 + iGate * 4;
              maskedPixelsPar->maskSinglePixel(id.getID(),  uCell * c_nVCells + vCell);
            }
            B2RESULT("Masking drain line at with drainID=" << drainID << " on sensor " << id);
          }
        }
      }
    }

    if (maskRows) {
      double rowHitThr = rowMultiplier * medianNumberOfHits;
      B2RESULT("Row hit threshold is "  << rowHitThr << " for sensor " << id);

      for (auto vCell = 0; vCell < c_nVCells; vCell++) {
        if (unmaskedHitsAlongRow[vCell] > minHitsRow && unmaskedCellsAlongRow[vCell] > 0) {
          // Compute average number of hits per row
          float nhits = unmaskedHitsAlongRow[vCell] / unmaskedCellsAlongRow[vCell];
          // Mask residual hot row
          if (nhits > rowHitThr) {
            for (auto uCell = 0; uCell < c_nUCells; uCell++)
              maskedPixelsPar->maskSinglePixel(id.getID(),  uCell * c_nVCells + vCell);

            B2RESULT("Masking complete row with vCell=" << vCell << " on sensor " << id);
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
    B2RESULT("SensorID " << VxdID(id) << " has fraction of masked pixels of " << (float)singles.size() / (c_nVCells * c_nUCells));
  }

  // Save the hot pixel mask to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(maskedPixelsPar, "PXDMaskedPixelPar");

  B2INFO("PXDHotPixelMask Calibration Successful");
  return c_OK;
}
