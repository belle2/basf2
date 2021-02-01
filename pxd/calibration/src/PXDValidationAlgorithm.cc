/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDValidationAlgorithm.h>
//#include <pxd/dbobjects/PXDGainMapPar.h>
//#include <calibration/dbobjects/TestCalibMean.h>

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


// Anonymous namespace for data objects used by PXDValidationAlgorithm class
namespace {

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


PXDValidationAlgorithm::PXDValidationAlgorithm():
  CalibrationAlgorithm("PXDCDSTCollector")
  , minTrackPoints(1000), save2DHists(false)
  //,m_file(nullptr), m_tree(nullptr)
  //,minTrackPoints(1000), safetyFactor(2.0), forceContinue(false), strategy(0)
{
  setDescription(
    " -------------------------- PXDValidationAlgorithm ---------------------------------\n"
    "                                                                                         \n"
    "  Algorithm for filling validation histograms. \n"
    " ----------------------------------------------------------------------------------------\n"
  );


}

PXDValidationAlgorithm::~PXDValidationAlgorithm()
{
}

CalibrationAlgorithm::EResult PXDValidationAlgorithm::calibrate()
{
  // Turn off ROOT GUI
  gROOT->SetBatch();

  // Current Exp No. and Run No.
  auto expRuns = getRunList();
  auto expRunsAll = getRunListFromAllData();
  // size should be 1
  if (!expRuns.size())
    return c_Failure;

  m_exp = expRuns.back().first;
  m_run = expRuns.back().second;
  B2INFO("Current ExpRuns: [("
         << expRuns.front().first << ", " << expRuns.front().second << "), ("
         << m_exp << ", " << m_run << ")]");

  // Get counter histograms and set pointers
  auto cluster_counter = getObjectPtr<TH1I>("PXDTrackClusterCounter");
  auto point_counter = getObjectPtr<TH1I>("PXDTrackPointCounter");

  // Extract number of sensors from counter histograms
  auto nSensors = getNumberOfSensors(cluster_counter);

  // Extract the number of grid bins from counter histograms
  unsigned short nBinsU = 0;
  unsigned short nBinsV = 0;
  getNumberOfBins(cluster_counter, nBinsU, nBinsV);


  B2INFO("Start info collection using a " << nBinsU << "x" << nBinsV << " grid per sensor.");
  B2INFO("Number of collected track points is " << point_counter->GetEntries()
         << " in " << nSensors << " sensors.");

  if (point_counter->GetEntries() < minTrackPoints) {
    B2WARNING("Not enough Data: Only " <<  point_counter->GetEntries() << " hits were collected but " << minTrackPoints
              << " needed!");
    return c_NotEnoughData;
  }


  auto hPassedHitsLayer1 = getObjectPtr<TH2F>("hPassedHitsLayer1");
  auto hTotalHitsLayer1 = getObjectPtr<TH2F>("hTotalHitsLayer1");
  auto hPassedHitsLayer2 = getObjectPtr<TH2F>("hPassedHitsLayer2");
  auto hTotalHitsLayer2 = getObjectPtr<TH2F>("hTotalHitsLayer2");
  if (!cluster_counter) return c_Failure;
  if (!point_counter) return c_Failure;
  if (!hPassedHitsLayer1) return c_Failure;
  if (!hTotalHitsLayer1) return c_Failure;
  if (!hPassedHitsLayer2) return c_Failure;
  if (!hTotalHitsLayer2) return c_Failure;

  // Save the current directory to change back later
  TDirectory* currentDir = gDirectory;
  //if (!currentDir) currentDir = gDirectory;

  // Create TFile if not exist
  if (!m_file) {
    std::string fileName = (this->getPrefix()) + "Validation.root";
    B2INFO("Creating file " << fileName);
    m_file = std::make_shared<TFile>(fileName.c_str(), "RECREATE");
  }

  // Create TTree if not exist
  if (!m_tree) {

    B2INFO("Creating TTree.");
    m_tree = std::make_shared<TTree>("tree", "PXD validation data");
    // Define histograms out of m_file
    currentDir->cd();
    m_hD0 = new TH1F("hD0", "Corrected d0;#Delta d0/#sqrt{2} [cm];Counts", 100, -0.03, 0.03);
    m_hZ0 = new TH1F("hZ0", "Corrected z0;#Delta z0/#sqrt{2} [cm];Counts", 100, -0.03, 0.03);
    m_file->cd();

    m_tree->Branch<int>("exp", &m_exp);
    m_tree->Branch<int>("run", &m_run);
    m_tree->Branch("pxdid", &m_pxdid);
    m_tree->Branch("uBin", &m_uBin);
    m_tree->Branch("vBin", &m_vBin);
    m_tree->Branch("nTrackPoints", &m_nTrackPoints);
    m_tree->Branch("nTrackClusters", &m_nTrackClusters);
    m_tree->Branch<TH1F>("hD0", &m_hD0, 32000, 0);
    m_tree->Branch<TH1F>("hZ0", &m_hZ0, 32000, 0);

    if (save2DHists) {
      m_hTrackPointsLayer1   = (TH2F*)hTotalHitsLayer1->Clone();
      m_hTrackClustersLayer1 = (TH2F*)hPassedHitsLayer1->Clone();
      m_hTrackPointsLayer2   = (TH2F*)hTotalHitsLayer2->Clone();
      m_hTrackClustersLayer2 = (TH2F*)hPassedHitsLayer2->Clone();

      // associated histograms
      m_tree->Branch<TH2F>("hTrackPointsLayer1", &m_hTrackPointsLayer1, 32000, 0);
      m_tree->Branch<TH2F>("hTrackClustersLayer1", &m_hTrackClustersLayer1, 32000, 0);
      m_tree->Branch<TH2F>("hTrackPointsLayer2", &m_hTrackPointsLayer2, 32000, 0);
      m_tree->Branch<TH2F>("hTrackClustersLayer2", &m_hTrackClustersLayer2, 32000, 0);
    }
  }

  m_file->cd();

  if (save2DHists) {
    // Just update 2D histograms
    *m_hTrackPointsLayer1   = *hTotalHitsLayer1;
    *m_hTrackClustersLayer1 = *hPassedHitsLayer1;
    *m_hTrackPointsLayer2   = *hTotalHitsLayer2;
    *m_hTrackClustersLayer2 = *hPassedHitsLayer2;

  }

  // Clear
  m_pxdid.clear();
  m_uBin.clear();
  m_vBin.clear();
  m_nTrackClusters.clear();
  m_nTrackPoints.clear();

  // Get resolution trees and create histograms
  auto tree_d0z0 = getObjectPtr<TTree>("tree_d0z0");
  if (!tree_d0z0) return c_Failure;
  float d0, z0;
  tree_d0z0->SetBranchAddress("d0", &d0);
  tree_d0z0->SetBranchAddress("z0", &z0);
  //TH1F hD0("hD0", "Corrected d0;#Delta d0/#sqrt{2} [cm];Counts", 100, -0.03, 0.03);
  //TH1F hZ0("hZ0", "Corrected z0;#Delta z0/#sqrt{2} [cm];Counts", 100, -0.03, 0.03);
  m_hD0->Reset();
  m_hZ0->Reset();
  string cuts = "abs(d0)<0.03&&abs(z0)<0.03";

  // Fill histograms from tree
  for (int i = 0; i < tree_d0z0->GetEntries(); i++) {
    tree_d0z0->GetEntry(i);
    if (fabs(d0) > 0.03 || fabs(z0) > 0.03)
      continue;
    m_hD0->Fill(d0);
    m_hZ0->Fill(z0);
  }




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
    int numberOfClusters = cluster_counter->GetBinContent(histoBin);
    int numberOfPoints = point_counter->GetBinContent(histoBin);

    m_pxdid.emplace_back(sensorID.getLayerNumber() * 1000 + sensorID.getLadderNumber() * 10 + sensorID.getSensorNumber());
    m_uBin.emplace_back(uBin);
    m_vBin.emplace_back(vBin);
    m_nTrackPoints.emplace_back(numberOfPoints);
    m_nTrackClusters.emplace_back(numberOfClusters);
  }

