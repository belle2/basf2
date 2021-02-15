/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDAnalyticGainCalibrationAlgorithm.h>
#include <pxd/calibration/PXDCalibrationUtilities.h>
#include <pxd/dbobjects/PXDGainMapPar.h>

#include <boost/format.hpp>

//ROOT
#include <TH2F.h>

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::PXD;


// Anonymous namespace for data objects used by PXDAnalyticGainCalibrationAlgorithm class
namespace {

  /** Signal in ADU of collected clusters */
  int m_signal;
  /** Estimated signal in ADU of collected clusters */
  float m_estimated;
  /** Run number to be stored in dbtree */
  int m_run;
  /** Experiment number to be stored in dbtree */
  int m_exp;
}


PXDAnalyticGainCalibrationAlgorithm::PXDAnalyticGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDPerformanceCollector"),
  minClusters(1000), safetyFactor(2.0), forceContinue(false), strategy(0), useChargeRatioHistogram(true), correctForward(false)
{
  setDescription(
    " -------------------------- PXDAnalyticGainCalibrationAlgorithm ---------------------------------\n"
    "                                                                                         \n"
    "  Algorithm for estimating pxd gains (conversion factor from charge to ADU)              \n"
    " ----------------------------------------------------------------------------------------\n"
  );
}


CalibrationAlgorithm::EResult PXDAnalyticGainCalibrationAlgorithm::calibrate()
{

  // Get counter histograms
  auto cluster_counter = getObjectPtr<TH1I>("PXDClusterCounter");

  // Extract number of sensors from counter histograms
  auto nSensors = getNumberOfSensors(cluster_counter);

  // Extract the number of grid bins from counter histograms
  unsigned short nBinsU = 0;
  unsigned short nBinsV = 0;
  getNumberOfBins(cluster_counter, nBinsU, nBinsV);

  // Check that we have collected enough Data
  if (cluster_counter->GetEntries() < int(safetyFactor * minClusters * nSensors * nBinsU * nBinsV)) {
    if (not forceContinue) {
      B2WARNING("Not enough Data: Only " <<  cluster_counter->GetEntries() << " hits were collected but " << int(
                  safetyFactor * minClusters *
                  nSensors * nBinsU * nBinsV) << " needed!");
      return c_NotEnoughData;
    } else {
      B2WARNING("Continue despite low statistics: Only " <<  cluster_counter->GetEntries() << " hits were collected but" << int(
                  safetyFactor * minClusters *
                  nSensors * nBinsU * nBinsV) << " would be desirable!");
    }
  }

  B2INFO("Start calibration using a " << nBinsU << "x" << nBinsV << " grid per sensor.");
  B2INFO("Number of collected clusters is " << cluster_counter->GetEntries());

  // This is the PXD gain correction payload for conditions DB
  PXDGainMapPar* gainMapPar = new PXDGainMapPar(nBinsU, nBinsV);
  set<VxdID> pxdSensors;

  // Loop over all bins of input histo
  for (auto histoBin = 1; histoBin <= cluster_counter->GetXaxis()->GetNbins(); histoBin++) {
    // The bin label contains the vxdid, uBin and vBin
    string label = cluster_counter->GetXaxis()->GetBinLabel(histoBin);

    // Parse label string format to read sensorID, uBin and vBin
    istringstream  stream(label);
    string token;
    getline(stream, token, '_');
    VxdID sensorID(token);

    getline(stream, token, '_');
    unsigned short uBin = std::stoi(token);

    getline(stream, token, '_');
    unsigned short vBin = std::stoi(token);

    // Read back the counters for number of collected clusters
    int numberOfHits = cluster_counter->GetBinContent(histoBin);

    // Only perform estimation, when enough data is available
    if (numberOfHits >= minClusters) {

      double gain = 1.0;
      // Estimate the gain on a certain part of PXD
      if (useChargeRatioHistogram) {
        auto hClusterChargeRatio = getObjectPtr<TH2F>("PXDClusterChargeRatio");
        TH1D* hRatios = hClusterChargeRatio->ProjectionY("proj", histoBin, histoBin);
        gain = EstimateGain(sensorID, uBin, vBin, hRatios);
      } else {
        gain = EstimateGain(sensorID, uBin, vBin);
      }
      // Store the gain
      gainMapPar->setContent(sensorID.getID(), uBin, vBin, gain);
    } else {
      B2WARNING(label << ": Number of hits is too small (" << numberOfHits << " < " << minClusters <<
                "). Use default gain.");
      gainMapPar->setContent(sensorID.getID(), uBin, vBin, 1.0);
    }
    pxdSensors.insert(sensorID);
  }

  // Post processing of gain map. It is possible that the gain
  // computation failed on some parts. Here, we replace default
  // values (1.0) by local averages of neighboring sensor parts.

  for (const auto& sensorID : pxdSensors) {

    // Special treatement for the last 2 vBin as Bhabha 2-track events
    // have no enough statistics there if nBinsV = 6
    if (correctForward && sensorID.getSensorNumber() == 1)
      for (unsigned short uBin = 0; uBin < nBinsU; ++uBin) {
        // Search for a vaid gain along v
        double gainForwardRegion = 1.0;
        unsigned short vBinToCheck = nBinsV - 1;
        for (unsigned short vBinGood = nBinsV - 2; vBinGood >= 1; --vBinGood) {
          auto temp = gainMapPar->getContent(sensorID.getID(), uBin, vBinGood);
          if (temp != 1.0) {
            gainForwardRegion = temp;
            vBinToCheck = vBinGood + 1;
            break;
          }
        }
        // loop part of the forward regions and check values
        if (gainForwardRegion != 1.0)
          for (unsigned short vBin = nBinsV - 1; vBin >= vBinToCheck; --vBin) {
            auto gain = gainMapPar->getContent(sensorID.getID(), uBin, vBin);
            if (gain == 1.0) {
              gainMapPar->setContent(sensorID.getID(), uBin, vBin, gainForwardRegion);
              B2RESULT("Gain calibration on sensor=" << sensorID << ", vBin=" << vBin << " uBin " << uBin <<
                       ": Replace default gain with that from the closest vBin with non default value "
                       << gainForwardRegion);
            }
          }
      }

    // general value check
    for (unsigned short vBin = 0; vBin < nBinsV; ++vBin) {

      float meanGain = 0;
      unsigned short nGood = 0;
      unsigned short nBad = 0;
      for (unsigned short uBin = 0; uBin < nBinsU; ++uBin) {
        auto gain = gainMapPar->getContent(sensorID.getID(), uBin, vBin);
        // Filter default gains
        if (gain != 1.0) {
          nGood += 1;
          meanGain += gain;
        } else {
          nBad += 1;
        }
      }
      B2RESULT("Gain calibration on sensor=" << sensorID << " and vBin=" << vBin << " was successful on " << nGood << "/" << nBinsU <<
               " uBins.");

      // Check if we can repair bad calibrations with a local avarage
      if (nGood > 0 && nBad > 0) {
        meanGain /= nGood;
        for (unsigned short uBin = 0; uBin < nBinsU; ++uBin) {
          auto gain = gainMapPar->getContent(sensorID.getID(), uBin, vBin);
          if (gain == 1.0) {
            gainMapPar->setContent(sensorID.getID(), uBin, vBin, meanGain);
            B2RESULT("Gain calibration on sensor=" << sensorID << ", vBin=" << vBin << " uBin " << uBin <<
                     ": Replace default gain with average "
                     << meanGain << " of uBins with non-default gains.");
          }
        }
      }
    }
  }


  // Save the gain map to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(gainMapPar, "PXDGainMapPar");

  B2INFO("PXD Gain Calibration Successful");
  return c_OK;
}


