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

#include <pxd/unpacking/PXDMappingLookup.h>

#include <string>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <functional>

#include <boost/format.hpp>
#include <cmath>


#include <TF1.h>
#include <TMath.h>
#include <TAxis.h>


using namespace std;
using boost::format;
using namespace Belle2;

PXDGainCalibrationAlgorithm::PXDGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDGainCollector"),
  minDigits(50)
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

  m_rootFile = new TFile("GainPlots.root", "recreate");
  m_rootFile->cd("");

  string treename = string("tree");
  auto tree = getObjectPtr<TTree>(treename);

  const auto nEntries = tree->GetEntries();
  B2INFO("Number of clusters is " << nEntries);

  tree->SetBranchAddress("sensorID", &m_sensorID);
  tree->SetBranchAddress("uCellID", &m_uCellID);
  tree->SetBranchAddress("vCellID", &m_vCellID);
  tree->SetBranchAddress("signal", &m_signal);
  tree->SetBranchAddress("isMC", &m_isMC);

  int nDCD = 4;
  int nSWB = 6;

  // List of sensors found in data
  vector< pair<VxdID, int > > sensorList;

  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    if (not m_isMC) {
      auto sensorID = VxdID(m_sensorID);
      auto it = std::find_if(sensorList.begin(), sensorList.end(),
      [&](const pair<VxdID, int>& element) { return element.first == sensorID ;});

      //Sensor exists in vector
      if (it != sensorList.end()) {
        //increment key in map
        it->second++;
      }
      //Sensor name does not exist
      else {
        //Not found, insert in vector
        sensorList.push_back(pair<VxdID, int>(VxdID(m_sensorID), 1));
      }
    }
  }

  // Loop over sensorList to select sensors for
  // next calibration step

  for (auto iter : sensorList) {
    auto sensorID = iter.first;
    auto counter = iter.second;

    for (int areaID = 0; areaID < nDCD * nSWB; areaID++) {
      int iDCD = areaID / 6 + 1;
      int iSWB = areaID % 6 + 1;

      auto residual = computeResidual(1.0, treename, VxdID(sensorID), areaID);
      B2INFO("Residual is " << std::to_string(residual));

      string histoname = str(format("signal_data_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % sensorID.getLayerNumber() %
                             sensorID.getLadderNumber() % sensorID.getSensorNumber() % iDCD % iSWB);
      TH1D dataHisto(histoname.c_str(), histoname.c_str(), 128, 0, 255);
      dataHisto.SetXTitle("cluster charge / ADU");
      dataHisto.SetYTitle("number of clusters");

      histoname = str(format("signal_mc_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % sensorID.getLayerNumber() % sensorID.getLadderNumber() %
                      sensorID.getSensorNumber() % iDCD % iSWB);
      TH1D mcHisto(histoname.c_str(), histoname.c_str(), 128, 0, 255);
      mcHisto.SetXTitle("cluster charge / ADU");
      mcHisto.SetYTitle("number of clusters");

      createValidationHistograms(dataHisto, mcHisto, 1.0, treename, VxdID(sensorID), areaID);

      m_rootFile->cd();
      dataHisto.Write();
      mcHisto.Write();
    }

  }

  // Save the cluster positions to database.
  //saveCalibration(positionEstimator, "PXDClusterPositionEstimatorPar");

  // ROOT Output
  m_rootFile->Write();
  m_rootFile->Close();

  B2INFO("PXD Gain Calibration Successful");
  return c_OK;
}

float PXDGainCalibrationAlgorithm::computeResidual(float gain, const std::string& treename, const VxdID& sensorID, int areaID)
{

  auto tree = getObjectPtr<TTree>(treename);

  tree->SetBranchAddress("sensorID", &m_sensorID);
  tree->SetBranchAddress("uCellID", &m_uCellID);
  tree->SetBranchAddress("vCellID", &m_vCellID);
  tree->SetBranchAddress("signal", &m_signal);
  tree->SetBranchAddress("isMC", &m_isMC);

  // Use TAxis to bin the data
  TAxis signalAxis(128, 0, 255);

  // Some counters for Data and MC
  vector<float> countData(128);
  vector<float> countMC(128);
  int sumData = 0;
  int sumMC = 0;

  // Loop over the tree is to fill the signal histograms
  const auto nEntries = tree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    int nDCD = 4;
    int nSWB = 6;

    auto currentSensorID = VxdID(m_sensorID);
    int iDCD = PXD::PXDMappingLookup::getDCDID(m_uCellID, m_vCellID, sensorID) - 1;
    int iSWB = PXD::PXDMappingLookup::getSWBID(m_vCellID) - 1;
    int currentAreaID = iDCD * nSWB + iSWB;

    if (currentSensorID == sensorID and currentAreaID == areaID) {

      if (m_isMC) {
        int bin = signalAxis.FindFixBin(gain * m_signal);
        countMC[bin - 1] = + 1;
        sumMC += 1;
      } else {
        int bin = signalAxis.FindFixBin(m_signal);
        countData[bin - 1] = + 1;
        sumData += 1;
      }
    }
  }
  float residual = 0;

  for (int i = 0; i < 128; i++) {
    residual += std::pow(countData[i] / sumData  - countMC[i] / sumMC, 2);
  }
  return residual;
}



void PXDGainCalibrationAlgorithm::createValidationHistograms(TH1D& dataHist, TH1D& mcHist, float gain, const std::string& treename,
    const VxdID& sensorID, int areaID)
{

  auto tree = getObjectPtr<TTree>(treename);

  tree->SetBranchAddress("sensorID", &m_sensorID);
  tree->SetBranchAddress("uCellID", &m_uCellID);
  tree->SetBranchAddress("vCellID", &m_vCellID);
  tree->SetBranchAddress("signal", &m_signal);
  tree->SetBranchAddress("isMC", &m_isMC);

  // Loop over the tree is to fill the signal histograms
  const auto nEntries = tree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    int nDCD = 4;
    int nSWB = 6;

    auto currentSensorID = VxdID(m_sensorID);
    int iDCD = PXD::PXDMappingLookup::getDCDID(m_uCellID, m_vCellID, sensorID) - 1;
    int iSWB = PXD::PXDMappingLookup::getSWBID(m_vCellID) - 1;
    int currentAreaID = iDCD * nSWB + iSWB;

    if (currentSensorID == sensorID and currentAreaID == areaID) {

      if (m_isMC) {
        mcHist.Fill(gain * m_signal);
      } else {
        dataHist.Fill(m_signal);
      }
    }
  }
  return;
}


