/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdPerformanceCollector/PXDPerformanceCollectorModule.h>

#include <framework/datastore/RelationArray.h>

#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>

#include <TTree.h>
#include <TH1I.h>
#include <TH2F.h>
//#include <TEfficiency.h>

#include <boost/format.hpp>

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDPerformanceCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDPerformanceCollectorModule::PXDPerformanceCollectorModule() : CalibrationCollectorModule()
  , m_selectedEff(true), m_selectedRes(true)
  , m_pxd2TrackEvent()
  , m_deltaD0oSqrt2(0.0), m_deltaZ0oSqrt2(0.0)
  , m_signal(0), m_estimated(0.0)
  , m_run(0), m_exp(0)
{
  // Set module properties
  setDescription("Calibration collector module for CDST data.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("minPt", m_minPt, "Minimum pt cut", float(1.0));
  addParam("minClusterCharge", m_minClusterCharge, "Minimum cluster charge cut", int(0));
  addParam("minClusterSize", m_minClusterSize, "Minimum cluster size cut ", int(2));
  addParam("maxClusterSize", m_maxClusterSize, "Maximum cluster size cut ", int(6));
  addParam("nBinsU", m_nBinsU, "Number of gain corrections per sensor along u side", int(4));
  addParam("nBinsV", m_nBinsV, "Number of gain corrections per sensor along v side", int(6));
  addParam("gainPayloadName", m_gainName, "Payload name for Gain to be read from DB", string(""));
  addParam("fillChargeRatioHistogram", m_fillChargeRatioHistogram,
           "Flag to fill Ratio (cluster charge to the expected MPV) histograms", bool(true));
  addParam("fillChargeTree", m_fillChargeTree, "Flag to fill cluster charge with the estimated MPV to TTree", bool(false));
  addParam("fillEventTree", m_fillEventTree, "Flag to fill event tree for validation", bool(false));

  // additional parameters for validation. Considering modularAnalysis for more flexible controls.
  addParam("minPt4Eff", m_minPt4Eff, "Minimum pt cut for efficiency validation", float(2.0));
  addParam("maxAbsVx", m_maxAbsVx, "Minimum abs(Vx) cut in cm for efficiency validation", float(0.03));
  addParam("maxAbsVy", m_maxAbsVy, "Minimum abs(Vy) cut in cm for efficiency validation", float(0.03));
  addParam("maxAbsVz", m_maxAbsVz, "Minimum abs(Vz) cut in cm for efficiency validation", float(0.155));

  addParam("minPt4Res", m_minPt4Res, "Minimum pt cut for resolution validation", float(1.0));
  addParam("minSVDHits", m_minSVDHits, "Minimum number of SVD hits foor resolution validation", int(8));
  addParam("minCDCHits", m_minCDCHits, "Minimum number of CDC hits foor resolution validation", int(21));
  addParam("maxAbsLambda", m_maxAbsLambda, "Maximum absolute dip angle (lambda)", float(0.5));
  addParam("minPBetaSinTheta3o2", m_minPBetaSinTheta3o2, "Minimum p*Beta*sin(theta_0)^{3/2}", float(2));
  addParam("maxAbsZ0", m_maxAbsZ0, "Maximum abs(z0)", float(1));
  addParam("maxAbsD0", m_maxAbsD0, "Maximum abs(d0)", float(0.3));


}

void PXDPerformanceCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxd2TrackEvents.isRequired(m_store2TrackEventsName);

  if (m_nBinsU == 0) {
    B2WARNING("Number of bins along u side incremented from 0->1");
    m_nBinsU = 1;
  }

  if (m_nBinsV == 0) {
    B2WARNING("Number of bins along v side incremented from 0->1");
    m_nBinsV = 1;
  }

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  int nPXDSensors = gTools->getNumberOfPXDSensors();

  //-------------------------------------------------------------------------------------
  // PXDClusterCounter: Count the number of PXDClusters and store charge for each uBin/vBin pair
  //-------------------------------------------------------------------------------------

  auto hPXDClusterCounter = new TH1I("hPXDClusterCounter", "Number of clusters found in data sample",
                                     m_nBinsU * m_nBinsV * nPXDSensors, 0,
                                     m_nBinsU * m_nBinsV * nPXDSensors);
  hPXDClusterCounter->GetXaxis()->SetTitle("bin id");
  hPXDClusterCounter->GetYaxis()->SetTitle("Number of clusters");
  auto hPXDClusterChargeRatio = new TH2F("hPXDClusterChargeRatio", "Charge ratio of clusters found in data sample",
                                         m_nBinsU * m_nBinsV * nPXDSensors, 0,
                                         m_nBinsU * m_nBinsV * nPXDSensors,
                                         400, 0., 4.);
  hPXDClusterChargeRatio->GetXaxis()->SetTitle("bin id");
  hPXDClusterChargeRatio->GetYaxis()->SetTitle("Cluster charge ratio (relative to expected MPV)");
  //auto hPXDTrackClusterCounter = new TH1I("hPXDTrackClusterCounter", "Number of clusters found in data sample",
  //m_nBinsU * m_nBinsV * nPXDSensors, 0,
  //m_nBinsU * m_nBinsV * nPXDSensors);
  //hPXDTrackClusterCounter->GetXaxis()->SetTitle("bin id");
  //hPXDTrackClusterCounter->GetYaxis()->SetTitle("Number of clusters");
  //auto hPXDTrackPointCounter = new TH1I("hPXDTrackPointCounter", "Number of clusters found in data sample",
  //m_nBinsU * m_nBinsV * nPXDSensors, 0,
  //m_nBinsU * m_nBinsV * nPXDSensors);
  //hPXDTrackPointCounter->GetXaxis()->SetTitle("bin id");
  //hPXDTrackPointCounter->GetYaxis()->SetTitle("Number of clusters");
  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int uBin = 0; uBin < m_nBinsU; uBin++) {
      for (int vBin = 0; vBin < m_nBinsV; vBin++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        string sensorDescr = id;
        hPXDClusterCounter->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
                                                    str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
        if (m_fillChargeRatioHistogram)
          hPXDClusterChargeRatio->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
                                                          str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
        //hPXDTrackClusterCounter->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
        //str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
        //hPXDTrackPointCounter->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
        //str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
      }
    }
  }

  registerObject<TH1I>("PXDClusterCounter", hPXDClusterCounter);
  if (m_fillChargeRatioHistogram)
    registerObject<TH2F>("PXDClusterChargeRatio", hPXDClusterChargeRatio);

  //-------------------------------------------------------------------------------------
  // PXDTrackClusterCounter: Count the number of PXDClustersFrom tracks (the same track selection as for track points)
  //-------------------------------------------------------------------------------------
  auto hPXDTrackClusterCounter = (TH1I*)hPXDClusterCounter->Clone("hPXDTrackClusterCounter");
  hPXDTrackClusterCounter->SetTitle("Number of track clusters");
  hPXDTrackClusterCounter->GetYaxis()->SetTitle("Number of track clusters");
  registerObject<TH1I>("PXDTrackClusterCounter", hPXDTrackClusterCounter);

  //-------------------------------------------------------------------------------------
  // PXDTrackPointCounter: Count the number of PXDClustersFrom tracks (the same track selection as for track points)
  //-------------------------------------------------------------------------------------
  auto hPXDTrackPointCounter = (TH1I*)hPXDClusterCounter->Clone("hPXDTrackPointCounter");
  hPXDTrackPointCounter->SetTitle("Number of track points");
  hPXDTrackPointCounter->GetYaxis()->SetTitle("Number of track points");
  registerObject<TH1I>("PXDTrackPointCounter", hPXDTrackPointCounter);

  //----------------------------------------------------------------------
  // PXDTrees for gain calibration: One tree to store the calibration data for each grid bin
  //----------------------------------------------------------------------

  if (m_fillChargeTree) // only fill the tree when required
    for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
      for (int uBin = 0; uBin < m_nBinsU; uBin++) {
        for (int vBin = 0; vBin < m_nBinsV; vBin++) {
          VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
          auto layerNumber = id.getLayerNumber();
          auto ladderNumber = id.getLadderNumber();
          auto sensorNumber = id.getSensorNumber();
          string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);
          auto tree = new TTree(treename.c_str(), treename.c_str());
          tree->Branch<int>("signal", &m_signal);
          tree->Branch<float>("estimated", &m_estimated);
          registerObject<TTree>(treename, tree);
        }
      }
    }

  // TEfficiency for validation, no Reset() and may be questionable for merging
  //auto effPXDLayer1 = new TEfficiency("PXDLayer1Efficiency", "Efficiency of PXD innner layer;#phi;#z [cm];#epsilon", 730, -M_PI, M_PI, 400, -3.2, 6.2);
  //auto effPXDLayer2 = new TEfficiency("PXDLayer2Efficiency", "Efficiency of PXD outer layer;#phi;#z [cm];#epsilon", 128, 1.4, 2.5, 400, -4.2, 8.2);
  //registerObject<TEfficiency>("PXDLayer1Efficiency", effPXDLayer1);
  //registerObject<TEfficiency>("PXDLayer2Efficiency", effPXDLayer2);
  auto hTotalHitsLayer1  = new TH2F("hTotalHitsLayer1",  "Total number of hits from layer 1;#phi;z [cm]",  730, -M_PI, M_PI, 400,
                                    -3.2, 6.2);
  auto hPassedHitsLayer1 = new TH2F("hPassedHitsLayer1", "Passed number of hits from layer 1;#phi;z [cm]", 730, -M_PI, M_PI, 400,
                                    -3.2, 6.2);
  auto hTotalHitsLayer2  = new TH2F("hTotalHitsLayer2",  "Total number of hits from layer 2;#phi;z [cm]",  128,   1.4,  2.5, 400,
                                    -4.2, 8.2);
  auto hPassedHitsLayer2 = new TH2F("hPassedHitsLayer2", "Passed number of hits from layer 2;#phi;z [cm]", 128,   1.4,  2.5, 400,
                                    -4.2, 8.2);
  registerObject<TH2F>("hTotalHitsLayer1", hTotalHitsLayer1);
  registerObject<TH2F>("hPassedHitsLayer1", hPassedHitsLayer1);
  registerObject<TH2F>("hTotalHitsLayer2", hTotalHitsLayer2);
  registerObject<TH2F>("hPassedHitsLayer2", hPassedHitsLayer2);

  // trees for correctd d0 and z0 to the IP
  auto treeD0Z0 = new TTree("tree_d0z0", "TTree of corrected d0 and z0");
  treeD0Z0->Branch<float>("d0", &m_deltaD0oSqrt2);
  treeD0Z0->Branch<float>("z0", &m_deltaZ0oSqrt2);
  registerObject<TTree>("tree_d0z0", treeD0Z0);

  // dbtree
  auto dbtree = new TTree("dbtree", "dbtree");
  dbtree->Branch<int>("run", &m_run);
  dbtree->Branch<int>("exp", &m_exp);
  dbtree->Branch<PXDGainMapPar>("gainMap", &m_gainMap);
  registerObject<TTree>("dbtree", dbtree);

  if (m_fillEventTree) {
    auto tree = new TTree("pxd", "PXD 2-track events");
    tree->Branch<PXD2TrackEvent>("PXD2TrackEvent", &m_pxd2TrackEvent, 8000, 1);
    registerObject<TTree>("pxd", tree);
  }
}