double PXDAnalyticGainCalibrationAlgorithm::EstimateGain(VxdID sensorID, unsigned short uBin, unsigned short vBin, TH1* hist)
{
  double gain = 1.0; // default gain

  // function pointers for different strategy
  double (*estimateGainFromHist)(TH1*) = &CalculateMedian;
  double (*estimateGainFromVec)(std::vector<double>&) = &CalculateMedian;
  if (strategy == 0) {
    //estimateGainFromVec = &CalculateMedian;
    //estimateGainFromHist = &CalculateMedian;
  } else if (strategy == 1) {
    estimateGainFromVec = &FitLandau;
    estimateGainFromHist = &FitLandau;
  } else {
    B2FATAL("strategy unavailable, use 0 for medians or 1 for landau fit!");
  }

  // Do estimation
  if (hist) { // estimate gain from existing histogram
    gain = estimateGainFromHist(hist);
  } else { // estimate from TTree
    // Construct a tree name for requested part of PXD
    auto layerNumber = sensorID.getLayerNumber();
    auto ladderNumber = sensorID.getLadderNumber();
    auto sensorNumber = sensorID.getSensorNumber();
    const string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);
    // Vector with ratios (cluster charge to its estimation)
    vector<double> ratios;

    auto tree = getObjectPtr<TTree>(treename);
    tree->SetBranchAddress("signal", &m_signal);
    tree->SetBranchAddress("estimated", &m_estimated);

    // Loop over tree
    const auto nEntries_MC = tree->GetEntries();
    for (int i = 0; i < nEntries_MC; ++i) {
      tree->GetEntry(i);
      ratios.push_back(double(m_signal) / m_estimated);
    }
    gain = estimateGainFromVec(ratios);
  }

  // check if gain makes sense
  if (gain <= 0.0) {
    B2WARNING("Retrieved negative median/MPV for sensor=" << sensorID << " uBin=" << uBin << " vBin=" << vBin <<
              ". Set gain to default value (=1.0) as well.");
    return 1.0;
  }

  // calculate and return the absolute gain
  double gainFromDB = GetCurrentGainFromDB(sensorID, uBin, vBin);
  B2DEBUG(10, "Gain from db used in PXDDigitizer is " << gainFromDB);
  B2DEBUG(10, "New gain correction derived is " << gain);
  B2DEBUG(10, "The total gain we should return is " << gain * gainFromDB);

  return gain * gainFromDB;
}

double PXDAnalyticGainCalibrationAlgorithm::GetCurrentGainFromDB(VxdID sensorID, unsigned short uBin, unsigned short vBin)
{
  // Read back db payloads
  PXDGainMapPar* gainMapPtr = nullptr;

  auto dbtree = getObjectPtr<TTree>("dbtree");
  dbtree->SetBranchAddress("run", &m_run);
  dbtree->SetBranchAddress("exp", &m_exp);
  dbtree->SetBranchAddress("gainMap", &gainMapPtr);

  // Compute running average of gains from db
  double sum = 0;
  int counter = 0;

  // Loop over dbtree
  const auto nEntries = dbtree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    dbtree->GetEntry(i);
    sum += gainMapPtr->getContent(sensorID.getID(), uBin, vBin);
    counter += 1;
  }
  delete gainMapPtr;
  gainMapPtr = nullptr;

  return sum / counter;
}

bool PXDAnalyticGainCalibrationAlgorithm::isBoundaryRequired(const Calibration::ExpRun& /*currentRun*/)
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