  m_tree->Fill();

  // At the last run of the data
  if (m_exp == expRunsAll.back().first &&
      m_run == expRunsAll.back().second) {
    B2INFO("Reached Final ExpRun: (" << m_exp << ", " << m_run << ")");
    m_file->cd();
    m_tree->Write();
    B2INFO("Writing Successful.");
    //if (m_file) m_file->Close();// Don't do it! We are using shared_ptr!
    if (m_hD0) delete m_hD0;
    if (m_hZ0) delete m_hZ0;
    if (save2DHists) {
      if (m_hTrackPointsLayer1) delete m_hTrackPointsLayer1;
      if (m_hTrackClustersLayer1) delete m_hTrackClustersLayer1;
      if (m_hTrackPointsLayer2) delete m_hTrackPointsLayer2;
      if (m_hTrackClustersLayer2) delete m_hTrackClustersLayer2;
    }
    m_file->Close();

  }
  currentDir->cd();

  // Saving dummy DBObject. Required by CAF?
  //TestCalibMean* correction = new TestCalibMean(m_run, gRandom->Gaus());
  //B2INFO("Saving calibration results.");
  //saveCalibration(correction, "DummyCalibMean");

  // Always return ok to have run-by-run info
  return c_OK;
}


bool PXDValidationAlgorithm::isBoundaryRequired(const Calibration::ExpRun& /*currentRun*/)
{
  // First run in data as we iterate, but our boundaries weren't set manually already?
  // Just set the first run to be a boundary and we are done.
  if (m_boundaries.empty()) {
    B2INFO("This is the first run encountered, let's say it is a boundary.");
    return true;
  } else {
    //return true;// -> simple run-by-run
    return false;
  }
}