void PXDPerformanceCollectorModule::startRun() // Do your beginRun() stuff here
{
  m_run = m_evtMetaData->getRun();
  m_exp = m_evtMetaData->getExperiment();
  if (m_gainName.length()) {
    DBObjPtr<PXDGainMapPar> gainMap(m_gainName);
    m_gainMap = *gainMap;
  } else {
    m_gainMap = PXDGainMapPar();
  }
  getObjectPtr<TTree>("dbtree")->Fill();
}

void PXDPerformanceCollectorModule::collect() // Do your event() stuff here
{
  // If no input, nothing to do
  if (!m_pxd2TrackEvents) return;

  // Beam spot
  DBObjPtr<BeamSpot> beamSpotDB;
  auto ip = beamSpotDB->getIPPosition();

  // Actually only one event holder / event
  for (auto& pxd2TrackEvent : m_pxd2TrackEvents) {
    m_selectedRes = true;
    m_selectedEff = true;

    auto vertex = pxd2TrackEvent.getVertex();
    vertex -= ip; // correct vertex relative to ip
    if (fabs(vertex.X()) > m_maxAbsVx ||
        fabs(vertex.Y()) > m_maxAbsVy ||
        fabs(vertex.Z()) > m_maxAbsVz)
      m_selectedEff = false;

    // track level selection and collection
    collectFromTrack(pxd2TrackEvent.getTrackP());
    collectFromTrack(pxd2TrackEvent.getTrackM());

    // event level collection
    collectDeltaIP(pxd2TrackEvent);

    if (m_fillEventTree) {
      m_pxd2TrackEvent = pxd2TrackEvent;
      getObjectPtr<TTree>("pxd")->Fill();
    }
  }

}

