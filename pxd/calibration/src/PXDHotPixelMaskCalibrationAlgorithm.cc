/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDHotPixelMaskCalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDMaskedPixelPar.h>
#include <pxd/dbobjects/PXDDeadPixelPar.h>
#include <pxd/dbobjects/PXDOccupancyInfoPar.h>

#include <boost/format.hpp>
#include <string>
#include <vector>
#include "TH1I.h"
#include "TMath.h"

using namespace std;
using boost::format;
using namespace Belle2;


PXDHotPixelMaskCalibrationAlgorithm::PXDHotPixelMaskCalibrationAlgorithm(): CalibrationAlgorithm("PXDHotPixelMaskCollector"),
  forceContinueMasking(false), minEvents(10000), minHits(20), pixelMultiplier(7), maskDrains(true),
  drainMultiplier(7), maskRows(true),  rowMultiplier(7)
{
  setDescription(
    " -------------------------- PXDHotPixelMak Calibration Algorithm ------------------------\n"
    "                                                                                         \n"
    "  Algorithm which masks hot pixels with too large occupancy and dead pixels w/o no hits. \n"
    " ----------------------------------------------------------------------------------------\n"
  );
}

void PXDHotPixelMaskCalibrationAlgorithm::createDebugHistogram()
{
  // Run list of current IoV and all runs to be calibrated
  auto expRuns = getRunList();
  auto expRunsAll = getRunListFromAllData();

  // Save the current directory to change back later
  TDirectory* currentDir = gDirectory;

  // Create TFile if not exist
  if (!m_file) {
    std::string fileName = (this->getPrefix()) + "debug.root";
    B2INFO("Creating file " << fileName);
    m_file = std::make_shared<TFile>(fileName.c_str(), "RECREATE");
  }

  //m_file->cd();
  string iov_str = str(format("E%1%_R%2%_E%3%_R%4%") % expRuns.front().first
                       % expRuns.front().second % expRuns.back().first % expRuns.back().second);
  m_file->mkdir(iov_str.c_str());
  m_file->cd(iov_str.c_str());

  // Collector info
  auto collector_pxdhits = getObjectPtr<TH1I>("PXDHits");
  auto collector_pxdhitcounts = getObjectPtr<TH1I>("PXDHitCounts");

  auto nevents = collector_pxdhits->GetEntries();

  for (auto sensBin = 1; sensBin <= collector_pxdhitcounts->GetXaxis()->GetNbins(); sensBin++) {
    string sensorDescr =  collector_pxdhitcounts->GetXaxis()->GetBinLabel(sensBin);
    VxdID id(sensorDescr);

    // Get hitmap from collector
    string name = str(format("PXD_%1%_PixelHitmap") % id.getID());
    auto collector_pxdhitmap =  getObjectPtr<TH1I>(name.c_str());
    TH1I* debugHisto = new TH1I(*collector_pxdhitmap);
    // Set overflow to total number of events
    debugHisto->SetBinContent(debugHisto->GetNbinsX() + 1, nevents);
    debugHisto->Write();
    delete debugHisto;
  }

  // Close TFile
  if (expRuns.back() == expRunsAll.back()) {
    B2INFO("Reached Final ExpRun: (" << expRuns.back().first << ", " << expRuns.back().second << ")");
    m_file->Close();

  }
  currentDir->cd();
}

