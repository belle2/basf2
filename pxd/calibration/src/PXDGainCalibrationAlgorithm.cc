/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDGainCalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDGainMapPar.h>
#include <vxd/dataobjects/VxdID.h>

#include <string>
#include <algorithm>
#include <map>
#include <set>

#include <sstream>
#include <iostream>

#include <boost/format.hpp>
#include <cmath>


//ROOT
#include <TRandom.h>
#include <TH1I.h>

using namespace std;
using boost::format;
using namespace Belle2;


// Anonymous namespace for data objects used by PXDGainCalibrationAlgorithm class
namespace {

  std::shared_ptr<TTree> m_tree;

  /** Gain of collected clusters */
  float m_gain;
  /** Signal in ADU of collected clusters */
  int m_signal;
  /** Flag for MC data  */
  bool m_isMC;

  /** Helper function to extract number of gain corrections along u side and v side from histogram labels. */
  void getNumberOfGainBins(const std::shared_ptr<TH1I>& histo_ptr, unsigned short& nBinsU, unsigned short& nBinsV)
  {
    set<unsigned short> uBinSet;
    set<unsigned short> vBinSet;

    // Loop over all gain ids
    for (auto gainBin = 1; gainBin <= histo_ptr->GetXaxis()->GetNbins(); gainBin++) {
      // The bin label is almost the name of the tree containing the calibration data
      string label = histo_ptr->GetXaxis()->GetBinLabel(gainBin);

      // Parse label string format to read sensorID, gain uBin and gain vBin
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
      B2FATAL("Not able to determine the gain grid size. Something is wrong with collected data.");
    } else {
      nBinsU = *uBinSet.rbegin() + 1;
      nBinsV = *vBinSet.rbegin() + 1;
    }
  }
}


PXDGainCalibrationAlgorithm::PXDGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDGainCollector"),
  minClusters(1000), noiseSigma(1.0)
{
  setDescription(
    " -------------------------- PXDGainCalibrationAlgorithm ---------------------------------\n"
    "                                                                                         \n"
    "  Algorithm for estimating pxd gains (conversion factor from charge to ADU)              \n"
    " ----------------------------------------------------------------------------------------\n"
  );
}




CalibrationAlgorithm::EResult PXDGainCalibrationAlgorithm::calibrate()
{

  // Get counter histograms for MC and Data
  auto mc_counter = getObjectPtr<TH1I>("PXDMCCounter");
  auto data_counter = getObjectPtr<TH1I>("PXDDataCounter");

  // Extract the number of gain bins from counter histograms
  unsigned short nBinsU = 0;
  unsigned short nBinsV = 0;
  getNumberOfGainBins(data_counter, nBinsU, nBinsV);

  B2INFO("Start gain corrections using a " << nBinsU << "x" << nBinsV << " gain grid per sensor.");

  // This is the PXD gain correction payload for conditions DB
  PXDGainMapPar* gainMapPar = new PXDGainMapPar(nBinsU, nBinsV);

  // Loop over all gain ids
  for (auto gainBin = 1; gainBin <= data_counter->GetXaxis()->GetNbins(); gainBin++) {
    // The bin label is almost the name of the tree containing the calibration data
    string label = data_counter->GetXaxis()->GetBinLabel(gainBin);

    // Parse label string format to read sensorID, gain uBin and gain vBin
    istringstream  stream(label);
    string token;
    getline(stream, token, '_');
    VxdID sensorID(token);
    auto layerNumber = sensorID.getLayerNumber();
    auto ladderNumber = sensorID.getLadderNumber();
    auto sensorNumber = sensorID.getSensorNumber();

    getline(stream, token, '_');
    unsigned short uBin = std::stoi(token);

    getline(stream, token, '_');
    unsigned short vBin = std::stoi(token);

    // Read back the counters for number of collected clusters
    int numberOfDataHits = data_counter->GetBinContent(gainBin);
    int numberOfMCHits = mc_counter->GetBinContent(gainBin);

    // Only perform fitting, when enough data is available
    if (numberOfDataHits >= minClusters && numberOfMCHits >= minClusters) {

      string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);
      m_tree = getObjectPtr<TTree>(treename);
      m_tree->SetBranchAddress("gain", &m_gain);
      m_tree->SetBranchAddress("signal", &m_signal);
      m_tree->SetBranchAddress("isMC", &m_isMC);

      // Estimate the gain
      auto gain = EstimateGain();

      // Store the gain
      gainMapPar->setGainCorrection(sensorID.getID(), uBin, vBin, gain);
    } else {
      if (numberOfMCHits < minClusters) {
        B2WARNING(label << ": Number of mc hits too small for fitting (" << numberOfMCHits << " < " << minClusters <<
                  "). Use default gain=1.0");
      }
      if (numberOfDataHits < minClusters) {
        B2WARNING(label << ": Number of data hits too small for fitting (" << numberOfDataHits << " < " << minClusters <<
                  "). Use default gain=1.0");
      }
    }
  }

  // Save the gain map to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(gainMapPar, "PXDGainMapPar");

  B2INFO("PXD Gain Calibration Successful");
  return c_OK;
}


double PXDGainCalibrationAlgorithm::EstimateGain()
{
  vector<double> mc_signals;
  vector<double> data_signals;

  // Loop over the tree
  const auto nEntries = m_tree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    m_tree->GetEntry(i);

    double noise = gRandom->Gaus(0.0, noiseSigma);
    if (m_isMC) {
      mc_signals.push_back(m_signal + noise);
    } else {
      data_signals.push_back(m_signal + noise);
    }
  }

  auto dataMedian = CalculateMedianOfSignals(data_signals);
  auto mcMedian = CalculateMedianOfSignals(mc_signals);

  double gain =  dataMedian / mcMedian;
  return gain;
}


double PXDGainCalibrationAlgorithm::CalculateMedianOfSignals(vector<double>& signals)
{
  auto size = signals.size();

  if (size == 0) {
    return 0;  // Undefined, really.
  } else {
    sort(signals.begin(), signals.end());
    if (size % 2 == 0) {
      return (signals[size / 2 - 1] + signals[size / 2]) / 2;
    } else {
      return signals[size / 2];
    }
  }
}

