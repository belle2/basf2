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
#include <pxd/dbobjects/PXDClusterChargeMapPar.h>

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

  /** Gain of collected clusters */
  float m_gain;
  /** Signal in ADU of collected clusters */
  int m_signal;
  /** Flag for MC data  */
  bool m_isMC;

  /** Run number to be stored in dbtree */
  int m_run;
  /** Experiment number to be stored in dbtree */
  int m_exp;
  /** ChargeMap to be stored in dbtree */
  PXDClusterChargeMapPar m_chargeMap;
  /** GainMap to be stored in dbtree */
  PXDGainMapPar m_gainMap;

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


PXDGainCalibrationAlgorithm::PXDGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDGainCollector"),
  minClusters(1000), noiseSigma(1.0), safetyFactor(2.0)
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
  auto counter_MC = getObjectPtr<TH1I>("PXDMCCounter");

  // Make some consistency checks here to assure the Data and MC were collected for
  // the same geometry.

  // Extract number of sensors from counter histograms
  auto nSensors = getNumberOfSensors(counter_MC);

  // Extract the number of grid bins from counter histograms
  unsigned short nBinsU = 0;
  unsigned short nBinsV = 0;
  getNumberOfBins(counter_MC, nBinsU, nBinsV);

  // Check that we have collected enough MC
  if (counter_MC->GetEntries() < int(safetyFactor * minClusters * nSensors * nBinsU * nBinsV)) {
    B2INFO("Not enough MC: Only " <<  counter_MC->GetEntries() << " hits were collected but " << int(safetyFactor * minClusters *
           nSensors  * nBinsU * nBinsV) << " needed!");
    return c_NotEnoughData;
  }

  B2INFO("Start calibration using a " << nBinsU << "x" << nBinsV << " grid per sensor.");

  // This is the PXD gain correction payload for conditions DB
  PXDGainMapPar* gainMapPar = new PXDGainMapPar(nBinsU, nBinsV);

  // Loop over all bins of input histo
  for (auto histoBin = 1; histoBin <= counter_MC->GetXaxis()->GetNbins(); histoBin++) {
    // The bin label contains the vxdid, uBin and vBin
    string label = counter_MC->GetXaxis()->GetBinLabel(histoBin);

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
    int numberOfMCHits = counter_MC->GetBinContent(histoBin);

    // Only perform fitting, when enough data is available
    if (numberOfMCHits >= minClusters) {

      // Estimate the gain on a certain part of PXD
      auto gain = EstimateGain(sensorID, uBin, vBin);

      // Store the gain
      gainMapPar->setContent(sensorID.getID(), uBin, vBin, gain);
    } else {
      B2WARNING(label << ": Number of mc hits too small for fitting (" << numberOfMCHits << " < " << minClusters <<
                "). Use default gain.");
    }
  }

  // Save the gain map to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(gainMapPar, "PXDGainMapPar");

  B2INFO("PXD Gain Calibration Successful");
  return c_OK;
}


double PXDGainCalibrationAlgorithm::EstimateGain(VxdID sensorID, unsigned short uBin, unsigned short vBin)
{

  // Read back db payloads
  PXDGainMapPar* gainMapPtr = &m_gainMap;
  PXDClusterChargeMapPar* chargeMapPtr = &m_chargeMap;

  auto dbtree = getObjectPtr<TTree>("dbtree");
  dbtree->SetBranchAddress("run", &m_run);
  dbtree->SetBranchAddress("exp", &m_exp);
  dbtree->SetBranchAddress("chargeMap", &chargeMapPtr);
  dbtree->SetBranchAddress("gainMap", &gainMapPtr);

  // Loop over dbtree
  const auto nEntries = dbtree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    dbtree->GetEntry(i);
    B2INFO("Found dbtree entry with exp " << m_exp << " run " << m_run << " nbinsU " << m_chargeMap.getBinsU() << " nbinsV " <<
           m_chargeMap.getBinsV() << " content " << m_chargeMap.getContent(sensorID.getID(), uBin, vBin));
    B2INFO("Found dbtree entry with exp " << m_exp << " run " << m_run << " nbinsU " << m_gainMap.getBinsU() << " nbinsV " <<
           m_gainMap.getBinsV() << " content " << m_gainMap.getContent(sensorID.getID(), uBin, vBin));
  }

  // Construct a tree name for requested part of PXD
  auto layerNumber = sensorID.getLayerNumber();
  auto ladderNumber = sensorID.getLadderNumber();
  auto sensorNumber = sensorID.getSensorNumber();
  const string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);

  // Vector with cluster signals from collected mc
  vector<double> mc_signals;

  auto tree_MC = getObjectPtr<TTree>(treename);
  tree_MC->SetBranchAddress("gain", &m_gain);
  tree_MC->SetBranchAddress("signal", &m_signal);
  tree_MC->SetBranchAddress("isMC", &m_isMC);

  // Loop over tree_MC
  const auto nEntries_MC = tree_MC->GetEntries();
  for (int i = 0; i < nEntries_MC; ++i) {
    tree_MC->GetEntry(i);

    double noise = gRandom->Gaus(0.0, noiseSigma);
    if (m_isMC) {
      mc_signals.push_back(m_signal + noise);
    } else {
      B2WARNING("Found data cluster in mc tree. This is very fishy and points to a mistake in your CAF script.");
    }
  }

  auto dataMedian = m_chargeMap.getContent(sensorID.getID(), uBin, vBin);
  auto mcMedian = CalculateMedian(mc_signals);

  double gain =  dataMedian / mcMedian;
  return gain;
}


double PXDGainCalibrationAlgorithm::CalculateMedian(vector<double>& signals)
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