CalibrationAlgorithm::EResult PXDHotPixelMaskCalibrationAlgorithm::calibrate()
{

  auto collector_pxdhits = getObjectPtr<TH1I>("PXDHits");
  auto collector_pxdhitcounts = getObjectPtr<TH1I>("PXDHitCounts");

  // We should have some minimum number of events
  auto nevents = collector_pxdhits->GetEntries();
  if (nevents < minEvents) {
    if (not forceContinueMasking) {
      B2INFO("Not enough data: Only " << nevents << " events were collected!");
      return c_NotEnoughData;
    } else {
      B2WARNING("Not enough data: Only " << nevents << " events were collected! The masking continous but the mask may be empty.");
    }
  }

  B2RESULT("Found total of " << nevents << " events in collected data.");

  // Get the total number of PXD hits and sensors
  unsigned long long int nPXDHits = 0;
  int nPXDSensors = 0;
  for (auto sensBin = 1; sensBin <= collector_pxdhitcounts->GetXaxis()->GetNbins(); sensBin++) {
    // The bin label is assumed to be a string representation of VxdID
    string sensorDescr =  collector_pxdhitcounts->GetXaxis()->GetBinLabel(sensBin);
    VxdID id(sensorDescr);
    //Increment number of sensors
    nPXDSensors += 1;
    // Increment number of  of collected hits
    unsigned long long int nSensorHits = collector_pxdhitcounts->GetBinContent(sensBin);
    nPXDHits += nSensorHits;

    B2RESULT("Number of hits for sensor sensor=" << id << " is " << nSensorHits);
  }

  // We should have enough hits in the PXD before we decide a single sensor is dead
  unsigned long long int minPXDHits =  minHits * nPXDSensors * c_nUCells * c_nVCells;
  if (nPXDHits < minPXDHits) {
    if (not forceContinueMasking) {
      B2INFO("Not enough data: Only " << nPXDHits << " raw hits were collected!");
      return c_NotEnoughData;
    } else {
      B2WARNING("Not enough data: Only " << nPXDHits << " raw hits were collected! The masking continous but the mask may be empty.");
    }
  }

  B2RESULT("Found total of " << nPXDHits  << " raw hits in collected data.");

  // Check that the median number of hits is large enough
  for (auto sensBin = 1; sensBin <= collector_pxdhitcounts->GetXaxis()->GetNbins(); sensBin++) {
    // The bin label is assumed to be a string representation of VxdID
    string sensorDescr =  collector_pxdhitcounts->GetXaxis()->GetBinLabel(sensBin);
    VxdID id(sensorDescr);

    m_medianOfHitsMap[id] = 0.0;

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
    if (medianNumberOfHits <= 0) {
      B2WARNING("Median number of hits per senor is smaller <1. Raise median to 1 instead.");
      medianNumberOfHits = 1;
    } else {
      B2RESULT("Median of hits is "  << medianNumberOfHits  << " for sensor " << id);
    }

    // Keep the median of later use
    m_medianOfHitsMap[id] = medianNumberOfHits;

    // Check median number of hits is large enough
    if (medianNumberOfHits < minHits) {
      if (not forceContinueMasking) {
        B2INFO("Not enough data: Median number of his is only  " << medianNumberOfHits << "!");
        return c_NotEnoughData;
      } else {
        B2WARNING("Not enough data: Median number of hits is only  " << medianNumberOfHits <<
                  "! The masking continous but the mask may be empty.");
      }
    }
  }

  // This is the occupancy info payload for conditions DB
  PXDOccupancyInfoPar* occupancyInfoPar = new PXDOccupancyInfoPar();

  // This is the dead pixels payload for conditions DB
  PXDDeadPixelPar* deadPixelsPar = new PXDDeadPixelPar();

  // This is the hot pixel masking payload for conditions DB
  PXDMaskedPixelPar* maskedPixelsPar = new PXDMaskedPixelPar();

  // Remember the number of events, helps to judge the reliability of calibrations.
  occupancyInfoPar->setNumberOfEvents(nevents);

  // Compute the masks for all sensors
  for (auto sensBin = 1; sensBin <= collector_pxdhitcounts->GetXaxis()->GetNbins(); sensBin++) {
    // The bin label is assumed to be a string representation of VxdID
    string sensorDescr =  collector_pxdhitcounts->GetXaxis()->GetBinLabel(sensBin);
    VxdID id(sensorDescr);

    // If we reach here, a sensor with no hits is deemed dead
    int nSensorHits = collector_pxdhitcounts->GetBinContent(sensBin);
    if (nSensorHits == 0)  {
      deadPixelsPar->maskSensor(id.getID());
      continue;
    }

    // Get hitmap from collector
    string name = str(format("PXD_%1%_PixelHitmap") % id.getID());
    auto collector_pxdhitmap =  getObjectPtr<TH1I>(name.c_str());
    if (collector_pxdhitmap == nullptr) {
      B2WARNING("Cannot find PixelHitmap although there should be hits. This is strange!");
      continue;
    }

    double medianNumberOfHits = m_medianOfHitsMap[id];
    int nBins = collector_pxdhitmap->GetXaxis()->GetNbins();

    // Dead pixel masking
    if (medianNumberOfHits >= minHits) {

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
      B2INFO("Entering masking of dead rows ...");
      for (auto vCell = 0; vCell < c_nVCells; vCell++) {
        // Get number of hits per row
        int nhits = hitsAlongRow[vCell];
        // Mask dead row
        if (nhits == 0) {
          deadPixelsPar->maskRow(id.getID(), vCell);
          B2RESULT("Dead row with vCell=" << vCell << " on sensor " << id);
        }
      }

      // Dead drain masking
      B2INFO("Entering masking of dead drains  ...");
      for (auto drainID = 0; drainID < c_nDrains; drainID++) {
        // Get number of hits per drain
        int nhits = hitsAlongDrain[drainID];
        // Mask dead drain
        if (nhits == 0) {
          deadPixelsPar->maskDrain(id.getID(), drainID);
          B2RESULT("Dead drain line at drainID=" << drainID << " on sensor " << id);
        }
      }

      // Dead pixel masking
      B2INFO("Entering masking of single dead pixels  ...");
      for (auto bin = 1; bin <= nBins; bin++) {
        // First, we mask single pixels exceeding hit threshold
        int nhits = collector_pxdhitmap->GetBinContent(bin);
        int pixID = bin - 1;
        int uCell = pixID / c_nVCells;
        int vCell = pixID % c_nVCells;
        int drainID = uCell * 4 + vCell % 4;
        // Mask dead pixel
        if (nhits == 0 && !deadPixelsPar->isDeadRow(id.getID(), vCell) && !deadPixelsPar->isDeadDrain(id.getID(), drainID)) {
          // This pixel is dead, we have to mask it
          deadPixelsPar->maskSinglePixel(id.getID(), pixID);
          B2RESULT("Dead single pixel with ucell=" << uCell << ", vcell=" << vCell << " on sensor " << id);
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

    // After the masking is done, we compute the average sensor occupancy after
    // hot pixel masking.

    // Count all unmasked hits
    int numberOfUnmaskedHits = 0;

    for (auto bin = 1; bin <= nBins; bin++) {
      // Find the current pixel cell
      int pixID = bin - 1;

      if (maskedPixelsPar->pixelOK(id.getID(), pixID)) {
        numberOfUnmaskedHits += collector_pxdhitmap->GetBinContent(bin);
      }
    }

    // Compute mean occupancy before masking
    float meanOccupancyAfterMasking = (float)numberOfUnmaskedHits / nevents / c_nVCells / c_nUCells;
    B2RESULT("Hotpixel filtered occupancy sensor=" << id << " is " << meanOccupancyAfterMasking);

    occupancyInfoPar->setOccupancy(id.getID(), meanOccupancyAfterMasking);
  }

  for (auto elem : maskedPixelsPar->getMaskedPixelMap()) {
    auto id = elem.first;
    auto singles = elem.second;
    B2RESULT("SensorID " << VxdID(id) << " has filtered occupancy of " << occupancyInfoPar->getOccupancy(id));
    B2RESULT("SensorID " << VxdID(id) << " has fraction of masked pixels of " << (float)singles.size() / (c_nVCells * c_nUCells));
  }

  // Save the hot pixel mask to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(maskedPixelsPar, "PXDMaskedPixelPar");
  saveCalibration(deadPixelsPar, "PXDDeadPixelPar");
  saveCalibration(occupancyInfoPar, "PXDOccupancyInfoPar");

  // Create debugging histo if we asked for it
  if (m_debugHisto) createDebugHistogram();

  B2INFO("PXDHotPixelMask Calibration Successful");
  return c_OK;
}
