/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDMedianChargeCalibrationAlgorithm.h>
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


// Anonymous namespace for data objects used by PXDMedianChargeCalibrationAlgorithm class
namespace {

  /** Gain of collected clusters */
  float m_gain;
  /** Signal in ADU of collected clusters */
  int m_signal;
  /** Flag for MC data  */
  bool m_isMC;

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


PXDMedianChargeCalibrationAlgorithm::PXDMedianChargeCalibrationAlgorithm():
  CalibrationAlgorithm("PXDGainCollector"),
  minClusters(1000), noiseSigma(1.0), safetyFactor(2.0)
{
  setDescription(
    " -------------------------- PXDMedianChargeCalibrationAlgorithm -------------------------------\n"
    "                                                                                               \n"
    "  Algorithm for estimating pxd median cluster charges for different position on sensor in ADU  \n"
    " ----------------------------------------------------------------------------------------------\n"
  );
}




CalibrationAlgorithm::EResult PXDMedianChargeCalibrationAlgorithm::calibrate()
{

  // Get counter histograms for MC and Data
  auto counter_MC = getObjectPtr<TH1I>("PXDMCCounter");
  auto counter_Data = getObjectPtr<TH1I>("PXDDataCounter");

  // Make some consistency checks here to assure the Data and MC were collected for
  // the same geometry.

  if (getNumberOfSensors(counter_Data) != getNumberOfSensors(counter_MC)) {
    B2FATAL("Number of sensors in Data and MC collector outputs are different.");
  }

  if (counter_Data->GetXaxis()->GetNbins() != counter_MC->GetXaxis()->GetNbins()) {
    B2FATAL("Number of grid bins in Data and MC collector outputs are different.");
  }

  // Extract number of sensors from counter histograms
  auto nSensors = getNumberOfSensors(counter_Data);

  // Extract the number of grid bins from counter histograms
  unsigned short nBinsU = 0;
  unsigned short nBinsV = 0;
  getNumberOfBins(counter_Data, nBinsU, nBinsV);

  // Check that we have collected enough Data
  if (counter_Data->GetEntries() < int(safetyFactor * minClusters * nSensors * nBinsU * nBinsV)) {
    B2INFO("Not enough Data: Only " <<  counter_Data->GetEntries() << " hits were collected but " << int(safetyFactor * minClusters *
           nSensors * nBinsU * nBinsV) << " needed!");
    return c_NotEnoughData;
  }

  B2INFO("Start calibration using a " << nBinsU << "x" << nBinsV << " grid per sensor.");

  // This is the PXD charge calibration payload for conditions DB
  PXDClusterChargeMapPar* chargeMapPar = new PXDClusterChargeMapPar(nBinsU, nBinsV);

  // Loop over all bins of input histo
  for (auto histoBin = 1; histoBin <= counter_Data->GetXaxis()->GetNbins(); histoBin++) {
    // The bin label contains the vxdid, uBin and vBin
    string label = counter_Data->GetXaxis()->GetBinLabel(histoBin);

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
    int numberOfDataHits = counter_Data->GetBinContent(histoBin);

    // Only perform fitting, when enough data is available
    if (numberOfDataHits >= minClusters) {

      // Compute the median cluster charge for the part of PXD
      auto medianCharge = EstimateMedianCharge(sensorID, uBin, vBin);

      // Store the charge
      chargeMapPar->setContent(sensorID.getID(), uBin, vBin, medianCharge);
    } else {
      B2WARNING(label << ": Number of data hits too small for fitting (" << numberOfDataHits << " < " << minClusters <<
                "). Use default value.");
    }
  }

  // Save the charge map to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(chargeMapPar, "PXDClusterChargeMapPar");

  B2INFO("PXD Cluster Charge Calibration Successful");
  return c_OK;
}


double PXDMedianChargeCalibrationAlgorithm::EstimateMedianCharge(VxdID sensorID, unsigned short uBin, unsigned short vBin)
{

  // Construct a tree name for requested part of PXD
  auto layerNumber = sensorID.getLayerNumber();
  auto ladderNumber = sensorID.getLadderNumber();
  auto sensorNumber = sensorID.getSensorNumber();
  const string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);

  // Vector with cluster signals from collected data
  vector<double> signals;

  // Fill data_signal vector from input data
  auto tree = getObjectPtr<TTree>(treename);
  tree->SetBranchAddress("gain", &m_gain);
  tree->SetBranchAddress("signal", &m_signal);
  tree->SetBranchAddress("isMC", &m_isMC);

  // Loop over tree
  const auto nEntries = tree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    double noise = gRandom->Gaus(0.0, noiseSigma);
    if (m_isMC) {
      //Found MC cluster in data tree. This can happen when using SingleIoV strategy
    } else {
      signals.push_back(m_signal + noise);
    }
  }

  return CalculateMedian(signals);
}


double PXDMedianChargeCalibrationAlgorithm::CalculateMedian(vector<double>& signals)
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