void PXDPerformanceCollectorModule::collectDeltaIP(const PXD2TrackEvent& event)
{
  if (!m_selectedRes) return;
  auto d0p_1 = event.getTrackP().d0p;
  auto d0p_2 = event.getTrackM().d0p;
  auto z0p_1 = event.getTrackP().z0p;
  auto z0p_2 = event.getTrackM().z0p;
  m_deltaD0oSqrt2 = (d0p_1 + d0p_2) / sqrt(2.);
  m_deltaZ0oSqrt2 = (z0p_1 - z0p_2) / sqrt(2.);

  // Fill the tree of impact parameters
  getObjectPtr<TTree>("tree_d0z0")->Fill();
}

void PXDPerformanceCollectorModule::collectFromTrack(const PXD2TrackEvent::baseType& track)
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  bool selected4Gain = true;
  bool selected4Eff  = true;

  if (track.pt < m_minPt) selected4Gain = false;
  if (track.pt < m_minPt4Eff) selected4Eff = false;  // just applied on track level

  // Track level filtering for resolution validation
  if (track.pt < m_minPt4Res) m_selectedRes = false;
  if (track.nPXDHits < 1 || track.nCDCHits < m_minCDCHits || track.nSVDHits < m_minSVDHits)
    m_selectedRes = false;
  if (fabs(track.d0p) > m_maxAbsD0 || fabs(track.z0p) > m_maxAbsZ0)
    m_selectedRes = false;
  auto lambda0 = atan(track.tanLambda);
  if (fabs(lambda0) > m_maxAbsLambda)
    m_selectedRes = false;
  auto sinTheta0 = 1. / sqrt(1. + pow(track.tanLambda, 2));
  auto pBetaSinTheta3o2 = track.pt * 1.0 * sqrt(sinTheta0);
  if (pBetaSinTheta3o2 < m_minPBetaSinTheta3o2)
    m_selectedRes = false;

  for (auto& trackCluster : track.trackClusters) {
    bool selectedCluster = true;
    auto cluster = trackCluster.cluster;
    auto intersection = trackCluster.intersection;
    auto usedInTrack = trackCluster.usedInTrack;

    // Check for valid cluster and intersection
    if (!usedInTrack || intersection.chargeMPV <= 0)
      selectedCluster = false;

    // Apply cluster selection cuts
    if (cluster.charge < m_minClusterCharge || cluster.size < m_minClusterSize || cluster.size > m_maxClusterSize)
      selectedCluster = false;

    if (cluster.pxdID <= 0) {
      B2FATAL("Unexpected cluster module id : " << cluster.pxdID);

    }

    // Fill tree or histograms for gain calibration
    if (selected4Gain && selectedCluster) {

      // Compute variables from cluster needed for gain estimation
      m_signal = cluster.charge;
      m_estimated = intersection.chargeMPV;

      VxdID sensorID = getVxdIDFromPXDModuleID(cluster.pxdID);
      const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      auto uID = Info.getUCellID(cluster.posU);
      auto vID = Info.getVCellID(cluster.posV);
      auto iSensor = gTools->getPXDSensorIndex(sensorID);
      auto layerNumber = sensorID.getLayerNumber();
      auto ladderNumber = sensorID.getLadderNumber();
      auto sensorNumber = sensorID.getSensorNumber();
      auto uBin = PXD::PXDGainCalibrator::getInstance().getBinU(sensorID, uID, vID, m_nBinsU);
      auto vBin = PXD::PXDGainCalibrator::getInstance().getBinV(sensorID, vID, m_nBinsV);
      // Calculate bin ID based on iSensor, uBin, vBin and number of bins in u/v
      int binID = iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin;

      // Increment the counter
      getObjectPtr<TH1I>("PXDClusterCounter")->Fill(binID);

      // Fill variabels into tree
      if (m_fillChargeTree) {
        string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);
        getObjectPtr<TTree>(treename)->Fill();
      }

      // Fill cluster charge ratio histogram if enabled
      if (m_fillChargeRatioHistogram) {
        double ratio = m_signal / m_estimated;
        auto axis = getObjectPtr<TH2F>("PXDClusterChargeRatio")->GetYaxis();
        double maxY  = axis->GetBinCenter(axis->GetNbins());
        // Manipulate too large ratio for better estimation on median.
        getObjectPtr<TH2F>("PXDClusterChargeRatio")->Fill(binID, TMath::Min(ratio, maxY));
      }
    }

    // Fill effciency
    if (m_selectedEff && selected4Eff) {
      auto x = intersection.x;
      auto y = intersection.y;
      auto phi = atan2(y, x);
      auto z = intersection.z;

      // Get uBin and vBin from a global point.
      VxdID sensorID = getVxdIDFromPXDModuleID(cluster.pxdID);
      const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      auto localPoint = Info.pointToLocal(TVector3(x, y, z));
      auto uID = Info.getUCellID(localPoint.X());
      auto vID = Info.getVCellID(localPoint.Y());
      auto iSensor = gTools->getPXDSensorIndex(sensorID);
      auto uBin = PXD::PXDGainCalibrator::getInstance().getBinU(sensorID, uID, vID, m_nBinsU);
      auto vBin = PXD::PXDGainCalibrator::getInstance().getBinV(sensorID, vID, m_nBinsV);

      // Filling counters
      getObjectPtr<TH1I>("PXDTrackPointCounter")->Fill(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin);
      if (usedInTrack)
        getObjectPtr<TH1I>("PXDTrackClusterCounter")->Fill(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin);

      // Filling 2D histograms
      if (cluster.pxdID < 2000) {
        //getObjectPtr<TEfficiency>("PXDLayer1Efficiency")->Fill(usedInTrack,phi,z);
        getObjectPtr<TH2F>("hTotalHitsLayer1")->Fill(phi, z);
        if (usedInTrack)
          getObjectPtr<TH2F>("hPassedHitsLayer1")->Fill(phi, z);
      } else {
        //getObjectPtr<TEfficiency>("PXDLayer2Efficiency")->Fill(usedInTrack,phi,z);
        getObjectPtr<TH2F>("hTotalHitsLayer2")->Fill(phi, z);
        if (usedInTrack)
          getObjectPtr<TH2F>("hPassedHitsLayer2")->Fill(phi, z);
      }
    }

  } // end loop trackClusters

}
