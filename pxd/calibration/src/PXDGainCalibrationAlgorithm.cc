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
#include <vxd/dataobjects/VxdID.h>
#include <pxd/unpacking/PXDMappingLookup.h>

#include <string>
#include <tuple>

#include <boost/format.hpp>
#include <cmath>

#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TMath.h"

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

  string* shapeNamePtr = &m_shapeName;

  tree->SetBranchAddress("sensorID", &m_sensorID);
  tree->SetBranchAddress("ShapeName", &shapeNamePtr);
  tree->SetBranchAddress("ClusterEta", &m_clusterEta);
  tree->SetBranchAddress("uCellID", &m_uCellID);
  tree->SetBranchAddress("vCellID", &m_vCellID);
  tree->SetBranchAddress("signal", &m_signal);

  int nDCD = 4;
  int nSWB = 6;

  // List of sensors found in data
  vector< pair<VxdID, int > > sensorList;

  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

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


  // Loop over sensorList to select sensors for
  // next calibration step


  // Vector for digit charge histograms stored by sensorID and areaID
  vector< pair<VxdID, vector<TH1D> > > signalHistosVec;

  for (auto iter : sensorList) {
    auto sensorID = iter.first;
    auto counter = iter.second;
    vector<TH1D> signalHistos;

    for (int areaID = 0; areaID < nDCD * nSWB; areaID++) {
      int iDCD = areaID / 6 + 1;
      int iSWB = areaID % 6 + 1;
      string histoname = str(format("signal_sensor_%1%_dcd_%2%_swb_%3%") % sensorID % iDCD % iSWB);
      TH1D signalHisto(histoname.c_str(), histoname.c_str(), 255, 0, 255);
      signalHisto.SetXTitle("cluster charge / ADU");
      signalHisto.SetYTitle("number of clusters");

      signalHistos.push_back(signalHisto);
    }
    signalHistosVec.push_back(pair<VxdID, vector<TH1D>>(sensorID, signalHistos));
  }

  // Loop over the tree is to fill the signal histograms
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    auto sensorID = VxdID(m_sensorID);
    int iDCD = PXD::PXDMappingLookup::getDCDID(m_uCellID, m_vCellID, sensorID) - 1;
    int iSWB = PXD::PXDMappingLookup::getSWBID(m_vCellID) - 1;
    int areaID = iDCD * nSWB + iSWB;

    auto it = std::find_if(signalHistosVec.begin(), signalHistosVec.end(),
    [&](const pair<VxdID, vector<TH1D> >& element) { return element.first == sensorID;});
    //Item exists in map
    if (it != signalHistosVec.end()) {
      it->second.at(areaID).Fill(m_signal);
    }
  }

  // Dump histos to file
  for (auto iter : signalHistosVec) {
    auto sensorID = iter.first;
    auto& histos = iter.second;

    string mapname = string(sensorID) + "_MPV";
    TH2D mpvMap(mapname.c_str(), mapname.c_str() , 4, 1, 5, 6, 1, 7);
    mpvMap.SetXTitle("DCD ID");
    mpvMap.SetYTitle("SWB ID");
    mpvMap.SetZTitle("ADU");
    mpvMap.SetStats(false);

    for (int areaID = 0; areaID < nDCD * nSWB; areaID++) {

      int iDCD = areaID / 6;
      int iSWB = areaID % 6;

      TF1 f1("f1", "landau", 18, 80);
      histos.at(areaID).Fit("f1", "R");

      TF1 f2("f2", "landau", f1.GetParameter(1) - 10, f1.GetParameter(1) + 30);
      histos.at(areaID).Fit("f2", "R");

      mpvMap.SetBinContent(iDCD + 1, iSWB + 1, f2.GetParameter(1));

      m_rootFile->cd();
      histos.at(areaID).Write();
    }
    m_rootFile->cd();
    mpvMap.Write();
  }

  // Save the cluster positions to database.
  //saveCalibration(positionEstimator, "PXDClusterPositionEstimatorPar");

  // ROOT Output
  m_rootFile->Write();
  m_rootFile->Close();

  B2INFO("PXD Gain Calibration Successful");
  return c_OK;
}








