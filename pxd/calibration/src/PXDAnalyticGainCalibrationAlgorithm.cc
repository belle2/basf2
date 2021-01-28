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
#include <pxd/dbobjects/PXDGainMapPar.h>

#include <string>
#include <algorithm>
#include <set>

#include <sstream>
#include <iostream>

#include <boost/format.hpp>

//ROOT
#include <TRandom.h>
#include <TH1.h>
#include <TF1.h>

using namespace std;
using boost::format;
using namespace Belle2;


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

  /** Helper function to extract number of bins along u side and v side from counter histogram labels. */
  void getNumberOfBins(const std::shared_ptr<TH1I>& histo_ptr, unsigned short& nBinsU, unsigned short& nBinsV)
  {
    set<unsigned short> uBinSet;
    set<unsigned short> vBinSet;

    // Loop over all bins of input histo
    for (auto histoBin = 1; histoBin <= histo_ptr->GetXaxis()->GetNbins(); histoBin++) {
      // The bin label contains the vxdid, uBin and vBin
      string label = histo_ptr->GetXaxis()->GetBinLabel(histoBin);

      // Parse label string format to read sensorID, uBin and vBin
      istringstream  stream(label);
      string token;
      getline(stream, token, '_');
      getline(stream, token, '_');
      unsigned short uBin = std::stoi(token);

      getline(stream, token, '_');
      unsigned short vBin = std::stoi(token);

      uBinSet.insert(uBin);
      vBinSet.insert(vBin);
    }

    if (uBinSet.empty() || vBinSet.empty()) {
      B2FATAL("Not able to determine the grid size. Something is wrong with collected data.");
    } else {
      nBinsU = *uBinSet.rbegin() + 1;
      nBinsV = *vBinSet.rbegin() + 1;
    }
  }

  /** Helper function to extract number of sensors from counter histogram labels. */
  unsigned short getNumberOfSensors(const std::shared_ptr<TH1I>& histo_ptr)
  {
    set<unsigned short> sensorSet;

    // Loop over all bins of input histo
    for (auto histoBin = 1; histoBin <= histo_ptr->GetXaxis()->GetNbins(); histoBin++) {
      // The bin label contains the vxdid, uBin and vBin
      string label = histo_ptr->GetXaxis()->GetBinLabel(histoBin);

      // Parse label string format to read sensorID, uBin and vBin
      istringstream  stream(label);
      string token;
      getline(stream, token, '_');
      VxdID sensorID(token);
      sensorSet.insert(sensorID.getID());
    }
    return sensorSet.size();
  }

}


PXDAnalyticGainCalibrationAlgorithm::PXDAnalyticGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDCDSTCollector"),

  minClusters(1000), safetyFactor(2.0), forceContinue(false), strategy(0)
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

      // Estimate the gain on a certain part of PXD
      auto gain = EstimateGain(sensorID, uBin, vBin);

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
            B2RESULT("Gain calibration on sensor=" << sensorID << ", vBin=" << vBin << " uBin " << uBin << ": Replace default gain wih average "
                     << meanGain);
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


double PXDAnalyticGainCalibrationAlgorithm::EstimateGain(VxdID sensorID, unsigned short uBin, unsigned short vBin)
{
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

  double gain = 1.0;
  if (strategy == 0) gain = CalculateMedian(ratios);
  else if (strategy == 1) gain = FitLandau(ratios);
  else {
    B2FATAL("strategy unavailable, use 0 for medians or 1 for landau fit!");
  }

  // check if gain makes sense
  if (gain <= 0.0) {
    B2WARNING("Retrieved negative median/MPV for sensor=" << sensorID << " uBin=" << uBin << " vBin=" << vBin <<
              ". Set gain to default value (=1.0) as well.");
    return 1.0;
  }


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


double PXDAnalyticGainCalibrationAlgorithm::CalculateMedian(vector<double>& signals)
{
  auto size = signals.size();

  if (size == 0) {
    return 0.0;  // Undefined, really.
  } else {
    sort(signals.begin(), signals.end());
    if (size % 2 == 0) {
      return (signals[size / 2 - 1] + signals[size / 2]) / 2;
    } else {
      return signals[size / 2];
    }
  }
}

double PXDAnalyticGainCalibrationAlgorithm::FitLandau(vector<double>& signals)
{
  auto size = signals.size();
  if (size == 0) return 0.0; // Undefined, really.

  // get max and min values of signal vector
  double max = *max_element(signals.begin(), signals.end());
  double min = *min_element(signals.begin(), signals.end());

  // create histogram to hold signals and fill it
  TH1D* hist_signals = new TH1D("", "", max - min, min, max);
  for (auto it = signals.begin(); it != signals.end(); ++it) {
    hist_signals->Fill(*it);
  }

  // create fit function
  TF1* landau = new TF1("landau", "TMath::Landau(x,[0],[1])*[2]", min, max);
  landau->SetParNames("MPV", "sigma", "scale");
  landau->SetParameters(1, 0.2, 1000);

  // do fit and get results
  Int_t status = hist_signals->Fit("landau", "Lq", "", min, max);
  double MPV = landau->GetParameter("MPV");

  // clean up
  delete hist_signals;
  delete landau;

  // check fit status
  if (status == 0) return MPV;
  else {
    B2WARNING("Fit failed!. using default value.");
    return 0.0;
  }
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
