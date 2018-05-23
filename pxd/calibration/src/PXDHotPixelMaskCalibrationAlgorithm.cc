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
#include <pxd/dbobjects/PXDDeadPixelPar.h>
#include <pxd/dbobjects/PXDOccupancyInfoPar.h>

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
  forceContinueMasking(true), minEvents(10000), minHits(20), pixelMultiplier(10), maskDrains(false), minHitsDrain(200),
  drainMultiplier(10), maskRows(false), minHitsRow(200), rowMultiplier(10)
{
  setDescription(
    " -------------------------- PXDHotPixelMak Calibration Algorithm ------------------------\n"
    "                                                                                         \n"
    "  Algorithm which masks hot pixels with too large occupancy and dead pixels w/o no hits. \n"
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

  // This is the occupancy info payload for conditions DB
  PXDOccupancyInfoPar* occupancyInfoPar = new PXDOccupancyInfoPar();

  // This is the dead payload for conditions DB
  PXDDeadPixelPar* deadPixelsPar = new PXDDeadPixelPar();

  // This is the masking payload for conditions DB
  PXDMaskedPixelPar* maskedPixelsPar = new PXDMaskedPixelPar();

  // Remember the number of events, helps to judge the reliability of calibrations.
  occupancyInfoPar->setNumberOfEvents(nevents);

  // Loop over all sensor from collector
  auto collector_pxdhitcounts = getObjectPtr<TH1I>("PXDHitCounts");
  for (auto sensBin = 1; sensBin <= collector_pxdhitcounts->GetXaxis()->GetNbins(); sensBin++) {
    // The bin label is assumed to be a string representation of VxdID
    string sensorDescr =  collector_pxdhitcounts->GetXaxis()->GetBinLabel(sensBin);
    VxdID id(sensorDescr);

    // Number of collected hits per sensor
    int numberOfHits = collector_pxdhitcounts->GetBinContent(sensBin);

    // Compute mean occupancy before masking
    float meanOccupancy = (float)numberOfHits / nevents;
    occupancyInfoPar->getRawOccupancyMap()[id.getID()] = meanOccupancy;

    // FIXME remove this before merging into master
    B2RESULT("Raw occupancy sensor=" << id << " is " << meanOccupancy);

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

    // FIXME: this is not really the number i want to store
    occupancyInfoPar->getOccupancyMap()[id.getID()] =  medianNumberOfHits / nevents;

    // Dead pixel masking

    // Mask sensor in case we see no hits despite having some sizebale number of events
    if (numberOfHits == 0 && nevents > minEvents)  {
      deadPixelsPar->maskSensor(id.getID());
    }

    // It is easier to find dead drains and rows than pixels.
    // Forget about dead pixel masking if we have not enough statistics to even
    // mask on drain or row level
    if (medianNumberOfHits >= minHitsDrain || medianNumberOfHits >= minHitsRow) {

      // Bookkeeping for masking of drains and rows
      vector<int> hitsAlongRow(c_nVCells, 0);
      vector<int> hitsAlongDrain(c_nDrains, 0);

      // Accumulate hits along drains and rows
      for (auto bin = 1; bin <= nBins; bin++) {
        // Find the current pixel cell
        int pixID = bin - 1;
        int uCell = pixID / c_nVCells;
        int vCell = pixID % c_nVCells;
        int drainID = uCell * 4 + vCell % 4;
        int nhits = collector_pxdhitmap->GetBinContent(bin);
        hitsAlongDrain[drainID] += nhits;
        hitsAlongRow[vCell] += nhits;
      }

      // Dead row masking
      if (medianNumberOfHits >= minHitsRow) {
        for (auto vCell = 0; vCell < c_nVCells; vCell++) {
          // Get number of hits per row
          int nhits = hitsAlongRow[vCell];
          // Mask dead row
          if (nhits == 0) {
            deadPixelsPar->maskRow(id.getID(), vCell);
            B2RESULT("Dead row with vCell=" << vCell << " on sensor " << id);
          }
        }
      }

      // Dead drain masking
      if (medianNumberOfHits >= minHitsDrain) {
        for (auto drainID = 0; drainID < c_nDrains; drainID++) {
          // Compute number of hits per drain
          int nhits = hitsAlongDrain[drainID];
          // Mask dead drain
          if (nhits == 0) {
            deadPixelsPar->maskDrain(id.getID(), drainID);
            B2RESULT("Dead drain line at drainID=" << drainID << " on sensor " << id);
          }
        }
      }

      // Dead pixel masking
      if (medianNumberOfHits >= minHits) {
        for (auto bin = 1; bin <= nBins; bin++) {
          // First, we mask single pixels exceeding hit threshold
          int nhits = collector_pxdhitmap->GetBinContent(bin);
          // Mask dead pixel
          if (nhits == 0) {
            // Find the current pixel cell
            int pixID = bin - 1;
            int uCell = pixID / c_nVCells;
            int vCell = pixID % c_nVCells;
            // This pixel is dead, we have to mask it
            deadPixelsPar->maskSinglePixel(id.getID(), pixID);
            B2RESULT("Dead single pixel with ucell=" << uCell << ", vcell=" << vCell << " on sensor " << id);
          }
        }
      }
    }

    // Hot pixel masking

    // Bookkeeping for masking hot drains
    vector<float> unmaskedHitsAlongDrain(c_nDrains, 0);
    vector<int> unmaskedCellsAlongDrain(c_nDrains, 0);

    // Bookkeeping for maskign hot rows
    vector<float> unmaskedHitsAlongRow(c_nVCells, 0);
    vector<int> unmaskedCellsAlongRow(c_nVCells, 0);

    // Mask all single pixels exceeding medianNumberOfHits x multiplier
    double pixelHitThr = pixelMultiplier * medianNumberOfHits;
    B2RESULT("Pixel hit threshold is "  << pixelHitThr  << " for sensor " << id);

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

      if (nhits > pixelHitThr) {
        // This pixel is hot, we have to mask it
        maskedPixelsPar->maskSinglePixel(id.getID(), pixID);
        masked = true;
        B2RESULT("Masking single pixel with ucell=" << uCell << ", vcell=" << vCell << " on sensor " << id);
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
        if (unmaskedCellsAlongDrain[drainID] > 0) {
          // Compute average number of hits per drain
          float nhits = unmaskedHitsAlongDrain[drainID] / unmaskedCellsAlongDrain[drainID];
          // Mask residual hot drain
          if (nhits > drainHitThr) {
            for (auto iGate = 0; iGate < 192; iGate++) {
              int uCell = drainID / 4;
              int vCell = drainID % 4 + iGate * 4;
              maskedPixelsPar->maskSinglePixel(id.getID(),  uCell * c_nVCells + vCell);
            }
            B2RESULT("Masking drain line with drainID=" << drainID << " on sensor " << id);
          }
        }
      }
    }

    if (maskRows) {
      double rowHitThr = rowMultiplier * medianNumberOfHits;
      B2RESULT("Row hit threshold is "  << rowHitThr << " for sensor " << id);

      for (auto vCell = 0; vCell < c_nVCells; vCell++) {
        if (unmaskedCellsAlongRow[vCell] > 0) {
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
  saveCalibration(deadPixelsPar, "PXDDeadPixelPar");
  saveCalibration(occupancyInfoPar, "PXDOccupancyInfoPar");

  B2INFO("PXDHotPixelMask Calibration Successful");
  return c_OK;
}
