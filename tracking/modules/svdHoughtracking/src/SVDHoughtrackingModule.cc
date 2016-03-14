/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/svdHoughtracking/SVDHoughtrackingModule.h>
//#include <tracking/modules/svdHoughtracking/basf2_tracking.h>
#include <vxd/geometry/GeoCache.h>
//#include <vxd/geometry/GeoVXDPosition.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/SVDHoughCluster.h>
#include <tracking/dataobjects/SVDHoughTrack.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <root/TMath.h>
#include <root/TGeoMatrix.h>
#include <root/TRandom.h>
#include <time.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

/* Options */
//#define INJECT_GHOSTS
//#define DBG_THETA

#undef B2DEBUG
#define B2DEBUG(level, streamText) \
  B2LOG(LogConfig::c_Debug, level, streamText)


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDHoughtracking)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDHoughtrackingModule::SVDHoughtrackingModule() : Module(), curTrackEff(0.0), totFakeTracks(0.0), totTracks(0), allTracks(0),
  curROIEff(0.0), totROITrueHits(0), curHitsInROIs(0), m_rootFile(0), m_histROIy(0), m_histROIz(0), m_histSimHits(0)
{
  //Set module properties
  setDescription("Hough tracking algorithm for SVD data");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Define module parameters

  // 1. Collections
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticle collection name", string(""));
  addParam("SVDTrueHits", m_storeSVDTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("SVDSimHits", m_storeSVDSimHitsName,
           "SimHit collection name", string(""));
  addParam("SVDDigits", m_storeSVDDigitsName,
           "Digits collection name", string(""));
  addParam("SVDClusters", m_storeSVDClusterName,
           "SVD Cluster collection name", string(""));
  addParam("PXDDigits", m_storePXDDigitsName,
           "Digits collection name", string(""));
  addParam("PXDSimHits", m_storePXDSimHitsName,
           "SimHit collection name", string(""));
  addParam("PXDTrueHits", m_storePXDTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("SVDHoughCluster", m_storeHoughCluster,
           "SVDHoughCluster Collection", string(""));
  addParam("SVDHoughTrack", m_storeHoughTrack,
           "SVDHoughTrack Collection", string(""));
  addParam("PXDExtrapolatedHits", m_storeExtrapolatedHitsName,
           "Extrapolated hits Collection", string(""));

  // 2. VXD vis
  addParam("SVDPosName", m_svdPosName,
           "VXD Position file name", string(""));
  addParam("SVDStripName", m_svdStripName,
           "VXD Position coordinate file name", string(""));

  // 3. Options
  addParam("UseClusters", m_useClusters,
           "Use SVD clusters?", bool(true));
  addParam("UseFPGAClusters", m_useFPGAClusters,
           "Use FPGA algorithm for clustering?", bool(false));
  addParam("AnalyseFPGAClusters", m_analyseFPGAClusters,
           "Analyse FPGA clusters", bool(false));
  addParam("UseTrueHitClusters", m_useTrueHitClusters,
           "Use True Hit Clusters?", bool(false));
  addParam("UseSimHitClusters", m_useSimHitClusters,
           "Use True Hit Clusters?", bool(false));
  addParam("SaveHits", m_saveHits,
           "Save hits for FPGA conversion", bool(false));
  addParam("SaveStrips", m_saveStrips,
           "Save strips for FPGA conversion", bool(false));
  addParam("FullTrackingPipeline", m_fullTrackingPipeline,
           "Run full tracking pipelin?", bool(false));
  addParam("WriteHoughSpace", m_writeHoughSpace,
           "Write Hough space into a gnuplot file?", bool(false));
  addParam("WriteHoughSectors", m_writeHoughSectors,
           "Write Secotrs into a gnuplot file?", bool(false));
  addParam("UseSensorFilter", m_useSensorFilter,
           "Use the Sensor layer filter", bool(true));
  addParam("UseRadiusFilter", m_useRadiusFilter,
           "Use radius filter", bool(true));
  addParam("UseHashPurify", m_useHashPurify,
           "Chose the Hash (true), or the List purifier", bool(false));
  addParam("UseTrackMerger", m_useTrackMerger,
           "Use the track merger", bool(false));
  addParam("UsePhiOnly", m_usePhiOnly,
           "Use Phi/radius reconstruction only", bool(false));
  addParam("UseThetaOnly", m_useThetaOnly,
           "Use Theta reconstruction only", bool(false));
  addParam("CompareWithMCParticle", m_compareMCParticle,
           "Compare reconstructed tracks with MC Particles?", bool(false));
  addParam("CompareWithMCParticleVerbose", m_compareMCParticleVerbose,
           "Verbose output of the comparator function", bool(false));
  addParam("ConformalTrafoP", m_conformalTrafoP,
           "Use Conformal trafo in P", bool(true));
  addParam("ConformalTrafoN", m_conformalTrafoN,
           "Use Conformal trafo in N", bool(false));
  addParam("TbMapping", m_tbMapping,
           "Use testbeam mapping", bool(false));
  addParam("StraightTracks", m_straightTracks,
           "Reconstruct only straight tracks", bool(false));

  /* ROIs */
  addParam("CreateROIs", m_createROI,
           "Create Region of Interests", bool(true));
  addParam("CreatePXDMap", m_createPXDMap,
           "Create a gnuplot hitmap of the PXD?", bool(false));
  addParam("AnalyseROIVerbose", m_analyseROIVerbose,
           "Verbose output of ROI analyser", bool(false));
  addParam("PXDExtrapolation", m_PXDExtrapolation,
           "Run PXD extrapolation?", bool(false));
  addParam("PXDTbExtrapolation", m_PXDTbExtrapolation,
           "Use testbeam extrapolation", bool(false));
  addParam("UsePhiExtrapolation", m_usePhiExtrapolation,
           "Use Phi extrapolation to PXD", bool(false));
  addParam("UseThetaExtrapolation", m_useThetaExtrapolation,
           "Use Phi extrapolation to PXD", bool(false));
  addParam("PrintTrackInfo", m_printTrackInfo,
           "Use testbeam extrapolation", bool(false));
  addParam("PrintStatistics", m_printStatistics,
           "Use testbeam extrapolation", bool(false));
  addParam("statisticsFilename", m_statisticsFileName,
           "File name for statistics generation", string(""));
  addParam("rootFilename", m_rootFilename,
           "File name for statistics generation", string(""));

  // 4. Noise Filter
  addParam("DisableNoiseFilter", m_disableNoiseFilter,
           "Disable noise filter for clusters?", bool(false));
  addParam("noiseFactor", m_noiseFactor,
           "Factor for next neighbour detecton", (unsigned int)(4));
  addParam("noiseDownThreshold", m_noiseUpThreshold,
           "Noise filter down threshold", (unsigned int)(1));
  addParam("noiseUpThreshold", m_noiseDownThreshold,
           "Noise filter upper threshold", (unsigned int)(250));

  /* 5. Hough Trafo */
  addParam("minimumLines", m_minimumLines,
           "Minimum lines to be found in order to continue", (unsigned int)(4));
  addParam("criticalIterationsN", m_critIterationsN,
           "Critical iterations ones a cell becomes a track", (unsigned int)(7));
  addParam("criticalIterationsP", m_critIterationsP,
           "Critical iterations ones a cell becomes a track", (unsigned int)(7));
  addParam("maxIterationsN", m_maxIterationsN,
           "Critical iterations ones a cell becomes a track", (unsigned int)(12));
  addParam("maxIterationsP", m_maxIterationsP,
           "Critical iterations ones a cell becomes a track", (unsigned int)(12));
  addParam("rectSizeN", m_rectSizeN,
           "Size of rectengular in N", (double)(1.0));
  addParam("rectSizeP", m_rectSizeP,
           "Size of rectengular in P", (double)(1.0));
  addParam("rectScaleN", m_rectScaleN,
           "Scale rectengular in N", (double)(1.0));
  addParam("rectScaleP", m_rectScaleP,
           "Scale rectengular in P", (double)(1.0));
  addParam("rectXP1", m_rectXP1,
           "Size of rectengular in x P", (double)(M_PI / -2.0));
  addParam("rectXP2", m_rectXP2,
           "Size of rectengular in x P", (double)(M_PI / 2.0));
  addParam("rectXN1", m_rectXN1,
           "Size of rectengular in x N", (double)(M_PI / -2.0));
  addParam("rectXN2", m_rectXN2,
           "Size of rectengular in x N", (double)(M_PI / 2.0));
  addParam("RadiusThreshold", m_radiusThreshold,
           "Cut off radius threshold", (double)(5.0));
  addParam("MergeThreshold", m_mergeThreshold,
           "Merge threshold", (double)(0.01));
}

void
SVDHoughtrackingModule::initialize()
{
  int theta_bins = 133; /* Number of bins in theta */
  int phi_bins = 360; /* Number of bins in phi */
  int pt_bins = 396; /* Number of bins pT */
  TDirectory* clusterDir, *houghDir, *effDir, *fakeDir, *roiDir;
  StoreArray<SVDHoughCluster>::registerPersistent(m_storeHoughCluster, DataStore::c_Event, false);
  StoreArray<SVDHoughTrack>::registerPersistent(m_storeHoughTrack, DataStore::c_Event, false);
  StoreArray<SVDHoughCluster>::registerPersistent(m_storeExtrapolatedHitsName, DataStore::c_Event, false);

  /* Axis description */
  TString xAxisPt = "pT [GeV]";
  TString xAxisPhi = "#varphi [deg]";
  TString xAxisTheta = "#theta [deg]";

  /* For testbeam only! */
  tb_radius[0] = -5.6; /* Layer 1 of PXD */
  tb_radius[1] = -4.8; /* Layer 2 of PXD */

  /* Thresholds for sector finding */
  dist_thres[0] = 4.2397;
  dist_thres[1] = 2.55;
  dist_thres[2] = 3.15;
  //dist_thres[0] = 4.60;
  //dist_thres[1] = 2.98;
  //dist_thres[2] = 3.65;


  /* Sanity check of some options */
  if (m_usePhiOnly && m_useThetaOnly) {
    B2WARNING("Both UsePhi and UseThetaOnly set. Disable Theta..")
    m_useThetaOnly = false;
  }

  /* Init min and max R */
  minR = -9E+99;
  maxR = +9E+99;

  /* Open file for output position writing */
  if (!m_svdPosName.empty()) {
    of_pos.open((const char*) m_svdPosName.c_str(), ofstream::out);
    of_pos << "# Start data" << endl;
  }

  if (!m_svdStripName.empty()) {
    of_strip.open((const char*) m_svdStripName.c_str(), ofstream::out);
    of_strip << "# Start coordinate" << endl;
  }

  /* Create (debug) plots */
  if (!m_rootFilename.empty()) {
    m_rootFile = new TFile(m_rootFilename.c_str(), "RECREATE");
    m_rootFile->cd();

    /* Clustering engine */
    if (m_analyseFPGAClusters) {
      clusterDir = m_rootFile->mkdir("Cluster");
      clusterDir->cd();
      m_histClusterU = new TH1D("DeltaU", "Cluster spread in U", 100, -0.015, 0.015);
      m_histClusterV = new TH1D("DeltaV", "Cluster spread in V", 100, -0.015, 0.015);
      m_histTrueClusterU = new TH1D("DeltaTrueU", "Spread in U between Clusters and TrueHits", 100, -0.100, 0.100);
      m_histTrueClusterV = new TH1D("DeltaTrueV", "Spread in V between Clusters and TrueHits", 100, -0.100, 0.100);
    }

    /* Hough debugging */
    if (m_compareMCParticle) {
      houghDir = m_rootFile->mkdir("Hough");
      houghDir->cd();
      m_histHoughDiffPhi = new TH1D("DeltaPhi", "Spread in Phi", 1000, -5.0, 5.0);
      m_histHoughDiffTheta = new TH1D("DeltaTheta", "Spread in Theta", 1000, -5.0, 5.0);
    }

    /* ROI extrapolation */
    m_rootFile->cd();
    m_histROIy = new TH1D("Dy", "ROI spread in y", 1000, -0.005, 0.005);
    m_histROIz = new TH1D("Dz", "ROI spread in z", 1000, -0.005, 0.005);
    m_histDist = new TH1D("Distance", "Distance spread in sector map", 1000, 3.0, 10.0);
    m_histSimHits = new TH1I("Sim hits", "Simhits per run", 100, 0.0, 100);

    /* Efficiency plots */
    effDir = m_rootFile->mkdir("Efficiency");
    effDir->cd();
    /* Histogram for pT-distribution */
    m_histPtDist = new TH1D("pTDist", "pT-Distribution", pt_bins, 0.02, 2.0);
    m_histPtDist->GetXaxis()->SetTitle(xAxisPt);
    m_histPtDist->GetYaxis()->SetTitle("# tracks");
    /* Histogram for correctly reconstructed tracks in pT */
    m_histPtPhiRecon = new TH1D("pTPhirecon", "pT of reconstructed tracks in Phi", pt_bins, 0.02, 2.0);
    m_histPtPhiRecon->GetXaxis()->SetTitle(xAxisPt);
    m_histPtPhiRecon->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histPtThetaRecon = new TH1D("pTThetarecon", "pT of reconstructed tracks in Theta", pt_bins, 0.02, 2.0);
    m_histPtThetaRecon->GetXaxis()->SetTitle(xAxisPt);
    m_histPtThetaRecon->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histPtRecon = new TH1D("pTRecon", "pT of reconstructed tracks", pt_bins, 0.02, 2.0);
    m_histPtRecon->GetXaxis()->SetTitle(xAxisPt);
    m_histPtRecon->GetYaxis()->SetTitle("# reconstructed tracks");
    /* Histogram for efficiency vs pT (only in phi while reconstruction in theta is not proved) */
    m_histPtEffPhi = new TH1D("pTEffPhi", "Efficiency vs pT (in Phi)", pt_bins, 0.02, 2.0);
    m_histPtEffPhi->GetXaxis()->SetTitle(xAxisPt);
    m_histPtEffPhi->GetYaxis()->SetTitle("#epsilon");
    m_histPtEffTheta = new TH1D("pTEffTheta", "Efficiency vs pT (in Theta)", pt_bins, 0.02, 2.0);
    m_histPtEffTheta->GetXaxis()->SetTitle(xAxisPt);
    m_histPtEffTheta->GetYaxis()->SetTitle("#epsilon");
    m_histPtEff = new TH1D("pTEff", "Efficiency vs pT", pt_bins, 0.02, 2.0);
    m_histPtEff->GetXaxis()->SetTitle(xAxisPt);
    m_histPtEff->GetYaxis()->SetTitle("#epsilon");
    /* Histogram for Phi-distribution */
    m_histPhiDist = new TH1D("PhiDist", "Phi-Distribution", phi_bins, -180, 180);
    m_histPhiDist->GetXaxis()->SetTitle(xAxisPhi);
    m_histPhiDist->GetYaxis()->SetTitle("# tracks");
    /* Histogram for correctly reconstructed tracks in Phi */
    m_histPhiRecon = new TH1D("Phirecon", "Phi of reconstructed tracks", phi_bins, -180, 180);
    m_histPhiRecon->GetXaxis()->SetTitle(xAxisPhi);
    m_histPhiRecon->GetYaxis()->SetTitle("# reconstructed tracks");
    /* Histogram for efficiency vs phi */
    m_histEffPhi = new TH1D("EffPhi", "Efficiency vs Phi", phi_bins, -180, 180);
    m_histEffPhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histEffPhi->GetYaxis()->SetTitle("#epsilon");
    /* Histogram for Theta-distribution */
    m_histThetaDist = new TH1D("ThetaDist", "Theta-Distribution", theta_bins, 17, 150);
    m_histThetaDist->GetXaxis()->SetTitle(xAxisTheta);
    m_histThetaDist->GetYaxis()->SetTitle("# tracks");
    m_histProjectedThetaDist = new TH1D("ProjThetaDist", "Projected Theta Distribution", 180, 0, 180);
    m_histProjectedThetaDist->GetXaxis()->SetTitle(xAxisTheta);
    m_histProjectedThetaDist->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histThetaPhiDist = new TH1D("ThetaPhiDist", "Theta Eff vs Phi", phi_bins, -180, 180);
    m_histThetaPhiDist->GetXaxis()->SetTitle(xAxisPhi);
    m_histThetaPhiDist->GetYaxis()->SetTitle("# reconstructed tracks");
    /* Histogram for correctly reconstructed tracks in Theta */
    m_histThetaRecon = new TH1D("Thetarecon", "Theta of reconstructed tracks", theta_bins, 17, 150);
    m_histThetaRecon->GetXaxis()->SetTitle(xAxisTheta);
    m_histThetaRecon->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histProjectedThetaRecon = new TH1D("ProjThetarecon", "Projected Theta of reconstructed tracks", 181, 0, 180);
    m_histProjectedThetaRecon->GetXaxis()->SetTitle(xAxisTheta);
    m_histProjectedThetaRecon->GetYaxis()->SetTitle("# reconstructed tracks");
    /* Histogram for efficiency vs Theta */
    m_histEffTheta = new TH1D("EffTheta", "Efficiency vs Theta", theta_bins, 17, 150);
    m_histEffTheta->GetXaxis()->SetTitle(xAxisTheta);
    m_histEffTheta->GetYaxis()->SetTitle("#epsilon");
    m_histEffProjectedTheta = new TH1D("EffProjTheta", "Efficiency vs Projected Theta", 181, 0, 180);
    m_histEffProjectedTheta->GetXaxis()->SetTitle(xAxisTheta);
    m_histEffProjectedTheta->GetYaxis()->SetTitle("#epsilon");
    /* Histogram for Theta-reconstruction-efficiency vs Phi */
    m_histEffThetaPhi = new TH1D("EffThetaPhi", "Theta Efficiency vs Phi", phi_bins, -180, 180);
    m_histEffThetaPhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histEffThetaPhi->GetYaxis()->SetTitle("#epsilon");
    /* Missed hit distribution */
    m_histMissedTheta = new TH1D("MissedHitTheta", "Missing Hits vs Theta", theta_bins, 17, 150);
    m_histMissedPhi = new TH1D("MissedHitPhi", "Missing Hits vs Phi", phi_bins, -180, 180);

    /* Fake plots */
    fakeDir = m_rootFile->mkdir("Fake");
    fakeDir->cd();
    /* Histogram for fake rate vs pT (only in phi while reconstruction in theta is not proved) */
    m_histPtFakePhi = new TH1D("pTFakePhi", "Fake rate vs pT (in Phi)", pt_bins, 0.02, 2.0);
    m_histPtFakePhi->GetXaxis()->SetTitle(xAxisPt);
    m_histPtFakePhi->GetYaxis()->SetTitle("# fake tracks");
    /* Histogram for fake rate vs Phi */
    m_histFakePhi = new TH1D("PhiFake", "Fake rate vs Phi", phi_bins, -180, 180);
    m_histFakePhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histFakePhi->GetYaxis()->SetTitle("Number of Fake Tracks");
    /* Histogram for fake rate vs Theta */
    m_histFakeTheta = new TH1D("ThetaFake", "Fake rate vs Theta", theta_bins, 17, 150);
    m_histFakeTheta->GetXaxis()->SetTitle(xAxisTheta);
    m_histFakeTheta->GetYaxis()->SetTitle("# fake tracks");
    /* Histogram for average fake rate */
    m_histPtAverageFake = new TH1D("AverageFakepT", "Average # fake tracks vs pT", pt_bins, 0.02, 2.0);
    m_histPtAverageFake->GetXaxis()->SetTitle(xAxisPt);
    m_histPtAverageFake->GetYaxis()->SetTitle("Average # fake tracks");
    m_histPhiAverageFake = new TH1D("AverageFakePhi", "Average Number of Fake Tracks vs Phi", phi_bins, -180, 180);
    m_histPhiAverageFake->GetXaxis()->SetTitle(xAxisPhi);
    m_histPhiAverageFake->GetYaxis()->SetTitle("Average # fake tracks");
    m_histThetaAverageFake = new TH1D("AverageFakeTheta", "Average Number of Fake Tracks vs Theta", theta_bins, 17, 150);
    m_histThetaAverageFake->GetXaxis()->SetTitle(xAxisTheta);
    m_histThetaAverageFake->GetYaxis()->SetTitle("Average # fake tracks");
    /* Histogram for fake rate vs pT (only in phi while reconstruction in theta is not proved) */
    m_histPtFake = new TH1D("pTFake", "Fake rate vs pT", pt_bins, 0.02, 2.0);
    m_histPtFake->GetXaxis()->SetTitle("Fake");
    m_histPtFake->GetYaxis()->SetTitle("pT [GeV]");

    /* ROI plots */
    roiDir = m_rootFile->mkdir("ROI");
    roiDir->cd();
    /* Distance difference in Phi */
    m_histROIDiffPhi = new TH1D("ROIDiffPhi", "Difference in Phi", 500, 0.0, 0.3);
    m_histROIDiffPhi->GetXaxis()->SetTitle("distance [cm]");
    m_histROIDiffPhiPx = new TH1D("ROIDiffPhiPx", "Difference in Phi", 60, 0.0, 60.0);
    m_histROIDiffPhiPx->GetXaxis()->SetTitle("distance [px]");
    m_histROIDiffTheta = new TH1D("ROIDiffTheta", "Difference in Theta", 100, 0.0, 1.0);
    m_histROIDiffTheta->GetXaxis()->SetTitle("distance [cm]");
    m_histROIDiffThetaPx = new TH1D("ROIDiffThetaPx", "Difference in Phi", 60, 0.0, 60.0);
    m_histROIDiffThetaPx->GetXaxis()->SetTitle("distance [px]");
    /* Plot occupancy in PXD */
    m_histROIPXD_l1 = new TH2D("PXDROIL1", "Occupancy and ROIs in PXD", 768 * 2, 0.0, 4.48 * 2.0,
                               (250 + 20) * 8, 0.0, 8 * (1.20 + 0.20));
    m_histROIPXD_l1->GetXaxis()->SetTitle("");
    m_histROIPXD_l2 = new TH2D("PXDROIL2", "Occupancy and ROIs in PXD", 768 * 2, 0.0, 6.144 * 2.0,
                               (250 + 20) * 12, 0.0, 1 * (1.25 + 0.20));
    m_histROIPXD_l2->GetXaxis()->SetTitle("");
  }

  B2INFO("SVDHoughtracking initilized");
}

void
SVDHoughtrackingModule::beginRun()
{
  /* Clear total clock cycles */
  totClockCycles = 0.0;

  /* Init some event variables */
  runNumber = 0;
  validEvents = 0;

  /* Cache PXD sensors */
  pxd_sensors.clear();
  /*VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::PXD)) {
    BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
      BOOST_FOREACH(VxdID sensor, geo.getSensors(ladder)) {
      }
    }
  }*/
  /* Cache SVD sensors for faster lookup */
  svd_sensors.clear();
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::SVD)) {
    BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
      BOOST_FOREACH(VxdID sensor, geo.getSensors(ladder)) {
        svd_sensors[sensor] = Sensor();
      }
    }
  }
}

void
SVDHoughtrackingModule::endRun()
{
  TString xAxisPt = "pT [GeV]";
  TString xAxisPhi = "#varphi [deg]";
  TString xAxisTheta = "#theta [deg]";

  if (m_printStatistics) {
    B2INFO("End of Run: Hough Statistics: ");
    B2INFO("  Events processed: " << runNumber);
    B2INFO("  Reconstructed tracks: " << allTracks);
    B2INFO("  Total tracks found: " << totTracks);
    B2INFO("  Track total efficiency: " << curTrackEff / ((double) validEvents));
    B2INFO("  Track Theta efficiency: " << curTrackEffN / ((double) validEvents));
    B2INFO("  Track Phi efficiency: " << curTrackEffP / ((double) validEvents));
    B2INFO("  Fake Rate: " << totFakeTracks / ((double) validEvents));
    B2INFO("  Total time: " << totClockCycles << " ms");
    B2INFO("  Average time: " << totClockCycles / ((double) validEvents) << " ms");
    B2INFO("  Min R: " << minR << " Max R: " << maxR);

    if (m_PXDExtrapolation) {
      B2INFO("  Average hits found: " << totROITrueHits / ((double) validEvents));
      B2INFO("  Average Hits in ROIs: " << ((double) curHitsInROIs) / ((double) validEvents));
      B2INFO("  ROI efficiency: " << curROIEff / ((double) validEvents));
      B2INFO("  DRF: " << ((double) totROITrueHits) / ((double) curHitsInROIs));
    } else {
      B2INFO("  Average hits found: " << "disabled");
      B2INFO("  Average Hits in ROIs: " << "disabled");
      B2INFO("  ROI efficiency: " << "disabled");
      B2INFO("  DRF: " << "disabled");
    }
  }

  /* Statistics output */
  if (!m_statisticsFileName.empty()) {
    /* Open file stream */
    statout.open(m_statisticsFileName, ofstream::out | ofstream::app);

    /* Check if stat file already exists, if not write legend */
    if (statout.tellp() == 0) {
      statout << "#Hash" << "\t" << "CritN" << "\t" << "CritP" << "\t" << "RectN" << "\t" << "RectP"
              << "\t" << "EffP" << "\t" << "EffN" << "\t" << "EffTot" << "\t" << "Fake" << "\t" << "ROI-Eff"
              << "\t" << "DRF" << endl;
    }

    if (m_useHashPurify) {
      statout << 1 << "\t";
    } else {
      statout << 0 << "\t";
    }

    statout << m_critIterationsN << "\t" << m_critIterationsP << "\t"
            << boost::format("%1.3f") % (double) m_rectSizeN << "\t"
            << boost::format("%1.3f") % (double) m_rectSizeP << "\t"
            << boost::format("%1.3f") % (curTrackEffP / (double) validEvents) << "\t"
            << boost::format("%1.3f") % (curTrackEffN / (double) validEvents) << "\t"
            << boost::format("%1.3f") % (curTrackEff / (double) validEvents) << "\t"
            << (totFakeTracks / ((double) validEvents)) << "\t"
            << boost::format("%1.3f") % (curROIEff / ((double) validEvents)) << "\t"
            << boost::format("%1.3f") % (((double) totROITrueHits) / ((double) curHitsInROIs)) << "\t"
            << endl;

    /* Close Statistic output */
    statout.close();
  }

  /* Fill efficiency histograms */
  if (!m_rootFilename.empty()) {
    m_rootFile->cd("Efficiency"); /* Change to Efficiency directory */

    *m_histPtEffPhi = (*m_histPtPhiRecon) / (*m_histPtDist);
    m_histPtEffPhi->SetName("pTEffPhi");
    m_histPtEffPhi->SetTitle("Efficiency vs pT (in Phi)");
    m_histPtEffPhi->GetXaxis()->SetTitle(xAxisPt);
    m_histPtEffPhi->GetXaxis()->SetRangeUser(0.0, 2.0);
    m_histPtEffPhi->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histPtEffTheta = (*m_histPtThetaRecon) / (*m_histPtDist);
    m_histPtEffTheta->SetName("pTEffTheta");
    m_histPtEffTheta->SetTitle("Efficiency vs pT (in Theta)");
    m_histPtEffTheta->GetXaxis()->SetTitle(xAxisPt);
    m_histPtEffTheta->GetXaxis()->SetRangeUser(0.0, 2.0);
    m_histPtEffTheta->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histPtEff = (*m_histPtRecon) / (*m_histPtDist);
    m_histPtEff->SetName("pTEff");
    m_histPtEff->SetTitle("Efficiency vs pT");
    m_histPtEff->GetXaxis()->SetTitle(xAxisPt);
    m_histPtEff->GetXaxis()->SetRangeUser(0.0, 2.0);
    m_histPtEff->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histEffPhi = (*m_histPhiRecon) / (*m_histPhiDist);
    m_histEffPhi->SetName("EffPhi");
    m_histEffPhi->SetTitle("Efficiency vs Phi");
    m_histEffPhi->GetXaxis()->SetTitle(xAxisPhi);

    *m_histEffTheta = (*m_histThetaRecon) / (*m_histThetaDist);
    m_histEffTheta->SetName("EffTheta");
    m_histEffTheta->SetTitle("Efficiency vs Theta");
    m_histEffTheta->GetXaxis()->SetTitle("#theta [deg]");
    m_histEffTheta->GetYaxis()->SetTitle("#epsilon");
    m_histEffTheta->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histEffThetaPhi = (*m_histThetaPhiDist) / (*m_histPhiDist);
    m_histEffThetaPhi->SetName("EffThetaPhi");
    m_histEffThetaPhi->SetTitle("Theta Efficiency vs Phi");
    m_histEffThetaPhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histEffThetaPhi->GetYaxis()->SetTitle("#epsilon");
    m_histEffThetaPhi->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histEffProjectedTheta = (*m_histProjectedThetaRecon) / (*m_histProjectedThetaDist);
    m_histEffProjectedTheta->SetName("EffRrojTheta");
    m_histEffProjectedTheta->SetTitle("Efficiency vs projected theta");
    m_histEffProjectedTheta->GetXaxis()->SetTitle("Projected #theta [deg]");

    m_rootFile->cd("Fake"); /* Change to Fake directory */
    *m_histPtAverageFake = (*m_histPtFakePhi) / (*m_histPtDist);
    m_histPtAverageFake->SetName("AverageFakepT");
    m_histPtAverageFake->SetTitle("Average # fake tracks vs pT");
    m_histPtAverageFake->GetXaxis()->SetTitle(xAxisPt);
    m_histPtAverageFake->GetYaxis()->SetTitle("Average # fake tracks");
    m_histPtAverageFake->GetXaxis()->SetRangeUser(0.0, 2.0);

    *m_histPhiAverageFake = (*m_histFakePhi) / (*m_histPhiDist);
    m_histPhiAverageFake->SetName("AverageFakePhi");
    m_histPhiAverageFake->SetTitle("Average Number of Fake Tracks vs Phi");
    m_histPhiAverageFake->GetXaxis()->SetTitle(xAxisPhi);
    m_histPhiAverageFake->GetYaxis()->SetTitle("Average # fake tracks");

    *m_histThetaAverageFake = (*m_histFakeTheta) / (*m_histThetaDist);
    m_histThetaAverageFake->SetName("AverageFakeTheta");
    m_histThetaAverageFake->SetTitle("Average Number of Fake Tracks vs Theta");
    m_histThetaAverageFake->GetXaxis()->SetTitle(xAxisTheta);
    m_histThetaAverageFake->GetYaxis()->SetTitle("Average # of fake tracks");

  }
}

void
SVDHoughtrackingModule::event()
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<SVDCluster> storeSVDCluster(m_storeSVDClusterName);
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);

  TVector3 vec, abs_pos;
  float u, v;
  std::map<int, TVector3>::iterator iter;

  if (!storeHoughCluster.isValid())
    storeHoughCluster.create();
  else
    storeHoughCluster.getPtr()->Clear();

  /* Reset cluster index variables */
  p_idx = 0;
  n_idx = 0;

  ++runNumber;
  /* Use FPGA clustering algorithm or True hits*/
  if (m_useFPGAClusters || m_useTrueHitClusters || m_useSimHitClusters) {
    /* Clear everything first */
    p_clusters.clear();
    n_clusters.clear();
    p_hough.clear();
    p_houghCand.clear();
    p_houghTrackCand.clear();
    n_hough.clear();
    n_houghCand.clear();
    n_houghTrackCand.clear();

    if (m_useTrueHitClusters) {
      convertTrueHits();
    } else if (m_useSimHitClusters) {
      /* This is for background simulation, so add sim and true hits */
      convertSimHits();
      //convertTrueHits();
      //mixTrueSimHits();
    } else {
      clusterStrips();
    }

    /* Save hits for FPGA conversion */
    if (m_saveHits) {
      saveHits();
    }

    /* Save hits for FPGA conversion */
    if (m_saveStrips) {
      saveStrips();
    }

    /* Create plots for analysing the clusters */
    if (m_analyseFPGAClusters) {
      analyseClusterStrips();
    }

    /* List clusters */
    //printClusters(n_clusters, true);
    //printClusters(p_clusters, false);

    /* Run the tracking pipeline */
    if (m_fullTrackingPipeline) {
      trackingPipeline();
    }

    if (m_compareMCParticle) {
      trackAnalyseMCParticle();
    }

    /* Deal with the extrapolation to PXD */
    if (m_PXDExtrapolation) {
      pxdExtrapolation();
      if (m_PXDTbExtrapolation) {
        createTbResiduals();
      } else {
        createResiduals();
      }
    }

    /* Print the customazible summary */
    printCustomSummary();
  } else {
    int num_simhit = storeSVDSimHit.getEntries();
    if (m_histSimHits) {
      m_histSimHits->Fill(validEvents, num_simhit);
    }

    for (int i = 0; i < num_simhit; ++i) {
      vec = storeSVDSimHit[i]->getPosIn();
      VxdID sensorID = storeSVDSimHit[i]->getSensorID();
      static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.get(sensorID);
      abs_pos = info.pointToGlobal(storeSVDSimHit[i]->getPosIn());
      //   storeHoughCluster.appendNew(SVDHoughTrack(i, abs_pos));

      B2DEBUG(250, "Simhits: " << storeSVDSimHit[i]->getPosIn().Px()
              << " " << storeSVDSimHit[i]->getPosIn().Py()
              << " " << storeSVDSimHit[i]->getPosIn().Pz()
              << " VSize: " << info.getVSize()
              << " Position: " << abs_pos.Px()
              << " " << abs_pos.Py()
              << " " << abs_pos.Pz());
    }

    /* Run the coordinate clustering */
    clustering(false); /* SVD */
    clustering(true); /* PXD */

    /* Create sector map */
    create_sector_map();
    /* Use straight PXD extrapolation */
    pxdStraightExtrapolation();
    /* Run the analyzer */
    analyseExtrapolatedHits();


    B2DEBUG(200, "Number of SVD Cluster candidates " << cluster_map.size())
    for (iter = cluster_map.begin(); iter != cluster_map.end(); ++iter) {
      storeHoughCluster.appendNew(SVDHoughCluster(iter->first, iter->second));
      B2DEBUG(200, "Cluster candidate " << iter->first << ": " << iter->second.Px()
              << " " << iter->second.Py()
              << " " << iter->second.Pz());
    }

#ifdef INJECT_GHOSTS
    /* Add ghosts from dead strips */
    iter = cluster_map.end();

    B2DEBUG(200, "Number of SVD Cluster candidates " << cluster_map.size())
    for (iter = cluster_map.begin(); iter != cluster_map.end(); ++iter) {
      storeHoughCluster.appendNew(SVDHoughCluster(iter->first, iter->second));
      B2DEBUG(200, "Cluster candidate " << iter->first << ": " << iter->second.Px()
              << " " << iter->second.Py()
              << " " << iter->second.Pz());
    }
#endif

    /* Interface to Hough tracking */
    //fullHoughTracking();

    if (m_PXDExtrapolation) {
      pxdExtrapolation();
      if (m_PXDTbExtrapolation) {
        createTbResiduals();
      } else {
        createResiduals();
      }
    }

    /* Uncomment to stop here */
    return;

    /* For SVD Clusters */
    int num_svdClusters = storeSVDCluster.getEntries();
    if (m_useClusters && storeSVDCluster.isValid() && num_svdClusters > 0) {
      for (int i = 0; i < num_svdClusters / 2; i = i + 2) {
        bool isU =  storeSVDCluster[i]->isUCluster();
        VxdID sensorID = storeSVDCluster[i]->getSensorID();

        if (sensorID == storeSVDCluster[i + 1]->getSensorID() &&
            isU != storeSVDCluster[i + 1]->isUCluster()) {
          if (isU) {
            u = storeSVDCluster[i]->getPosition();
            v = storeSVDCluster[i + 1]->getPosition();
          } else {
            v = storeSVDCluster[i]->getPosition();
            u = storeSVDCluster[i + 1]->getPosition();
          }

          B2DEBUG(200, "Cluster Info for Sensor " << sensorID << " local Pos: " << u << " " << v);
        }

        if (isU) {
          B2DEBUG(200, "Cluster Info: Num = " << i
                  << " Sensor = " << storeSVDCluster[i]->getSensorID()
                  << " Position = " << "u"
                  << " " << storeSVDCluster[i]->getPosition());
        } else {
          B2DEBUG(200, "Cluster Info: Num = " << i
                  << " Sensor = " << storeSVDCluster[i]->getSensorID()
                  << " Position = " << "v"
                  << " " << storeSVDCluster[i]->getPosition());
        }
      }
    }

    /* test */
    if (!m_svdPosName.empty()) {
      save_hits();
    } else if (!m_svdStripName.empty()) {
    }

    //print_num();
  }
}

/*
 * Convert given integer into Verilog 32 bit value
 */
char*
SVDHoughtrackingModule::convertInt(int32_t n)
{
  size_t size = 256;
  static char* buf = NULL;
  if (buf == NULL) {
    if ((buf = (char*) malloc(size * sizeof(char))) == NULL) {
      perror("Error: malloc() ");
      return NULL;
    }
  }
  memset(buf, 0, size);

  if (n < 0) {
    snprintf(buf, size, "-32'sd%i", -1 * n);
  } else {
    snprintf(buf, size, "32'sd%i", n);
  }

  return (buf);
}

void
SVDHoughtrackingModule::saveHits()
{
  uint32_t cnt;
  bool createLUT = true;
  bool hess = true;
  double scale = 1024.0 * 1024.0 * 1024.0;
  double r;
  double x, y, z;
  int32_t x32, y32, z32;
  short layer;
  ofstream p_pos;

  if (m_tbMapping) {
    hess = false;
    scale = 1;
  }

  /* Open file for output position writing */
  if (createLUT) {
    p_pos.open("p_clusters.v", ofstream::out);
    p_pos << "\t\t\tcase(cnt)" << endl;
    cnt = 0;

    p_pos << "\t\t\t\t32'd" << cnt << ": begin" << endl;
    p_pos << "\t\t\t\t\tx <= " << convertInt(0) << ";" << endl;
    p_pos << "\t\t\t\t\ty <= " << convertInt(0) << ";" << endl;
    p_pos << "\t\t\t\t\twr <= 0;" << endl;
    p_pos << "\t\t\t\t\tlayer <= 2'd" << 0 << ";" << endl;
    p_pos << "\t\t\t\t\tlast_out <= 0;" << endl;
    p_pos << "\t\t\t\tend" << endl;
    ++cnt;

    for (auto it = p_clusters.begin(); it != p_clusters.end(); ++it) {
      x = it->second.second.X() / Unit::um;
      y = it->second.second.Y() / Unit::um;
      z = it->second.second.Z() / Unit::um;
      layer = it->second.first.getLayerNumber() - 3; /* we start at layer 0 */

      if (hess) {
        r = ((x * x) + (y * y));
        x = x / r;
        y = y / r;
      }

      x32 = (int32_t)(x * scale);
      y32 = (int32_t)(y * scale);
      z32 = (int32_t)(z * scale);

      p_pos << "\t\t\t\t32'd" << cnt << ": begin" << endl;
      p_pos << "\t\t\t\t\tx <= " << convertInt(x32) << ";" << endl;
      p_pos << "\t\t\t\t\ty <= " << convertInt(y32) << ";" << endl;
      p_pos << "\t\t\t\t\twr <= 1;" << endl;
      p_pos << "\t\t\t\t\tlayer <= 2'd" << layer << ";" << endl;
      p_pos << "\t\t\t\t\tlast_out <= 0;" << endl;
      p_pos << "\t\t\t\tend" << endl;
      ++cnt;
    }
    p_pos << "\t\t\tendcase" << endl;
    p_pos.close();

    /* N clusters */
    p_pos.open("n_clusters.v", ofstream::out);
    p_pos << "\t\t\tcase(cnt)" << endl;
    cnt = 0;

    p_pos << "\t\t\t\t32'd" << cnt << ": begin" << endl;
    p_pos << "\t\t\t\t\tx <= " << convertInt(0) << ";" << endl;
    p_pos << "\t\t\t\t\ty <= " << convertInt(0) << ";" << endl;
    p_pos << "\t\t\t\t\twr <= 0;" << endl;
    p_pos << "\t\t\t\t\tlayer <= 2'd" << 0 << ";" << endl;
    p_pos << "\t\t\t\t\tlast_out <= 0;" << endl;
    p_pos << "\t\t\t\tend" << endl;
    ++cnt;

    for (auto it = n_clusters.begin(); it != n_clusters.end(); ++it) {
      x = it->second.second.X() / Unit::um;
      y = it->second.second.Y() / Unit::um;
      z = it->second.second.Z() / Unit::um;
      layer = it->second.first.getLayerNumber() - 3; /* we start at layer 0 */

      if (hess) {
        r = ((x * x) + (y * y));
        x = x / r;
        y = y / r;
      }

      x32 = (int32_t)(x * scale);
      y32 = (int32_t)(y * scale);
      z32 = (int32_t)(z * scale);

      p_pos << "\t\t\t\t32'd" << cnt << ": begin" << endl;
      p_pos << "\t\t\t\t\tx <= " << convertInt(x32) << ";" << endl;
      p_pos << "\t\t\t\t\ty <= " << convertInt(z32) << ";" << endl;
      p_pos << "\t\t\t\t\twr <= 1;" << endl;
      p_pos << "\t\t\t\t\tlayer <= 2'd" << layer << ";" << endl;
      p_pos << "\t\t\t\t\tlast_out <= 0;" << endl;
      p_pos << "\t\t\t\tend" << endl;
      ++cnt;
    }
    p_pos << "\t\t\tendcase" << endl;
  } else {
    p_pos.open("p_clusters.dat", ofstream::out);

    for (auto it = p_clusters.begin(); it != p_clusters.end(); ++it) {
      p_pos << it->second.second.X() << " " << it->second.second.Y() << " "
            << it->second.second.Z() << " " << endl;
    }
  }

  p_pos.close();
}

void
SVDHoughtrackingModule::save_hits()
{
  StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  StoreArray<SVDDigit>   storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<PXDTrueHit> storePXDTrueHits(m_storePXDTrueHitsName);
  StoreArray<PXDDigit>   storePXDDigits(m_storePXDDigitsName);
  RelationArray relDigitTrueHit(storeSVDDigits, storeSVDTrueHits, m_relDigitTrueHitName);

  /* Write out PXD data */
  unsigned int nPXDTruehits = storePXDTrueHits.getEntries();
  B2DEBUG(250, "Start hits processing: Number " << nPXDTruehits);
  for (unsigned int i = 0; i < nPXDTruehits; ++i) {
    of_pos << "# Sensor: " << storePXDTrueHits[i]->getSensorID() << endl;
    of_pos << storePXDTrueHits[i]->getU() / Unit::mm << " "
           <<  storePXDTrueHits[i]->getV() / Unit::mm << endl;
  }

  /* Write out PXD data */
  unsigned int nSVDTruehits = storeSVDTrueHits.getEntries();
  B2DEBUG(250, "Start hits processing: Number " << nSVDTruehits);
  for (unsigned int i = 0; i < nSVDTruehits; ++i) {
    of_pos << "# Sensor: " << storeSVDTrueHits[i]->getSensorID() << endl;
    of_pos << storeSVDTrueHits[i]->getU() / Unit::mm << " "
           <<  storeSVDTrueHits[i]->getV() / Unit::mm << endl;
  }
}

void
SVDHoughtrackingModule::terminate()
{
  B2DEBUG(250, "End of SVDHoughTrack");
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }

  of_pos.close();
  of_strip.close();
}

void SVDHoughtrackingModule::print_num()
{
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);

  /* Number of found Hough Clusters */
  int num = storeHoughCluster.getEntries();
  B2DEBUG(200, "Number of in print_num: " << num << " ID: " << storeHoughCluster[num - 1]->getClusterID());
}

/*
 * Custom clustering.
 */
void
SVDHoughtrackingModule::clustering(bool pxd)
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<PXDSimHit> storePXDSimHits(m_storePXDSimHitsName);
  TVector3 vec, abs_pos;
  int num_simhit = storeSVDSimHit.getEntries();
  int num_pxdsimhit = storePXDSimHits.getEntries();
  int cnt, i;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  sensorMap map, real_map;
  TVector3 clusterDelta(0.15, 0.15, 0.15);

  /* First convert to absolute hits and save into a map */
  if (pxd) {
    /* Clear global cluser maps */
    pxd_cluster_map.clear();
    for (i = 0; i < num_pxdsimhit; ++i) {
      vec = storePXDSimHits[i]->getPosIn();
      VxdID sensorID = storePXDSimHits[i]->getSensorID();
      static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.get(sensorID);
      abs_pos = info.pointToGlobal(storePXDSimHits[i]->getPosIn());
      pxd_cluster_map.insert(std::make_pair(i, abs_pos));
    }
  } else {
    /* Clear global cluser maps */
    svd_sensor_cluster_map.clear();
    for (i = 0; i < num_simhit; ++i) {
      vec = storeSVDSimHit[i]->getPosIn();
      VxdID sensorID = storeSVDSimHit[i]->getSensorID();
      static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.get(sensorID);
      abs_pos = info.pointToGlobal(storeSVDSimHit[i]->getPosIn());
      pos_map.insert(std::make_pair(i, abs_pos));
      sensor_cluster = (std::make_pair(sensorID, abs_pos));
      svd_sensor_cluster_map.insert(std::make_pair(i, sensor_cluster));
    }
  }

  int cluster_id;
  i = 0;
  if (!pxd) {
    svd_real_cluster_map.clear();
  }
  for (iter = pos_map.begin(); iter != pos_map.end(); ++iter) {
    TVector3 clus_pos;
    cnt = 0;
    for (inner_iter = cpy_map.begin(); inner_iter != cpy_map.end();) {
      TVector3 diff = iter->second - inner_iter->second;
      if (fabs(diff.Px()) < clusterDelta.Px() && fabs(diff.Py()) < clusterDelta.Py()
          && fabs(diff.Pz()) < clusterDelta.Pz()) {
        ++cnt;
        clus_pos += inner_iter->second;
        cluster_id = inner_iter->first;
        cpy_map.erase(inner_iter++);
      } else {
        ++inner_iter;
      }
    }

    if (cnt >= 1) {
      TVector3 div_fac((double) cnt, (double) cnt, (double) cnt);
      clus_pos.SetXYZ(clus_pos.X() / (double) cnt, clus_pos.Y() / (double) cnt,
                      clus_pos.Z() / (double) cnt);
      if (pxd) {
        pxd_cluster_map.insert(std::make_pair(i, clus_pos));
      } else {
        cluster_map.insert(std::make_pair(i, clus_pos));
        map = svd_sensor_cluster_map[cluster_id];
        real_map = std::make_pair(map.first, clus_pos);
        svd_real_cluster_map.insert(std::make_pair(i, real_map));
      }
      i++;
    }
  }
}

/*
 * Create sector map
 */
void
SVDHoughtrackingModule::create_sector_map()
{
  int i, skip;
  unsigned short next_layer;
  bool valid;
  StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  clusterSensorMap::iterator iter;
  sensorMap map, next_map;
  sensorMap map_arr[4];
  VxdID cur_id;
  TVector3 cur_vec, vec, next_vec, distance, abs_pos;

  if (!storeHoughTrack.isValid()) {
    storeHoughTrack.create();
  } else {
    storeHoughTrack.getPtr()->Clear();
  }

  for (iter = svd_real_cluster_map.begin(); iter != svd_real_cluster_map.end(); ++iter) {
    map = iter->second;
    next_layer = 4;
    i = 1;
    if (map.first.getLayerNumber() == 3) {
      B2DEBUG(250, "New Seed: " << map.first << " Coord: " << cur_vec.X()
              << " " << cur_vec.Y()
              << cur_vec.Z());
      next_map = map;
      map_arr[0] = map;
      skip = 0;
      while (next_layer <= 6) {
        next_map = get_next_hit(next_map, next_layer, &valid);
        if (!valid) {
          skip = 1;
          break;
        }
        map_arr[i] = next_map;
        ++i;
        ++next_layer;
      }
      if (!skip) {
        sector_track_fit(map_arr); /* Fit tracks */
      } else {
        B2DEBUG(250, "...skip");
      }
    }
  }
}

/*
 * Get next hit in the next layer
 */
sensorMap
SVDHoughtrackingModule::get_next_hit(sensorMap map, unsigned short layer, bool* valid)
{
  static int run = 0;
  double last_dist;
  clusterSensorMap::iterator iter;
  sensorMap closest_hit, inner_map;
  TVector3 cur_vec, vec, next_vec, distance;

  cur_vec = map.second;
  last_dist = 1e128;
  for (iter = svd_real_cluster_map.begin(); iter != svd_real_cluster_map.end(); ++iter) {
    inner_map = iter->second;

    if (inner_map.first.getLayerNumber() == layer) {
      next_vec = inner_map.second;
      distance = cur_vec - next_vec;
      if (distance.Mag() < last_dist) {
        last_dist = distance.Mag();
        closest_hit = inner_map;

        if (m_histDist && layer == 4) {
          m_histDist->Fill(last_dist);
        }
      }
    }
  }

  ++run;
  if (last_dist > dist_thres[(layer - 4)]) {
    B2DEBUG(200, " Next: " << layer << " hit " << closest_hit.first << " distance: "
            << last_dist << " Angle: " << closest_hit.second.Angle(cur_vec) << " failed: "
            << dist_thres[(layer - 4)] << " < " << last_dist);
    *valid = 0;
  } else {
    B2DEBUG(200, " Next: " << layer << " hit " << closest_hit.first << " distance: "
            << last_dist << " Angle: " << closest_hit.second.Angle(cur_vec) << " valid");
    *valid = 1;
  }

  return (closest_hit);
}

/*
 * Fit track
 */
int
SVDHoughtrackingModule::sector_track_fit(sensorMap* map)
{
  int i;
  double m1, a1, m2, a2, m_x, m_z;
  clusterSensorMap::iterator iter;
  TVector3 cur_vec, next_vec, diff;
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);

  m1 = 0;
  a1 = 0;
  m2 = 0;
  a2 = 0;
  for (i = 0; i < 2 - 1; ++i) {
    cur_vec = map[i].second;
    next_vec = map[i + 1].second;
    diff = next_vec - cur_vec;
    m_x = diff.Y() / diff.X();
    m1 += m_x;
    a1 += -1.0 * m_x * cur_vec.X() + cur_vec.Y();
    m_z = diff.Z() / diff.Y();
    m2 += m_z;
    a2 += -1.0 * m_z * cur_vec.Y() + cur_vec.Z();
  }
  /*m1 /= 4.0;
  a1 /= 4.0;
  m2 /= 4.0;
  a2 /= 4.0;*/

  storeHoughTrack.appendNew(SVDHoughTrack(m1, a1, m2, a2));
  B2DEBUG(200, "Track parameters: m1 = " << m1 << " a1 = " << a1 << " m2 = " << m2 << " a2 = " << a2);
  return (0);
}

/*
 * Run the full tracking pipeline.
 */
void
SVDHoughtrackingModule::trackingPipeline()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  TVector2 v1_s, v2_s, v3_s, v4_s;
  double rect_size, scale_y;
  vector<houghDbgPair> n_rect, p_rect;
  clock_t start, end;

  if (!storeHoughTrack.isValid()) {
    storeHoughTrack.create();
  } else {
    storeHoughTrack.getPtr()->Clear();
  }

  /* For testbeam only !! */
  if (m_tbMapping) {
    if (n_clusters.size() >= 12 || p_clusters.size() >= 12) {
      return;
    }
  }

  /* Start time measurement */
  start = clock();

  /* Hough transformation */
  houghTrafo2d(n_clusters, true, m_conformalTrafoN); /* in z, no Hess transformation */
  houghTrafo2d(p_clusters, false, m_conformalTrafoP); /* in y, with Hess */

  /*
   * Run hough trackign on P-Side
   */
  /* Initial rect size parameters for Hough trafo */
  rect_size = m_rectSizeP;
  scale_y = m_rectScaleP;
  /* Set start values */
  v1_s.Set(m_rectXP1, (scale_y * rect_size));
  v2_s.Set(m_rectXP2, (scale_y * rect_size));
  v3_s.Set(m_rectXP2, (-1.0 * scale_y * rect_size));
  v4_s.Set(m_rectXP1, (-1.0 * scale_y * rect_size));
  /* Run intercept finder */
  fastInterceptFinder2d(p_hough, false, v1_s, v2_s, v3_s, v4_s, 0, m_critIterationsP,
                        m_maxIterationsP, p_rect, m_minimumLines);
  /* Debug */
  if (m_writeHoughSectors) {
    gplotRect("dbg/p_rect.plot", p_rect);
  }

  /*
   * Run hough trackign on N-Side
   */
  /* Initial rect size parameters for Hough trafo */
  rect_size = m_rectSizeN;
  scale_y = m_rectScaleN;
  /* Set start values */
  v1_s.Set(m_rectXN1, (scale_y * rect_size));
  v2_s.Set(m_rectXN2, (scale_y * rect_size));
  v3_s.Set(m_rectXN2, (-1.0 * scale_y * rect_size));
  v4_s.Set(m_rectXN1, (-1.0 * scale_y * rect_size));
  /* Run intercept finder */
  fastInterceptFinder2d(n_hough, true, v1_s, v2_s, v3_s, v4_s, 0, m_critIterationsN,
                        m_maxIterationsN, n_rect, m_minimumLines);
  /* Debug */
  if (m_writeHoughSectors) {
    gplotRect("dbg/n_rect.plot", n_rect);
  }

  /* Print Hough list */
  printHoughCandidates();

  /* Purify candidates */
  purifyTrackCands();
  printTrackCandidates(); /* Print track candidates */

  /* Run track merger */
  if (m_useTrackMerger) {
    trackMerger();
  }

  /* Find and combine compatible sets and create real tracks */
  fac3d();

  /* End of time measurement */
  end = clock();
  totClockCycles += 1000.0 * (((double)((end - start))) / ((double)((CLOCKS_PER_SEC))));

  /* Print tracks */
  printTracks();
}

/*
 * Find and combine into 3D tracks.
 */
void
SVDHoughtrackingModule::fac3d()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  vector<unsigned int> n_idList, p_idList;
  unsigned int tracks;
  TVector2 n_tc, p_tc;
  double r, phi, theta;
  bool all = false; /* combine every track in N and P */

  /* During testbeam it is best to combine every track, because the sensors are very
   * noisy and partly broken.
   */
  if (m_tbMapping) {
    all = true;
  }

  if (!storeHoughTrack.isValid()) {
    storeHoughTrack.create();
  } else {
    storeHoughTrack.getPtr()->Clear();
  }

  tracks = 0;

  /* Phi reconstruction only */
  if (m_usePhiOnly) {
    for (auto it = p_houghTrackCand.begin(); it != p_houghTrackCand.end(); ++it) {
      p_idList = it->getIdList();
      ++tracks;
      p_tc = it->getCoord();
      r = 1.0 / (2.0 * p_tc.Y());

      /* Determine if we use tb mapping or not */
      if (m_tbMapping) {
        if (r < 0.0) {
          phi = 1.0 * p_tc.X() + M_PI / 2.0;
        } else {
          phi = (1.0 * p_tc.X() + M_PI / 2.0) - M_PI;
        }
        if (phi > M_PI) {
        } else if (phi < -1.0 * M_PI) {
          phi += 2 * M_PI;
        }
      } else {
        if (r < 0.0) {
          phi = 1.0 * p_tc.X() + M_PI / 2.0;
        } else {
          phi = (1.0 * p_tc.X() + M_PI / 2.0) - M_PI;
        }
        if (phi > M_PI) {
        } else if (phi < -1.0 * M_PI) {
          phi += 2 * M_PI;
        }
      }

      /* Radius Filter */
      if (m_useRadiusFilter) {
        if (fabs(r) > m_radiusThreshold) {
          storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, 0.0));
        }
      } else {
        storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, 0.0));
      }
    }

    return;
  }

  /* Theta reconstruction only */
  if (m_useThetaOnly) {
    for (auto it = n_houghTrackCand.begin(); it != n_houghTrackCand.end(); ++it) {
      ++tracks;
      n_tc = it->getCoord();

      /* Determine if we use tb mapping or not */
      if (m_tbMapping) {
        if (n_tc.X() > 0.0) {
          theta = n_tc.X(); // (M_PI) - n_tc.X();
        } else {
          theta = -1.0 * n_tc.X(); //(M_PI / 2) - n_tc.X();
        }
      } else {
        if (n_tc.X() > 0.0) {
          theta = n_tc.X(); // (M_PI) - n_tc.X();
        } else {
          theta = -1.0 * n_tc.X(); //(M_PI / 2) - n_tc.X();
        }
      }

      storeHoughTrack.appendNew(SVDHoughTrack(tracks, 0.0, 0.0, theta));
    }

    return;
  }

  for (auto it = n_houghTrackCand.begin(); it != n_houghTrackCand.end(); ++it) {
    for (auto it_in = p_houghTrackCand.begin(); it_in != p_houghTrackCand.end(); ++it_in) {
      n_idList = it->getIdList();
      p_idList = it_in->getIdList();
      //if ((it->getHash() == it_in->getHash() && it->getHitSize() == it_in->getHitSize()) || all) {
      if (compareList(p_idList, n_idList) || all) {
        ++tracks;
        n_tc = it->getCoord();
        p_tc = it_in->getCoord();
        r = 1.0 / (2.0 * p_tc.Y());


        if (m_straightTracks && all) {
          storeHoughTrack.appendNew(SVDHoughTrack(p_tc.X(), p_tc.Y(), n_tc.X(), n_tc.Y()));
          return;
        }

        /* Determine if we use tb mapping or not */
        if (m_tbMapping) {
          if (r < 0.0) {
            phi = 1.0 * p_tc.X() + M_PI / 2.0;
          } else {
            phi = (1.0 * p_tc.X() + M_PI / 2.0) - M_PI;
          }
          if (phi > M_PI) {
            phi -= 2.0 * M_PI;
          } else if (phi < -1.0 * M_PI) {
            phi += 2.0 * M_PI;
          }

          if (n_tc.X() > 0.0) {
            theta = n_tc.X(); // (M_PI) - n_tc.X();
          } else {
            theta = n_tc.X(); //(M_PI / 2) - n_tc.X();
          }

          /* Radius Filter */
          if (m_useRadiusFilter) {
            if (fabs(r) > m_radiusThreshold) {
              storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, theta));
            }
          } else {
            storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, theta));
          }
        } else {
          if (r < 0.0) {
            phi = 1.0 * p_tc.X() + M_PI / 2.0;
          } else {
            phi = (1.0 * p_tc.X() + M_PI / 2.0) - M_PI;
          }
          if (phi > M_PI) {
          } else if (phi < -1.0 * M_PI) {
            phi += 2 * M_PI;
          }

          if (n_tc.X() > 0.0) {
            theta = -1.0 * n_tc.X(); // (M_PI) - n_tc.X();
          } else {
            theta = -1.0 * n_tc.X(); //(M_PI / 2) - n_tc.X();
          }

          /* Radius Filter */
          if (m_useRadiusFilter) {
            if (fabs(r) > m_radiusThreshold) {
              storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, theta));
            }
          } else {
            storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, theta));
          }
        }
      }
    }
  }
}

/*
 * Merge duplicated tracks (means tracks with difference of m_mergeThreshold).
 */
void
SVDHoughtrackingModule::trackMerger()
{
  int cnt;
  //unsigned int id;
  vector<unsigned int> idList;
  TVector2 n_tc, p_tc, p_tc_in;
  double r, phi;

  std::vector<SVDHoughTrackCand> n_houghTrackCandCpy;
  std::vector<SVDHoughTrackCand> p_houghTrackCandCpy;
  std::vector<SVDHoughTrackCand> n_houghTrackCandMerged;
  std::vector<SVDHoughTrackCand> p_houghTrackCandMerged;

  n_houghTrackCandCpy = n_houghTrackCand;
  p_houghTrackCandCpy = p_houghTrackCand;

  B2DEBUG(200, " Size of cand list before: " << p_houghTrackCandCpy.size());
  for (auto it = p_houghTrackCandCpy.begin(); it != p_houghTrackCandCpy.end(); ++it) {
    idList = it->getIdList();
    p_tc = it->getCoord();
    r = 1.0 / (2.0 * p_tc.Y());
    phi =  p_tc.X();
    B2DEBUG(250, "  Phi: " << phi << " R: " << r);
  }

  //id = 0;
  while (p_houghTrackCandCpy.size() > 0) {
    auto it = p_houghTrackCandCpy.begin();
    idList = it->getIdList();
    p_tc = it->getCoord();
    r = 1.0 / (2.0 * p_tc.Y());
    phi = p_tc.X();
    cnt = 1;

redo:
    for (auto it_in = (p_houghTrackCandCpy.begin() + 1); it_in != p_houghTrackCandCpy.end(); ++it_in) {
      p_tc_in = it_in->getCoord();
      if (fabs(p_tc.X() - p_tc_in.X()) < m_mergeThreshold) {
        phi += p_tc_in.X();
        ++cnt;
        p_houghTrackCandCpy.erase(it_in);
        goto redo;
      }
    }
    if (p_houghTrackCandCpy.size() > 0) {
      p_houghTrackCandCpy.erase(it);
    }

    /* Add to list */
    p_houghTrackCandMerged.push_back(SVDHoughTrackCand(idList, TVector2(phi / ((double) cnt), p_tc.Y())));
  }

  B2DEBUG(200, "Size of cand list after: " << p_houghTrackCandMerged.size());
  for (auto it = p_houghTrackCandMerged.begin(); it != p_houghTrackCandMerged.end(); ++it) {
    idList = it->getIdList();
    p_tc = it->getCoord();
    phi =  p_tc.X();
    B2DEBUG(250, "  " << phi);
  }

  p_houghTrackCand = p_houghTrackCandMerged;
}

/*
 * Interface to the old Hough tracking pipeline.
 */
void
SVDHoughtrackingModule::printTracks()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  int nTracks;
  double r, phi, theta;
  double m1, m2, a1, a2;

  nTracks = storeHoughTrack.getEntries();

  if (m_printTrackInfo) {
    B2INFO("Found tracks: ");
  } else {
    B2DEBUG(1, "Found tracks: ");
  }
  for (int i = 0; i < nTracks; ++i) {
    if (storeHoughTrack[i]->getTrackType()) {
      m1 = storeHoughTrack[i]->getTrackM1();
      a1 = storeHoughTrack[i]->getTrackA1();
      m2 = storeHoughTrack[i]->getTrackM2();
      a2 = storeHoughTrack[i]->getTrackA2();
      if (m_printTrackInfo) {
        cout << "  Straight Track [ " << i << " ] M1: " << m1 << " A1: " << a1 << " M2: " << m2
             << " A2: " << a2 << endl;
      } else {
        B2DEBUG(1, "  Straight Track [ " << i << " ] M1: " << m1 << " A1: " << a1 << " M2: " << m2
                << " A2: " << a2);
      }
    } else {
      r = storeHoughTrack[i]->getTrackR();
      phi = storeHoughTrack[i]->getTrackPhi();
      theta = storeHoughTrack[i]->getTrackTheta();
      if (m_printTrackInfo) {
        cout << "  Track [ " << i << " ] Radius: " << r << " Phi: " << phi << " (" << phi / Unit::deg << ")"
             << " Theta: " << theta << " (" << theta / Unit::deg << ")" << endl;
      } else {
        B2DEBUG(1, "  Track [ " << i << " ] Radius: " << r << " Phi: " << phi << " (" << phi / Unit::deg << ")"
                << " Theta: " << theta << " (" << theta / Unit::deg << ")");
      }
    }
    ++allTracks;
  }
}

/*
 * Interface to the old Hough tracking pipeline.
 */
void
SVDHoughtrackingModule::fullHoughTracking()
{
  int first;
  struct svd_hit cluster_hits;
  struct svd_hit* pnext, *last;
  struct found_tracks* tracks, *ptracks;
  std::map<int, TVector3>::iterator iter;

  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);

  if (!storeHoughTrack.isValid())
    storeHoughTrack.create();
  else
    storeHoughTrack.getPtr()->Clear();

  first = 1;
  /* First convert to struct svd_hits */
  for (iter = cluster_map.begin(); iter != cluster_map.end(); ++iter) {
    if (first) {
      pnext = &cluster_hits;
      first = 0;
    } else {
      if ((pnext = (struct svd_hit*) malloc(sizeof(struct svd_hit))) == NULL) {
        B2ERROR("malloc()");
      }
      last->next = pnext;
    }

    pnext->strip_id = iter->first;
    pnext->coord.x = iter->second.Px();
    pnext->coord.y = iter->second.Py();
    pnext->coord.z = iter->second.Pz();
    pnext->next = NULL;
    last = pnext;
  }

  /* Call external library */
  tracks = basf2_tracking_pipeline(&cluster_hits, 1);

  if (tracks == NULL) {
    B2DEBUG(250, "No tracks found!");
    //return;
  }

  ptracks = tracks;
  while (ptracks != NULL) {
    storeHoughTrack.appendNew(SVDHoughTrack((unsigned int) ptracks->id, (double) ptracks->track.r,
                                            (double) ptracks->track.angle, (double) ptracks->track.theta));
    ptracks = ptracks->next;
  }
}

/*
 * Take the hough tracks and to the extrapolation to the PXD
 * layer.
 */
void
SVDHoughtrackingModule::pxdStraightExtrapolation()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  double x1, x2, y1, y2, z1;
  int track_samples = 42000;
  TVector3 closest;
  VxdID closest_id;
  double m, c;
  double min_x, min_y;
  double distance, closest_distance;

  if (!storeExtrapolatedHits.isValid()) {
    storeExtrapolatedHits.create();
  } else {
    storeExtrapolatedHits.getPtr()->Clear();
  }

  if (m_PXDTbExtrapolation) {
    B2DEBUG(250, "Enter PXD testbeam extrapolation");
    for (int i = 0; i < storeHoughTrack.getEntries(); ++i) {
      SVDHoughTrack* htrack = storeHoughTrack[i];
      if (htrack->getTrackType() == false) {
        return;
      }
      for (int j = 0; j < 2; ++j) {
        double radius = tb_radius[j];
        double width = 12.5 * Unit::mm;
        double length = 44.8 * Unit::mm;

        closest_id = VxdID(j + 1, 1, j + 1);
        y1 = radius * htrack->getTrackM1() + htrack->getTrackA1();
        z1 = y1 * htrack->getTrackM2() + htrack->getTrackA2();
        closest.SetXYZ(radius, y1, z1);

        if ((fabs(z1) < (length / 2.0)) && (fabs(y1) < (width / 2.0))) {
          B2DEBUG(200, "Extrapolation to sensor: " << closest_id << " Position: " << closest.x()
                  << " " << closest.y() << " " << closest.z());
          storeExtrapolatedHits.appendNew(SVDHoughCluster(0, closest, closest_id));
        }
      }
    }
  } else {
    VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::PXD)) {
      BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
        BOOST_FOREACH(VxdID sensor, geo.getSensors(ladder)) {
          if (sensor == VxdID(1, 3, 2) || sensor == VxdID(2, 4, 2)) {
            SVDHoughTrack* htrack = storeHoughTrack[0];
            /*const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
            double width = currentSensor->getWidth();
            double length = currentSensor->getLength();
            double z_shift = currentSensor->getTranslationZ();
            double phi = currentSensor->getRotationPhi();
            double radius = currentSensor->getRadius();*/
            double width = 0.0;
            double length = 0.0;
            double z_shift = 0.0;
            double phi = 0.0;
            double radius = 0.0;
            double r = htrack->getTrackR();
            double tphi = htrack->getTrackPhi() - (M_PI / 2.0);
            double theta = -1.0 * htrack->getTrackTheta() + M_PI / 2.0;

            x1 = cos(phi) * radius - (width / 2.0) * sin(phi);
            x2 = cos(phi) * radius + (width / 2.0) * sin(phi);
            y1 = sin(phi) * radius - (width / 2.0) * cos(phi);
            y2 = sin(phi) * radius + (width / 2.0) * cos(phi);
            m = (y2 - y1) / (x2 - x1);
            c = y1 - m * x1;

            B2DEBUG(250, "PXD Sensor Info: " << ladder << " ID: " << sensor << endl
                    << "\tPhi: " << phi << endl
                    << "\tx range: " << x1 << " " << x2 << endl
                    << "\ty range: " << y1 << " " << y2 << endl
                    << "\tm: " << m << " c: " << c << endl
                    << "\tTrack: " << r << " " << tphi << " " << theta << endl);

            closest_distance = 1e+128;
            for (int i = 0; i < track_samples; ++i) {
              //double x_tmp = x1 + (double) i * ((x2 - x1) / (double) track_samples);
              double x_tmp = (double) i * ((r / 2000) / (double) track_samples);
              double y_tmp = sqrt(-1.0 * (x_tmp  * x_tmp) + 2 * x_tmp * r);
              if (::isnan(y_tmp)) {
                continue;
              }

              double x = x_tmp * cos(tphi) - y_tmp * sin(tphi);
              double y = cos(theta) * (x_tmp * sin(tphi) + y_tmp * cos(tphi));
              double z = sin(theta) * (x_tmp * sin(tphi) + y_tmp * cos(tphi));

              if (fabs(y2 - y1) <= 0.05 && z > ((length / -2.0) + z_shift) &&
                  z < ((length / 2.0) + z_shift)) {
                min_x = - (x + 2 * m * y + 2 * m * c) / (1 + (m * m));
                min_y = m * min_x + c;

                distance = sqrt((min_x - x) * (min_x - x) + (min_y - y) * (min_y - y));

                if (distance < closest_distance) {
                  closest.SetXYZ(x, y, z);
                  closest_distance = distance;
                  closest_id = sensor;
                }
              }
            }
          }
        }
        storeExtrapolatedHits.appendNew(SVDHoughCluster(0, closest, closest_id));
      }
    }

    B2DEBUG(200, "Closet position:" << closest.x() << " " << closest.y() << " " << closest.z());
  }
}

/*
 * Wrapper around the extrapolation structure. Choose between
 * different types of extrapolation algorithm.
 */
void
SVDHoughtrackingModule::pxdExtrapolation()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);

  if (!storeExtrapolatedHits.isValid()) {
    storeExtrapolatedHits.create();
  } else {
    storeExtrapolatedHits.getPtr()->Clear();
  }

  if (m_usePhiExtrapolation) {
    pxdExtrapolationPhi();
    //pxdTestExtrapolationPhi();
    analyseExtrapolatedPhi();
  } else if (m_useThetaExtrapolation) {
    pxdExtrapolationTheta();
  } else {
    //pxdSingleExtrapolation();
    pxdExtrapolationFull();

    /* Analyse extrapolated hits */
    analyseExtrapolatedFull();

    /* create ROIs */
    if (m_createROI) {
      createROI();
      /* Create a PXD hit map with ROIs for the last event */
      if (m_createPXDMap) {
        createPXDMap();
      }
    }

    analyseROI();
  }
}

/*
 * This function is for testing what influence the not precise angle
 * has on the reconstruction.
 */
void
SVDHoughtrackingModule::pxdTestExtrapolationPhi()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  double pxdShift[] = {0.35, 0.35};
  double x, y;
  double mph_x, mph_y;
  double a, b, angle;
  TVector3 mph_pos;
  TGeoHMatrix transfMatrix;
  double* rotMatrix, *transMatrix;
  int nTracks;
  VxdID sensor;

  B2DEBUG(200, "PXD Phi Extrapolation: ");

  /*
   * For testing variations in the reco track parameters only!
   */
  nTracks = 1;
  for (int i = 0; i < nTracks; ++i) {
    SVDHoughTrack* htrack = storeHoughTrack[0];
    double r = -170.667 * 1.0;
    double tphi = 0.64577 - (M_PI / 2.0);

    VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::PXD)) {
      BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
        sensor = ladder;
        sensor.setSensorNumber(1);
        const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
        transfMatrix = currentSensor->getTransformation();
        rotMatrix = transfMatrix.GetRotationMatrix();
        transMatrix = transfMatrix.GetTranslation();
        double width = currentSensor->getWidth();
        double length = currentSensor->getLength();
        double shift = pxdShift[sensor.getLayerNumber() - 1];
        double z_shift = 0.0;
        double phi = acos(rotMatrix[2]);
        double radius = sqrt((transMatrix[0] * transMatrix[0] + transMatrix[1] * transMatrix[1]));

        if (asin(rotMatrix[0]) > 0.01) {
          phi = 2 * M_PI - phi;
        }

        if (phi > M_PI) {
          phi = phi - M_PI * 2 ;
        }

        angle = tphi - phi;
        a = radius * cos(angle) - fabs(r) * (pow(fabs(sin(angle)), 2.0));
        B2DEBUG(200, "  -> a: " << a << " angle: " << angle << " Phi: " << phi
                << " tphi-phi: " << fabs((htrack->getTrackPhi() - phi)));
        if ((pow(fabs(a), 2.0) - pow(radius, 2.0)) >= 0.0 &&
            fabs((htrack->getTrackPhi() - phi)) < (M_PI / 2.0)) {
          x = a + sqrt(pow(fabs(a), 2.0) - pow(radius, 2.0));
          b = -1 * pow(fabs(x), 2.0) + (2 * x * r);
          B2DEBUG(200, "    -> x: " << x << " b: " << b);

          if (b >= 0.0) {
            y = x * sin(angle) + sqrt(b) * cos(angle);

            B2DEBUG(200, "    --> y = " << y << " a: " << a << " x: " << x << " b: " << b << " angle: " << angle);
            if (y < (shift + (width / 2.0)) && y > (shift + (width / -2.0))) {
              B2DEBUG(200, "  Found in PXD Sensor ID: " << sensor << "\tPhi: "
                      << phi / Unit::deg << " Radius: " << radius << " Shift: " << z_shift
                      << " Width: " << width << " Length: " << length);
              B2DEBUG(200, "      Candidate is: " << radius << ", " << y);
              mph_x = radius * cos(phi) - y * sin(phi);
              mph_y = radius * sin(phi) + y * cos(phi);
              B2DEBUG(200, "       -> Transformed: " << mph_x << ", " << mph_y);
              mph_pos.SetXYZ(mph_x, mph_y, 0.0);

              storeExtrapolatedHits.appendNew(SVDHoughCluster(0, mph_pos, sensor));
            }
          }
        }

        /* Debug output for transformation- and translation Matrix */
        B2DEBUG(2000, "  Transformation Matrix: " << endl
                << "\t" << boost::format("%2.3f") % rotMatrix[0]
                << "\t" << boost::format("%2.3f") % rotMatrix[1]
                << "\t" << boost::format("%2.3f") % rotMatrix[2] << endl
                << "\t" << boost::format("%2.3f") % rotMatrix[3]
                << "\t" << boost::format("%2.3f") % rotMatrix[4]
                << "\t" << boost::format("%2.3f") % rotMatrix[5] << endl
                << "\t" << boost::format("%2.3f") % rotMatrix[6]
                << "\t" << boost::format("%2.3f") % rotMatrix[7]
                << "\t" << boost::format("%2.3f") % rotMatrix[8] << endl);
        B2DEBUG(2000, "  Translation Matrix: " << endl
                << "\t" << boost::format("%2.3f") % transMatrix[0]
                << "\t" << boost::format("%2.3f") % transMatrix[1]
                << "\t" << boost::format("%2.3f") % transMatrix[2] << endl);
      }
    }
  }
}

/*
 * Full Extrapolation to PXD layers.
 */
void
SVDHoughtrackingModule::pxdExtrapolationFull()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  //double pxdShift[] = {0.35, 0.35};
  double pxdZ1Shift[] = {4.8013, -2.0013};
  double pxdZ2Shift[] = {6.1513, -2.3513};
  double minY[] = { -0.42, -0.42};
  double maxY[] = {0.83, 0.83};
  double z_shift;
  double x, y;
  double mph_x, mph_y, mph_z;
  double a, b, angle;
  TVector3 mph_pos;
  TGeoHMatrix transfMatrix;
  double* rotMatrix, *transMatrix;
  unsigned int qOH; /* quality of hit */
  int nTracks, idx;
  VxdID sensorID;
  bool found;

  B2DEBUG(200, "PXD Full Extrapolation: ");

  extrapolatedHits.clear(); /* Clear last extrapolated hits */

  nTracks = storeHoughTrack.getEntries();
  idx = 0;
  for (int i = 0; i < nTracks; ++i) {
    SVDHoughTrack* htrack = storeHoughTrack[i];
    double r = htrack->getTrackR();
    double tphi = htrack->getTrackPhi() - (M_PI / 2.0);
    double ttheta = htrack->getTrackTheta();

    B2DEBUG(200, "  Track info r:" << r << " phi: " << tphi << " theta: " << ttheta)

    /* Determine qOH */
    if (r > 500.0) {
      qOH = 2;
    } else if (r > 250.0) {
      qOH = 1;
    } else {
      qOH = 0;
    }

    found = false;
    VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::PXD)) {
      BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
        BOOST_FOREACH(VxdID sensorID, geo.getSensors(ladder)) {
          const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
          transfMatrix = currentSensor->getTransformation();
          rotMatrix = transfMatrix.GetRotationMatrix();
          transMatrix = transfMatrix.GetTranslation();
          double width = currentSensor->getWidth();
          double length = currentSensor->getLength();
          //double shift = pxdShift[sensorID.getLayerNumber() - 1];
          double phi = acos(rotMatrix[2]);
          double radius = sqrt((transMatrix[0] * transMatrix[0] + transMatrix[1] * transMatrix[1]));

          /* Get z_shift for sensor */
          if (sensorID.getLayerNumber() == 1) {
            z_shift = pxdZ1Shift[sensorID.getSensorNumber() - 1];
          } else {
            z_shift = pxdZ2Shift[sensorID.getSensorNumber() - 1];
          }

          if (asin(rotMatrix[0]) > 0.01) {
            phi = 2.0 * M_PI - phi;
          }

          if (phi > M_PI) {
            phi = phi - M_PI * 2.0;
          }

          angle = tphi - phi;
          a = radius * cos(angle) - fabs(r) * (pow(fabs(sin(angle)), 2.0));
          if ((pow(fabs(a), 2.0) - pow(radius, 2.0)) >= 0.0 &&
              fabs((htrack->getTrackPhi() - phi)) < (M_PI / 2.0)) {
            x = a + sqrt(pow(fabs(a), 2.0) - pow(radius, 2.0));
            b = -1 * pow(fabs(x), 2.0) - (2 * x * fabs(r));

            if (b >= 0.0) {
              y = x * sin(angle) + sqrt(b) * cos(angle);

              //if (y < (shift + (width / 2.0)) && y > (shift + (width / -2.0))) {
              if (y < maxY[sensorID.getLayerNumber() - 1] && y > minY[sensorID.getLayerNumber() - 1]) {
                mph_x = radius * cos(phi) - y * sin(phi);
                mph_y = radius * sin(phi) + y * cos(phi);
                mph_z = mph_y / tan(ttheta);

                if (mph_z > ((length / -2.0) + z_shift) && mph_z < ((length / 2.0) + z_shift)) {
                  B2DEBUG(200, "    ->Found in PXD Sensor ID: " << sensorID << "\tPhi: "
                          << phi / Unit::deg << " Radius: " << radius << " Shift: " << z_shift
                          << " Width: " << width << " Length: " << length);
                  B2DEBUG(200, "        Candidate is: " << radius << ", " << y);
                  B2DEBUG(200, "         -> Transformed: " << mph_x << ", " << mph_y << ", " << mph_z);
                  mph_pos.SetXYZ(mph_x, mph_y, mph_z);

                  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
                  const VXD::SensorInfoBase& info = geo.get(sensorID);
                  TVector3 local_pos = info.pointToLocal(mph_pos);

                  //extrapolatedHits.push_back(std::make_pair(sensorID, TVector2(y - shift, mph_z - z_shift)));
                  extrapolatedHits.push_back(std::make_pair(sensorID, TVector2(local_pos.X(), local_pos.Y())));

                  storeExtrapolatedHits.appendNew(SVDHoughCluster(idx, mph_pos, sensorID, qOH));
                  ++idx;
                  found = true;
                  break;
                } else {
                  B2DEBUG(200, "    !Failed in PXD Sensor ID: " << sensorID << "\tPhi: "
                          << phi / Unit::deg << " Radius: " << radius << " Shift: " << z_shift
                          << " Width: " << width << " Length: " << length);
                  B2DEBUG(200, "      Candidate was: " << radius << ", " << y);
                }
              } else {
                B2DEBUG(200, "    !Failed at y = " << y << " a: " << a << " x: " << x << " b: " << b << " angle: " << angle);
              }
            } else {
              B2DEBUG(200, "    !Failed at x: " << x << " b: " << b << " a: " << a << " angle: " << angle
                      << " Sensor-Phi: " << phi << " |tphi-phi|: " << fabs((htrack->getTrackPhi() - phi)));
            }
          } else {
            B2DEBUG(200, "    !Failed at a: " << a << " angle: " << angle << " Sensor-Phi: " << phi
                    << " |tphi-phi|: " << fabs((htrack->getTrackPhi() - phi)));
          }
        }

        if (found) {
          //break;
        }
      }
    }
  }
}

/*
 * Take the hough tracks and to the extrapolation to the PXD
 * layer.
 */
void
SVDHoughtrackingModule::pxdExtrapolationTheta()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  double pxdShift[] = {0.35, 0.35};
  double x, y;
  double mph_x, mph_y;
  double a, b, angle;
  TVector3 mph_pos;
  TGeoHMatrix transfMatrix;
  double* rotMatrix, *transMatrix;
  int nTracks;
  VxdID sensor;

  B2DEBUG(200, "PXD Phi Extrapolation: ");

  nTracks = storeHoughTrack.getEntries();
  for (int i = 0; i < nTracks; ++i) {
    SVDHoughTrack* htrack = storeHoughTrack[0];
    double r = htrack->getTrackR();
    double tphi = htrack->getTrackPhi() - (M_PI / 2.0);

    VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::PXD)) {
      BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
        sensor = ladder;
        sensor.setSensorNumber(1);
        const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
        transfMatrix = currentSensor->getTransformation();
        rotMatrix = transfMatrix.GetRotationMatrix();
        transMatrix = transfMatrix.GetTranslation();
        double width = currentSensor->getWidth();
        double length = currentSensor->getLength();
        double shift = pxdShift[sensor.getLayerNumber() - 1];
        double z_shift = 0.0;
        double phi = acos(rotMatrix[2]);
        double radius = sqrt((transMatrix[0] * transMatrix[0] + transMatrix[1] * transMatrix[1]));

        if (asin(rotMatrix[0]) > 0.01) {
          phi = 2 * M_PI - phi;
        }

        if (phi > M_PI) {
          phi = phi - M_PI * 2 ;
        }

        angle = tphi - phi;
        a = radius * cos(angle) - fabs(r) * (pow(fabs(sin(angle)), 2.0));
        B2DEBUG(200, "  -> a: " << a << " angle: " << angle << " Phi: " << phi
                << " tphi-phi: " << fabs((htrack->getTrackPhi() - phi)));
        if ((pow(fabs(a), 2.0) - pow(radius, 2.0)) >= 0.0 &&
            fabs((htrack->getTrackPhi() - phi)) < (M_PI / 2.0)) {
          x = a + sqrt(pow(fabs(a), 2.0) - pow(radius, 2.0));
          b = -1 * pow(fabs(x), 2.0) + (2 * x * r);
          B2DEBUG(200, "    -> x: " << x << " b: " << b);

          if (b >= 0.0) {
            y = x * sin(angle) + sqrt(b) * cos(angle);

            B2DEBUG(200, "    --> y = " << y << " a: " << a << " x: " << x << " b: " << b << " angle: " << angle);
            if (y < (shift + (width / 2.0)) && y > (shift + (width / -2.0))) {
              B2DEBUG(200, "  Found in PXD Sensor ID: " << sensor << "\tPhi: "
                      << phi / Unit::deg << " Radius: " << radius << " Shift: " << z_shift
                      << " Width: " << width << " Length: " << length);
              B2DEBUG(200, "      Candidate is: " << radius << ", " << y);
              mph_x = radius * cos(phi) - y * sin(phi);
              mph_y = radius * sin(phi) + y * cos(phi);
              B2DEBUG(200, "       -> Transformed: " << mph_x << ", " << mph_y);
              mph_pos.SetXYZ(mph_x, mph_y, 0.0);

              storeExtrapolatedHits.appendNew(SVDHoughCluster(0, mph_pos, sensor));
            }
          }
        }
      }
    }
  }
}

/*
 * Take the hough tracks and to the extrapolation to the PXD
 * layer.
 */
void
SVDHoughtrackingModule::pxdExtrapolationPhi()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  double pxdShift[] = {0.35, 0.35};
  double x, y;
  double mph_x, mph_y;
  double a, b, angle;
  TVector3 mph_pos;
  TGeoHMatrix transfMatrix;
  double* rotMatrix, *transMatrix;
  int nTracks;
  VxdID sensor;

  B2DEBUG(200, "PXD Phi Extrapolation: ");

  nTracks = storeHoughTrack.getEntries();
  for (int i = 0; i < nTracks; ++i) {
    SVDHoughTrack* htrack = storeHoughTrack[0];
    double r = htrack->getTrackR();
    double tphi = htrack->getTrackPhi() - (M_PI / 2.0);

    VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::PXD)) {
      BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
        sensor = ladder;
        sensor.setSensorNumber(1);
        const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
        transfMatrix = currentSensor->getTransformation();
        rotMatrix = transfMatrix.GetRotationMatrix();
        transMatrix = transfMatrix.GetTranslation();
        double width = currentSensor->getWidth();
        double length = currentSensor->getLength();
        double shift = pxdShift[sensor.getLayerNumber() - 1];
        double z_shift = 0.0;
        double phi = acos(rotMatrix[2]);
        double radius = sqrt((transMatrix[0] * transMatrix[0] + transMatrix[1] * transMatrix[1]));

        if (asin(rotMatrix[0]) > 0.01) {
          phi = 2 * M_PI - phi;
        }

        if (phi > M_PI) {
          phi = phi - M_PI * 2 ;
        }

        angle = tphi - phi;
        a = radius * cos(angle) - fabs(r) * (pow(fabs(sin(angle)), 2.0));
        B2DEBUG(200, "  -> a: " << a << " angle: " << angle << " Phi: " << phi
                << " tphi-phi: " << fabs((htrack->getTrackPhi() - phi)));
        if ((pow(fabs(a), 2.0) - pow(radius, 2.0)) >= 0.0 &&
            fabs((htrack->getTrackPhi() - phi)) < (M_PI / 2.0)) {
          x = a + sqrt(pow(fabs(a), 2.0) - pow(radius, 2.0));
          b = -1 * pow(fabs(x), 2.0) + (2 * x * r);
          B2DEBUG(200, "    -> x: " << x << " b: " << b);

          if (b >= 0.0) {
            y = x * sin(angle) + sqrt(b) * cos(angle);

            B2DEBUG(200, "    --> y = " << y << " a: " << a << " x: " << x << " b: " << b << " angle: " << angle);
            if (y < (shift + (width / 2.0)) && y > (shift + (width / -2.0))) {
              B2DEBUG(200, "  Found in PXD Sensor ID: " << sensor << "\tPhi: "
                      << phi / Unit::deg << " Radius: " << radius << " Shift: " << z_shift
                      << " Width: " << width << " Length: " << length);
              B2DEBUG(200, "      Candidate is: " << radius << ", " << y);
              mph_x = radius * cos(phi) - y * sin(phi);
              mph_y = radius * sin(phi) + y * cos(phi);
              B2DEBUG(200, "       -> Transformed: " << mph_x << ", " << mph_y);
              mph_pos.SetXYZ(mph_x, mph_y, 0.0);

              storeExtrapolatedHits.appendNew(SVDHoughCluster(0, mph_pos, sensor));
            }
          }
        }

        /* Debug output for transformation- and translation Matrix */
        B2DEBUG(2000, "  Transformation Matrix: " << endl
                << "\t" << boost::format("%2.3f") % rotMatrix[0]
                << "\t" << boost::format("%2.3f") % rotMatrix[1]
                << "\t" << boost::format("%2.3f") % rotMatrix[2] << endl
                << "\t" << boost::format("%2.3f") % rotMatrix[3]
                << "\t" << boost::format("%2.3f") % rotMatrix[4]
                << "\t" << boost::format("%2.3f") % rotMatrix[5] << endl
                << "\t" << boost::format("%2.3f") % rotMatrix[6]
                << "\t" << boost::format("%2.3f") % rotMatrix[7]
                << "\t" << boost::format("%2.3f") % rotMatrix[8] << endl);
        B2DEBUG(2000, "  Translation Matrix: " << endl
                << "\t" << boost::format("%2.3f") % transMatrix[0]
                << "\t" << boost::format("%2.3f") % transMatrix[1]
                << "\t" << boost::format("%2.3f") % transMatrix[2] << endl);
      }
    }
  }
}


/*
 * Take the hough tracks and to the extrapolation to the PXD
 * layer.
 */
void
SVDHoughtrackingModule::pxdSingleExtrapolation()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  double x1, x2, y1, y2;
  int track_samples = 42000;
  TVector3 closest;
  VxdID closest_id;
  double m, c;
  double min_x, min_y;
  double distance, closest_distance;

  if (m_PXDTbExtrapolation) {
    B2DEBUG(250, "Enter PXD testbeam extrapolation");
    double tb_radius[] = { -5.6, -4.8};
    SVDHoughTrack* htrack = storeHoughTrack[0];
    for (int i = 0; i < 2; ++i) {
      double width = 12.5;
      //double length = 44.8;
      //double z_shift = 0.0;
      double phi = 0.0;
      double radius = tb_radius[i];
      double r = 1.0 * htrack->getTrackR();
      double tphi = 1.0 * htrack->getTrackPhi() - (M_PI / 2.0) + 0.012;
      double theta = -0.0 * htrack->getTrackTheta() + 0.0 * M_PI / 2.0;

      x1 = cos(phi) * radius - (width / 2.0) * sin(phi);
      x2 = cos(phi) * radius + (width / 2.0) * sin(phi);
      y1 = sin(phi) * radius - (width / 2.0) * cos(phi);
      y2 = sin(phi) * radius + (width / 2.0) * cos(phi);
      m = (y2 - y1) / (x2 - x1);
      c = y1 - m * x1;

      closest_distance = 1e+128;
      for (int i = 0; i < track_samples; ++i) {
        //double x_tmp = x1 + (double) i * ((x2 - x1) / (double) track_samples);
        double x_tmp = (double) i * ((r / 1000) / (double) track_samples);
        double y_tmp = sqrt(-1.0 * (x_tmp  * x_tmp) + 2 * x_tmp * r);
        if (::isnan(y_tmp)) {
          continue;
        }

        double x = x_tmp * cos(tphi) - y_tmp * sin(tphi) - 60.0;
        double y = cos(theta) * (x_tmp * sin(tphi) + y_tmp * cos(tphi));
        double z = sin(theta) * (x_tmp * sin(tphi) + y_tmp * cos(tphi));

        if (i == 0) {
          B2INFO("Sample from: " << x << " " << y << " temp: " << x_tmp << " " << y_tmp);
        } else if (i == track_samples - 1) {
          B2INFO("Sample to: " << x << " " << y << " temp: " << x_tmp << " " << y_tmp);
        }

        min_x = - (x + 2 * m * y + 2 * m * c) / (1 + (m * m));
        min_y = m * min_x + c;

        distance = fabs(x - radius);
        if (distance < closest_distance) {
          closest.SetXYZ(x, y, z);
          closest_distance = distance;
          if (i == 1) {
            closest_id = VxdID(1, 1, 1);
          } else {
            closest_id = VxdID(2, 1, 1);
          }
        }
      }

      B2INFO("Closet position: " << closest.x() << " " << closest.y() << " " << closest.z());
      storeExtrapolatedHits.appendNew(SVDHoughCluster(0, closest, closest_id));
    }
  } else {
    VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::PXD)) {
      BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
        BOOST_FOREACH(VxdID sensor, geo.getSensors(ladder)) {
          if (sensor == VxdID(1, 3, 2) || sensor == VxdID(2, 4, 2)) {
            SVDHoughTrack* htrack = storeHoughTrack[0];
            /*const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
            double width = currentSensor->getWidth();
            double length = currentSensor->getLength();
            double z_shift = currentSensor->getTranslationZ();
            double phi = currentSensor->getRotationPhi();
            double radius = currentSensor->getRadius();*/
            double r = htrack->getTrackR();
            double width = 0.0;
            double length = 0.0;
            double z_shift = 0.0;
            double phi = 0.0;
            double radius = 0.0;
            double tphi = htrack->getTrackPhi() - (M_PI / 2.0);
            double theta = -1.0 * htrack->getTrackTheta() + M_PI / 2.0;

            x1 = cos(phi) * radius - (width / 2.0) * sin(phi);
            x2 = cos(phi) * radius + (width / 2.0) * sin(phi);
            y1 = sin(phi) * radius - (width / 2.0) * cos(phi);
            y2 = sin(phi) * radius + (width / 2.0) * cos(phi);
            m = (y2 - y1) / (x2 - x1);
            c = y1 - m * x1;

            B2INFO("PXD Sensor Info: " << ladder << " ID: " << sensor << endl
                   << "\tPhi: " << phi << endl
                   << "\tx range: " << x1 << " " << x2 << endl
                   << "\ty range: " << y1 << " " << y2 << endl
                   << "\tm: " << m << " c: " << c << endl
                   << "\tTrack: " << r << " " << tphi << " " << theta << endl);

            closest_distance = 1e+128;
            for (int i = 0; i < track_samples; ++i) {
              //double x_tmp = x1 + (double) i * ((x2 - x1) / (double) track_samples);
              double x_tmp = (double) i * ((r / 2000) / (double) track_samples);
              double y_tmp = sqrt(-1.0 * (x_tmp  * x_tmp) + 2 * x_tmp * r);
              if (::isnan(y_tmp)) {
                continue;
              }

              double x = x_tmp * cos(tphi) - y_tmp * sin(tphi);
              double y = cos(theta) * (x_tmp * sin(tphi) + y_tmp * cos(tphi));
              double z = sin(theta) * (x_tmp * sin(tphi) + y_tmp * cos(tphi));

              if (fabs(y2 - y1) <= 0.05 && z > ((length / -2.0) + z_shift) &&
                  z < ((length / 2.0) + z_shift)) {
                min_x = - (x + 2 * m * y + 2 * m * c) / (1 + (m * m));
                min_y = m * min_x + c;

                distance = sqrt((min_x - x) * (min_x - x) + (min_y - y) * (min_y - y));

                if (distance < closest_distance) {
                  closest.SetXYZ(x, y, z);
                  closest_distance = distance;
                  closest_id = sensor;
                }
              }
            }
          }
        }
        storeExtrapolatedHits.appendNew(SVDHoughCluster(0, closest, closest_id));
      }
    }

    B2DEBUG(200, "Closet position:" << closest.x() << " " << closest.y() << " " << closest.z());
  }
}

/*
 * Analyer tool for the extrapolated hits and check for its performance
 * in the pxd detector.
 */
void
SVDHoughtrackingModule::analyseExtrapolatedFull()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDTrueHit> storePXDTrueHits(m_storePXDTrueHitsName);
  PXDTrueHit* currHit;
  MCParticle* particle;
  TVector2 local_pos;
  TVector2 mph_pos;
  VxdID sensorID, mph_sensorID;
  int nPXD, nMCParticles;
  int nMcTrue;
  double distance, min_distance;
  double min_distance_u, min_distance_v;

  RelationArray mcParticlesToTrueHits(storeMCParticles, storePXDTrueHits);

  nPXD = storePXDTrueHits.getEntries();
  nMcTrue = mcParticlesToTrueHits.getEntries();
  nMCParticles = storeMCParticles.getEntries();

  B2DEBUG(200, "Analyze extrapolated hits: " << nPXD << " PXD TrueHits and "
          << extrapolatedHits.size() << " extrapolated Hits");

  if (!mcParticlesToTrueHits.isValid()) {
    B2WARNING("  Relation Array not valid!");
  }


  B2DEBUG(200, "  Number of MCParticles: " << nMCParticles);
  for (int i = 0; i < nMcTrue; ++i) {
    B2DEBUG(200, "  nMCTrueRelation: " << i);
    particle = storeMCParticles[i];
    if (particle->getMother() == NULL) {
      B2DEBUG(200, "  I'm a primary!");
    }
    for (unsigned int j = 0; j < mcParticlesToTrueHits[i].getToIndices().size(); j++) {
      B2DEBUG(200, "    Hit id: " << mcParticlesToTrueHits[i].getToIndex(j));
    }
  }


  if (nPXD == 0) {
    return;
  }

  for (auto it = extrapolatedHits.begin(); it != extrapolatedHits.end(); ++it) {
    mph_sensorID = it->first;
    mph_pos = it->second;
    min_distance = 1e+99;
    B2DEBUG(200, "  Extrapolated hit SensorID: " << mph_sensorID << " Coord: "
            << mph_pos.X() << ", " << mph_pos.Y());

    for (int i = 0; i < nPXD; ++i) {
      currHit = storePXDTrueHits[i];
      sensorID = currHit->getSensorID();

      local_pos.Set(currHit->getU(), currHit->getV());
      B2DEBUG(200, "    PXD hit SensorID: " << sensorID << " Coord: " << local_pos.X() << ", " << local_pos.Y());

      if (mph_sensorID != sensorID) {
        continue;
      }

      distance = sqrt(pow(fabs(local_pos.X() - mph_pos.X()), 2.0)
                      + pow(fabs(local_pos.Y() - mph_pos.Y()), 2.0));

      if (distance < min_distance) {
        min_distance = distance;
        min_distance_u = fabs(local_pos.X() - mph_pos.X());
        min_distance_v = fabs(local_pos.Y() - mph_pos.Y());
      }
    }

    m_histROIDiffPhi->Fill(min_distance_u);
    m_histROIDiffTheta->Fill(min_distance_v);
    m_histROIDiffPhiPx->Fill(min_distance_u / 0.005);
    m_histROIDiffThetaPx->Fill(min_distance_v / 0.0075);

    B2DEBUG(100, "  => Min distance: " << min_distance << " u (x): " << min_distance_u << " v (y): "
            << min_distance_v);
  }
}

/*
 * Analyer tool for the extrapolated hits and check for its performance
 * in the pxd detector.
 */
void
SVDHoughtrackingModule::analyseExtrapolatedPhi()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  StoreArray<PXDTrueHit> storePXDTrueHits(m_storePXDTrueHitsName);
  PXDTrueHit* currHit;
  TVector3 local_pos, pos;
  TVector3 dist, mph_pos;
  VxdID sensorID, mph_sensorID;
  int nPXD, nMPH;
  double distance, min_distance;

  nPXD = storePXDTrueHits.getEntries();
  nMPH = storeExtrapolatedHits.getEntries();

  B2DEBUG(200, "Analyze extrapolated Phi hits: " << nPXD << " PXD TrueHits and "
          << nMPH << " extrapolated Hits");

  if (nPXD == 0 || nMPH == 0) {
    return;
  }

  for (int j = 0; j < nMPH; ++j) {
    mph_pos = storeExtrapolatedHits[j]->getHitPos();
    mph_sensorID = storeExtrapolatedHits[j]->getSensorID();
    min_distance = 1e+99;

    for (int i = 0; i < nPXD; ++i) {
      currHit = storePXDTrueHits[i];
      sensorID = currHit->getSensorID();

      if (mph_sensorID.getLayerNumber() != sensorID.getLayerNumber() /*||
            mph_sensorID.getLadderNumber() != sensorID.getLadderNumber()*/) {
        continue;
      }

      local_pos.SetX(currHit->getU());
      local_pos.SetY(currHit->getV());
      //local_pos.SetZ(currHit->getW());
      local_pos.SetZ(0.0);
      B2DEBUG(200, "  SensorID: " << sensorID << " Coord: " << local_pos.X() << ", " << local_pos.Y()
              << ", " << local_pos.Z());

      /* Convert local to global position */
      static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.get(sensorID);
      pos = info.pointToGlobal(local_pos);
      B2DEBUG(200, "     Global Position: " << pos.X() << ", " << pos.Y() << ", " << pos.Z());

      dist = mph_pos - pos;
      //distance = dist.Mag();
      distance = sqrt(pow(fabs(mph_pos.X() - pos.X()), 2.0) + pow(fabs(mph_pos.Y() - pos.Y()), 2.0));

      if (distance < min_distance) {
        min_distance = distance;
      }
    }

    m_histROIDiffPhi->Fill(min_distance);
    m_histROIDiffPhiPx->Fill(min_distance / 0.005);

    B2DEBUG(100, "  => Min distance: " << min_distance << endl);
  }
}

/*
 * Analyer tool for the extrapolated hits and check for its performance
 * in the pxd detector.
 */
void
SVDHoughtrackingModule::analyseExtrapolatedHits()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  std::map<int, TVector3>::iterator iter;
  TVector3 vec, closest_vec, clus;
  double dist, closest;

  unsigned int num_hits = storeExtrapolatedHits.getEntries();
  for (unsigned int i = 0; i < num_hits; ++i) {
    TVector3 hit = storeExtrapolatedHits[i]->getHitPos();
    VxdID sensor = storeExtrapolatedHits[i]->getSensorID();
    closest = 1.0e128;
    for (iter = pxd_cluster_map.begin(); iter != pxd_cluster_map.end(); ++iter) {
      clus = iter->second;
      vec = hit - clus;
      dist = vec.Mag();
      if ((fabs(tb_radius[sensor.getLayerNumber() - 1] - clus.X())) > 0.2) {
        continue;
      }

      if (dist < closest) {
        closest = dist;
        closest_vec = vec;
      }
    }
    //if (dist < 0.5) {
    const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
    B2DEBUG(250, "Residuals for Sensor: " << sensor << " Pitch: " << currentSensor->getVPitch());
    B2DEBUG(250, "\tDx: " << fabs(closest_vec.x()) << " ("
            << fabs(closest_vec.x()) / currentSensor->getUPitch() << " px)"
            << " Dy: " <<  fabs(closest_vec.y())
            << " Dz: " << fabs(closest_vec.z()) << " (" <<  fabs(closest_vec.z()) / currentSensor->getVPitch()
            << " px)" << " Distance: " << closest);
    if (m_histROIy && m_histROIz) {
      m_histROIy->Fill(closest_vec.Y());
      m_histROIz->Fill(closest_vec.Z());
    }
    //}
  }
}

/*
 * Take the hough tracks and to the extrapolation to the PXD
 * layer.
 */
void
SVDHoughtrackingModule::createResiduals()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  std::map<int, TVector3>::iterator iter;
  TVector3 vec;
  double dist;

  for (iter = pxd_cluster_map.begin(); iter != pxd_cluster_map.end(); ++iter) {
    unsigned int num_hits = storeExtrapolatedHits.getEntries();
    for (unsigned int i = 0; i < num_hits; ++i) {
      TVector3 hit = storeExtrapolatedHits[i]->getHitPos();
      VxdID sensor = storeExtrapolatedHits[i]->getSensorID();
      TVector3 clus = iter->second;
      vec = hit - clus;
      dist = vec.Mag();
      const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
      if (dist < 0.5) {
        B2INFO("Residuals for Sensor: " << sensor);
        B2INFO("Dx: " << fabs(vec.x()) << " (" << fabs(vec.x()) / currentSensor->getUPitch() << " px)"
               << " Dy: " <<  fabs(vec.y())
               << " Dz: " << fabs(vec.z()) << " (" <<  fabs(vec.z()) / currentSensor->getVPitch()
               << " px)");
      }
    }
  }
}

/*
 * Take the hough tracks and to the extrapolation to the PXD
 * layer.
 */
void
SVDHoughtrackingModule::createTbResiduals()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  std::map<int, TVector3>::iterator iter;
  TVector3 vec;
  double dist;
  double pitch = 50 * Unit::um;

  for (iter = pxd_cluster_map.begin(); iter != pxd_cluster_map.end(); ++iter) {
    unsigned int num_hits = storeExtrapolatedHits.getEntries();
    for (unsigned int i = 0; i < num_hits; ++i) {
      TVector3 hit = storeExtrapolatedHits[i]->getHitPos();
      VxdID sensor = storeExtrapolatedHits[i]->getSensorID();
      TVector3 clus = iter->second;
      vec = hit - clus;
      dist = vec.Mag();
      if (dist < 0.5) {
        B2INFO("Residuals for Sensor: " << sensor);
        B2INFO("Dx: " << fabs(vec.x()) << " (" << fabs(vec.x()) / pitch << " px)"
               << " Dy: " <<  fabs(vec.y()) << " (" << abs(vec.y()) / pitch << " px)"
               << " Dz: " << fabs(vec.z()) << " (" <<  fabs(vec.z()) / pitch
               << " px)");
      }
    }
  }
}

/*
 * Save digits.
 */
void
SVDHoughtrackingModule::saveStrips()
{
  StoreArray<SVDDigit> storeDigits(m_storeSVDDigitsName);
  int nDigits, sample_cnt;
  SVDDigit* digit;
  VxdID sensorID, last_sensorID;
  bool new_strip;
  bool n_side, last_side;
  short strip, last_strip;
  float sample;
  float samples[6];
  ofstream of_pstrip, of_nstrip;

  /* Open file for output position writing */
  of_pstrip.open("p_strips.dat", ofstream::out);
  of_nstrip.open("n_strips.dat", ofstream::out);
  of_pstrip << "Event " << runNumber << endl;
  of_nstrip << "Event " << runNumber << endl;

  of_pstrip << "FADC 1" << endl;
  of_nstrip << "FADC 129" << endl;

  last_sensorID = VxdID(0);
  last_side = false;
  new_strip = true;
  last_strip = 0;
  sample_cnt = 0;

  nDigits = storeDigits.getEntries();
  B2DEBUG(250, "Save strips: " << nDigits);
  if (nDigits == 0) {
    return;
  }

  /* Loop over all Digits and aggregate first all samples from a
   * single strip.
   * We assume they are sorted!
   */
  for (int i = 0; i < nDigits; i++) {
    digit = storeDigits[i];
    sensorID = digit->getSensorID();
    n_side = digit->isUStrip();
    strip = digit->getCellID();
    sample = digit->getCharge();

    const SensorInfo* sensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));
    B2DEBUG(350, "  ** Width: " << (sensorInfo->getWidth() / 2.0) << " Length: "
            << (sensorInfo->getLength() / 2.0));

    /* Same strip, add up all samples */
    if (sample_cnt == 5) {
      samples[sample_cnt] = sample;
      writeStrip(sensorID, n_side ? false : true, strip, samples, n_side ? of_pstrip : of_nstrip);
      sample_cnt = 0;
      new_strip = true;
    } else {
      if (!new_strip && (last_side != n_side || last_sensorID != sensorID || last_strip != strip)) {
        B2WARNING("Strips not sorted!");
      }

      new_strip = false;
      samples[sample_cnt] = sample;
      ++sample_cnt;
    }

    last_strip = strip;
    last_side = n_side;
    last_sensorID = sensorID;
  }

  of_pstrip.close();
  of_nstrip.close();
}

/*
 * Write strips to file.
 */
void
SVDHoughtrackingModule::writeStrip(VxdID sensorID, bool n_side, short strip, float* samples, ofstream& of)
{
  short layer = sensorID.getLayerNumber();
  short strips_per_apv = 128;
  short apv, max_strips, rest_strip;
  short apv_offset = 0;

  if (m_tbMapping) {
    if (layer == 3 || !n_side) {
      max_strips = 768;
    } else {
      max_strips = 512;
    }
    B2DEBUG(3500, "\tMaximum number of strips: " << max_strips);

    switch (layer) {
      case 3:
        apv_offset = 0;
        break;
      case 4:
        if (n_side) {
          apv_offset = 7;
        } else {
          apv_offset = 6;
        }
        break;
      case 5:
        if (n_side) {
          apv_offset = 25;
        } else {
          apv_offset = 24;
        }
        break;
      case 6:
        apv_offset = 30;
        break;
    }

    apv = (strip / strips_per_apv);
    rest_strip = strip - (apv * strips_per_apv);

    /* For testbeam we need to remap layer 4 and 5 n-side */
    if (layer == 5 && n_side) {
      switch (apv) {
        case 0:
          apv = 25;
          break;
        case 1:
          apv = 24;
          break;
        case 2:
          apv = 27;
          break;
        case 3:
          apv = 26;
          break;
      }
    } else if (layer == 4 && n_side) {
      switch (apv) {
        case 0:
          apv = 7;
          break;
        case 1:
          apv = 6;
          break;
        case 2:
          apv = 9;
          break;
        case 3:
          apv = 8;
          break;
      }
    } else {
      apv += apv_offset;
    }

    /* Write to stream, format:
     * apv strip sample1 sample2 sample3 sample4 sample5 sample6
     */
    of << apv << " " << rest_strip << " " << samples[0] << " " << samples[1] << " " << samples[2] << " "
       << samples[3] << " " << samples[4] << " " << samples[5] << endl;

    B2DEBUG(250, "  Sensor: " << sensorID << " Side: " << n_side << " Strip: " << strip
            << " Charge: " << samples[0] << " " << samples[1] << " " << samples[2] << " " << samples[3]
            << " " << samples[4] << " " << samples[5] << " APV: " << apv << " Rest strip: " << rest_strip);
  } else {
    B2WARNING("Only testbeam mapping implemented so far for write strips!");
  }
}

/*
 * Top module for the Strip Cluster engine.
 */
void
SVDHoughtrackingModule::clusterStrips()
{
  StoreArray<SVDDigit> storeDigits(m_storeSVDDigitsName);
  int nDigits, sample_cnt, cluster_size, first;
  SVDDigit* digit;
  VxdID sensorID, last_sensorID;
  bool n_side, last_side;
  bool valid;
  short strip, last_strip;
  float sample;
  float samples[6];
  float cur_peak_sample;
  double pos;

  B2DEBUG(250, "\nStart Clustering" << endl);

  nDigits = storeDigits.getEntries();
  if (nDigits == 0) {
    return;
  }

  /* Clear cluster maps for p and n side */
  p_clusters.clear();
  n_clusters.clear();

  last_sensorID = VxdID(0);
  last_side = false;
  last_strip = 0;
  sample_cnt = 0;
  cluster_size = 1;
  first = 1;

  B2DEBUG(1, "Cluster info: total digits: " << nDigits << " included");

  /* Loop over all Digits and aggregate first all samples from a
   * single strip. Then run the noise filter on them. Only when
   * the noise filter accepts the strips add it to the list of
   * cluster candidates.
   */
  for (int i = 0; i < nDigits; i++) {
    digit = storeDigits[i];
    sensorID = digit->getSensorID();
    n_side = digit->isUStrip();
    strip = digit->getCellID();
    sample = digit->getCharge();
    pos = digit->getCellPosition();

    B2DEBUG(350, "  Sensor: " << sensorID << " Side: " << n_side << " Strip: " << strip
            << " Charge: " << sample << " Position: " << pos);


    if ((last_sensorID == sensorID && last_side == n_side) || first) {
      /* Same strip, add up all samples */
      if ((strip == last_strip && sample_cnt != 6) || first) {
        first = 0;
        samples[sample_cnt] = sample;
        ++sample_cnt;
      } else {
        /* New strip, so noise filter old strip first */
        valid = noiseFilter(samples, sample_cnt, &cur_peak_sample);
        if (!valid) {
          if (cluster_size > 1) {
            clusterAdd(last_sensorID, last_side, last_strip, cluster_size); /* Add cluster */
            cluster_size = 1;
          }
          continue;
        }

        if (strip == (last_strip + 1) && cluster_size <= 4) {
          peak_samples[cluster_size - 1] = cur_peak_sample;
          ++cluster_size;
        } else {
          clusterAdd(last_sensorID, last_side, last_strip, cluster_size); /* Add cluster */
          cluster_size = 1;
          peak_samples[cluster_size - 1] = cur_peak_sample;
        }

        sample_cnt = 0;
        samples[sample_cnt] = sample;
        ++sample_cnt;
      }
    } else {
      valid = noiseFilter(samples, sample_cnt, &cur_peak_sample);
      if (valid) {
        peak_samples[cluster_size - 1] = cur_peak_sample;
        clusterAdd(last_sensorID, last_side, last_strip, cluster_size); /* Add cluster */
      } else {
        peak_samples[0] = cur_peak_sample;
      }
      cluster_size = 1;
      sample_cnt = 0;
      samples[sample_cnt] = sample;
      ++sample_cnt;
      //create_strip_cluster();
    }
    last_strip = strip;
    last_side = n_side;
    last_sensorID = sensorID;
  }

  /* Last cluster */
  if (nDigits > 0) {
    if (noiseFilter(samples, sample_cnt, &cur_peak_sample)) {
      peak_samples[cluster_size - 1] = cur_peak_sample;
      clusterAdd(last_sensorID, last_side, last_strip, cluster_size); /* Add cluster */
    }
  }

  B2DEBUG(250, "End Clustering" << endl);
}

/*
 * Run the noise filter over the given numbers of samples. If
 * it fullfil the requirements true is returned.
 */
bool
SVDHoughtrackingModule::noiseFilter(float* samples, int size, float* peak_sample)
{
  int i, peak_pos;
  *peak_sample = 0.0;
  peak_pos = 0;

  if (m_disableNoiseFilter) {
    return true;
  }

  /* Find maximum sample and save position */
  for (i = 0; i < size; ++i) {
    if (*peak_sample < samples[i]) {
      *peak_sample = samples[i];
      peak_pos = i;
    }
  }

  if (peak_pos > 0 && peak_pos < (size - 1)) {
    if (samples[peak_pos - 1] > (*peak_sample / m_noiseFactor)) {
      return (true);
    }
  }

  return (false);
}

/*
 * Analyse ROIs using first the TrueHits to check if the relevant
 * Hits are inside and then go over all SimHits to also include
 * background.
 */
void
SVDHoughtrackingModule::analyseROI()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDTrueHit> storePXDTrueHits(m_storePXDTrueHitsName);
  StoreArray<PXDSimHit> storePXDSimHit(m_storePXDSimHitsName);
  PXDTrueHit* currHit;
  MCParticle* particle;
  TVector3 pos, last_pos, local_pos;
  TVector3 diff, vec, abs_pos;
  TVector2 hit_pos;
  TVector2 v1, v2;
  VxdID sensorID, roi_sensorID;
  VxdID last_sensorID;
  int i, nMCParticles, nSimHit;
  int nMcTrue, nROI;
  int foundHits;
  unsigned int corrFound;
  unsigned int primaryTracks = 0;
  unsigned int nHits;
  bool layer[2], found_layer[2];
  double dist;


  RelationArray mcParticlesToTrueHits(storeMCParticles, storePXDTrueHits);

  nMcTrue = mcParticlesToTrueHits.getEntries();
  nMCParticles = storeMCParticles.getEntries();
  nROI = pxdROI.size();

  B2DEBUG(200, "Analyze ROIs: " << nROI);

  if (!mcParticlesToTrueHits.isValid()) {
    B2WARNING("  Relation Array not valid!");
  }

  B2DEBUG(200, "  Number of MCParticles: " << nMCParticles);
  foundHits = 0;
  corrFound = 0;
  nHits = 0;
  for (int i = 0; i < nMcTrue; ++i) {
    particle = storeMCParticles[i];
    layer[0] = false;
    layer[1] = false;
    found_layer[0] = false;
    found_layer[1] = false;
    if (particle->getMother() == NULL) {
      ++primaryTracks;
      /* Iterate over all TrueHits in a detector */
      for (unsigned int j = 0; j < mcParticlesToTrueHits[i].getToIndices().size(); j++) {
        currHit = storePXDTrueHits[mcParticlesToTrueHits[i].getToIndex(j)];
        sensorID = currHit->getSensorID();
        hit_pos.Set(currHit->getU(), currHit->getV());
        layer[sensorID.getLayerNumber() - 1] = true;

        B2DEBUG(200, "    Hit id: " << mcParticlesToTrueHits[i].getToIndex(j) << " hit in sensor " << sensorID
                << " pos: " << hit_pos.X() << ", " << hit_pos.Y());
        /* Iterate over all ROIs */
        for (int k = 0; k < nROI; ++k) {
          roi_sensorID = pxdROI[k].getSensorID();
          if (roi_sensorID == sensorID) {
            v1 = pxdROI[k].getV1();
            v2 = pxdROI[k].getV2();
            B2DEBUG(200, "      ROI v1: " << v1.X() << ", " << v1.Y() << " v2: "
                    << v2.X() << ", " << v2.Y());

            if (hit_pos.Y() >= v1.X() && hit_pos.Y() <= v2.X() &&
                hit_pos.X() >= v1.Y() && hit_pos.X() <= v2.Y()) {
              found_layer[sensorID.getLayerNumber() - 1] = true;
              ++foundHits;
            }
          }
        }
      }

      if (layer[0] == true) {
        ++nHits;
      }
      if (layer[1] == true) {
        ++nHits;
      }

      /* Accept ROIs for this track if either 1 ist found in both layers, or at
       * lest two
       */
      if (found_layer[0] == true && found_layer[1] == true) {
        corrFound += 2;
      } else if (foundHits > 1) {
        corrFound += 2;
      } else if ((found_layer[0] == true || found_layer[1] == true) &&
                 (mcParticlesToTrueHits[i].getToIndices().size() < 2)) {
        corrFound += 1;
      }
    }
  }
  totROITrueHits += nHits;

  nSimHit = storePXDSimHit.getEntries();
  /* Count numbers of clustered SimHits inside ROIs */
  svd_sensor_cluster_map.clear();
  dist = 1E+50;
  for (i = 0; i < nSimHit; ++i) {
    vec = storePXDSimHit[i]->getPosIn();
    VxdID sensorID = storePXDSimHit[i]->getSensorID();
    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

    /* Convert local to global position */
    const VXD::SensorInfoBase& info = geo.get(sensorID);
    abs_pos = info.pointToGlobal(storePXDSimHit[i]->getPosIn());
    pos = abs_pos;

    if (i == 0) {
      if (i == (nSimHit - 1)) {
        for (int k = 0; k < nROI; ++k) {
          roi_sensorID = pxdROI[k].getSensorID();
          if (roi_sensorID == last_sensorID) {
            v1 = pxdROI[k].getV1();
            v2 = pxdROI[k].getV2();

            if (vec.Y() >= v1.X() && vec.Y() <= v2.X() &&
                vec.X() >= v1.Y() && vec.X() <= v2.Y()) {
              ++curHitsInROIs;
            }
          }
        }
        ++totROITrueHits;
      }
    } else {
      //diff = pos - last_pos;
      diff = abs_pos - last_pos;
      dist = diff.Mag();

      if (fabs(dist) < 0.02 && i != (nSimHit - 1)) {
      } else {
        for (int k = 0; k < nROI; ++k) {
          roi_sensorID = pxdROI[k].getSensorID();
          if (roi_sensorID == last_sensorID) {
            v1 = pxdROI[k].getV1();
            v2 = pxdROI[k].getV2();

            if (vec.Y() >= v1.X() && vec.Y() <= v2.X() &&
                vec.X() >= v1.Y() && vec.X() <= v2.Y()) {
              ++curHitsInROIs;
            }
          }
        }
        ++totROITrueHits;
      }
    }

    last_pos = abs_pos;
    last_sensorID = sensorID;
  }

  B2DEBUG(200, "  -> Total ROIs: " << nHits << " Correct found: " << corrFound);

  if (nHits > 0) {
    curROIEff += ((double) corrFound) / ((double) nHits);
  }
}

/*
 * Calculate Region of Interests (ROI).
 */
void
SVDHoughtrackingModule::createROI()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  SVDHoughCluster* currHit;
  double size_v = 12.0;
  double size_u = 160.0;
  double offset_x, offset_y;
  double x, y;
  int nExHits;
  bool fixedSize = true;
  VxdID sensorID;
  TVector3 pos, local_pos;
  TVector2 v1, v2;
  ofstream of_l1, of_l2;

  /* Clear ROI vector first */
  pxdROI.clear();

  if (m_createPXDMap) {
    of_l1.open("dbg/rois_l1.dat", ofstream::out);
    of_l2.open("dbg/rois_l2.dat", ofstream::out);
  }

  B2DEBUG(200, "Create ROIs: ");

  nExHits = storeExtrapolatedHits.getEntries();
  for (int i = 0; i < nExHits; ++i) {
    currHit = storeExtrapolatedHits[i];
    pos = currHit->getHitPos();
    sensorID = currHit->getSensorID();
    unsigned qOH = currHit->getQOH();

    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    const VXD::SensorInfoBase& info = geo.get(sensorID);
    local_pos = info.pointToLocal(pos);

    const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
    double width = currentSensor->getWidth();
    double length = currentSensor->getLength();

    /* Determine size of ROI */
    if (fixedSize) {
      size_v = 12.0;
      size_u = 160.0;
    } else {
      switch (qOH) {
        case 1:
          size_v = 10;
          size_u = 64;
          break;
        case 2:
          size_v = 8;
          size_u = 16;
          break;
      }
    }

    /* For lower left edge */
    x = local_pos.Y() - ((size_u / 2.0) * 0.0075);
    y = local_pos.X() - ((size_v / 2.0) * 0.0050);
    if (x < (length / -2.0)) {
      x = length / -2.0;
    }
    if (y < (width / -2.0)) {
      y = width / -2.0;
    }
    v1.Set(x, y);

    /* For lower left edge */
    x = local_pos.Y() + ((size_u / 2.0) * 0.0075);
    y = local_pos.X() + ((size_v / 2.0) * 0.0050);
    if (x > (length / 2.0)) {
      x = length / 2.0;
    }
    if (y > (width / 2.0)) {
      y = width / 2.0;
    }
    v2.Set(x, y);

    B2DEBUG(200, "  Local position of ROI in sensor: " << sensorID << " Coord: "
            << local_pos.Y() << " " << local_pos.X() << " v1: " <<
            v1.X() << ", " << v1.Y() << " v2: " << v2.X() << ", " << v2.Y());
    pxdROI.push_back(SVDHoughROI(sensorID, v1, v2));

    if (m_createPXDMap) {
      if (sensorID.getLayerNumber() == 1) {
        offset_x = 4.48 * (sensorID.getSensorNumber() - 1) + 4.48 / 2.0;
        offset_y = (1.25 + 0.2) * (sensorID.getLadderNumber() - 1) + 1.25 / 2.0;

        of_l1 << "set object rect from " << v1.X() + offset_x << ", "
              << v1.Y() + offset_y << " to " << v2.X() + offset_x << ", "
              << v2.Y() + offset_y << " " << "fc rgb \"green\" fs solid 0.5 behind"
              << endl;
      } else {
        offset_x = 6.144 * (sensorID.getSensorNumber() - 1) + 6.144 / 2.0;
        offset_y = (1.25 + 0.2) * (sensorID.getLadderNumber() - 1) + 1.25 / 2.0;

        of_l2 << "set object rect from " << v1.X() + offset_x << ", "
              << v1.Y() + offset_y << " to " << v2.X() + offset_x << ", "
              << v2.Y() + offset_y << " " << "fc rgb \"green\" fs solid 0.5 behind"
              << endl;
      }
    }
  }

  if (m_createPXDMap) {
    of_l1.close();
    of_l2.close();
  }
}

/*
 * For occupancy and ROI reduction studies, convert the PXD Sim hits
 * into a local format.
 */
void
SVDHoughtrackingModule::createPXDMap()
{
  StoreArray<PXDSimHit> storePXDSimHit(m_storePXDSimHitsName);
  StoreArray<PXDTrueHit> storePXDTrueHits(m_storePXDTrueHitsName);
  int i, cluster_cnt;
  int nSimHit, nTrueHit;
  double dist;
  double u, v;
  VxdID sensorID, last_sensorID;
  TVector3 pos, last_pos, local_pos;
  TVector3 diff, vec, abs_pos;
  TVector2 truehit_pos;
  ofstream of_l1, of_l2;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  TVector3 clusterDelta(0.15, 0.15, 0.15);
  PXDTrueHit* currTrueHit;

  nSimHit = storePXDSimHit.getEntries();
  nTrueHit = storePXDTrueHits.getEntries();
  if (nSimHit == 0 && nTrueHit == 0) {
    return;
  }

  of_l1.open("dbg/pxd_occ_l1.dat", ofstream::out);
  of_l2.open("dbg/pxd_occ_l2.dat", ofstream::out);

  B2DEBUG(200, "Create PXD map: " << nSimHit);

  /* First convert to absolute hits and save into a map */
  svd_sensor_cluster_map.clear();
  dist = 1E+50;
  cluster_cnt = 0;
  for (i = 0; i < nSimHit; ++i) {
    vec = storePXDSimHit[i]->getPosIn();
    VxdID sensorID = storePXDSimHit[i]->getSensorID();
    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

    /* Convert local to global position */
    const VXD::SensorInfoBase& info = geo.get(sensorID);
    abs_pos = info.pointToGlobal(storePXDSimHit[i]->getPosIn());
    pos = abs_pos;

    if (i == 0) {
      if (i == (nSimHit - 1)) {
        B2DEBUG(200, "  Add in Layer " << sensorID.getLayerNumber() << " sensor: " << sensorID << " coord: "
                << vec.X() << " " << vec.Y() << " " << vec.Z());
        if (sensorID.getLayerNumber() == 1) {
          v = 4.48 * (sensorID.getSensorNumber() - 1) + vec.Y() + 4.48 / 2.0;
          u = (1.2 + 0.2) * (sensorID.getLadderNumber() - 1) + vec.X() + 1.2 / 2.0;
          m_histROIPXD_l1->Fill(v, u);
          of_l1 << v << " " << u << endl;
        } else {
          v = 6.144 * (sensorID.getSensorNumber() - 1) + vec.Y() + 6.144 / 2.0;
          u = (1.25 + 0.2) * (sensorID.getLadderNumber() - 1) + vec.X() + 1.25 / 2.0;
          m_histROIPXD_l2->Fill(v, u);
          of_l2 << v << " " << u << endl;
        }
        ++cluster_cnt;
      }
    } else {
      //diff = pos - last_pos;
      diff = abs_pos - last_pos;
      dist = diff.Mag();
      B2DEBUG(250, "   Distance: " << dist);

      if (fabs(dist) < 0.02 && i != (nSimHit - 1)) {
      } else {
        B2DEBUG(200, "  Add in sensor: " << last_sensorID << " coord: " << vec.X() << " " << vec.Y() << " " << vec.Z()
                << " Layer " << last_sensorID.getLayerNumber() << " Ladder: " << last_sensorID.getLadderNumber()
                << " Sensor: " << last_sensorID.getSensorNumber());
        if (last_sensorID.getLayerNumber() == 1) {
          v = 4.48 * (last_sensorID.getSensorNumber() - 1) + vec.Y() + 4.48 / 2.0;
          u = (1.2 + 0.2) * (last_sensorID.getLadderNumber() - 1) + vec.X() + 1.2 / 2.0;
          m_histROIPXD_l1->Fill(v, u);
          of_l1 << v << " " << u << endl;
        } else {
          v = 6.144 * (last_sensorID.getSensorNumber() - 1) + vec.Y() + 6.144 / 2.0;
          u = (1.25 + 0.2) * (last_sensorID.getLadderNumber() - 1) + vec.X() + 1.25 / 2.0;
          m_histROIPXD_l2->Fill(v, u);
          of_l2 << v << " " << u << endl;
        }
        ++cluster_cnt;
      }
    }

    //last_pos = pos;
    last_pos = abs_pos;
    last_sensorID = sensorID;
  }

  for (int i = 0; i < nTrueHit; ++i) {
    currTrueHit = storePXDTrueHits[i];
    sensorID = currTrueHit->getSensorID();

    truehit_pos.Set(currTrueHit->getU(), currTrueHit->getV());
    B2DEBUG(200, "    Add PXD TrueHit SensorID: " << sensorID << " Coord: "
            << local_pos.X() << ", " << local_pos.Y());
    if (sensorID.getLayerNumber() == 1) {
      v = 4.48 * (sensorID.getSensorNumber() - 1) + truehit_pos.Y() + 4.48 / 2.0;
      u = (1.2 + 0.2) * (sensorID.getLadderNumber() - 1) + truehit_pos.X() + 1.2 / 2.0;
      m_histROIPXD_l1->Fill(v, u);
      of_l1 << v << " " << u << endl;
    } else {
      v = 6.144 * (sensorID.getSensorNumber() - 1) + truehit_pos.Y() + 6.144 / 2.0;
      u = (1.25 + 0.2) * (sensorID.getLadderNumber() - 1) + truehit_pos.X() + 1.25 / 2.0;
      m_histROIPXD_l2->Fill(v, u);
      of_l2 << v << " " << u << endl;
    }
    ++cluster_cnt;
  }

  of_l1.close();
  of_l2.close();
}

/*
 * To bypass the FPGA cluster algorithm, we can directly convert
 * SimHits into the cluster format.
 */
void
SVDHoughtrackingModule::convertSimHits()
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);
  const StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  int i, cluster_cnt;
  int nSimHit;
  double dist;
  float time;
  VxdID sensorID, last_sensorID;
  TVector3 pos, last_pos, local_pos;
  TVector3 diff, vec, abs_pos;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  TVector3 clusterDelta(0.15, 0.15, 0.15);

  if (!storeHoughCluster.isValid()) {
    storeHoughCluster.create();
  } else {
    storeHoughCluster.getPtr()->Clear();
  }

  nSimHit = storeSVDSimHit.getEntries();
  if (nSimHit == 0) {
    return;
  }

  B2DEBUG(250, "Convert SimHits to Clusters");

  /* First convert to absolute hits and save into a map */
  /* Clear global cluser maps */
  svd_sensor_cluster_map.clear();
  dist = 1E+50;
  cluster_cnt = 0;
  for (i = 0; i < nSimHit; ++i) {
    vec = storeSVDSimHit[i]->getPosIn();
    time = storeSVDSimHit[i]->getGlobalTime();
    VxdID sensorID = storeSVDSimHit[i]->getSensorID();
    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

    /* Don't want off trigger hits */
    if (time < 0.0f || time > 6.0f) {
      continue;
    }

    /* Convert local to global position */
    const VXD::SensorInfoBase& info = geo.get(sensorID);
    abs_pos = info.pointToGlobal(storeSVDSimHit[i]->getPosIn());
    pos = abs_pos;

    B2DEBUG(250, " Local SimHit Pos: " << vec.X() << " " << vec.Y() << " " << vec.Z()
            << " Abs pos: " << abs_pos.X() << " " << abs_pos.Y() << " " << abs_pos.Z()
            << " Time: " << time << " " << " Sensor ID: " << sensorID);

    if (i == 0) {
      if (i == (nSimHit - 1)) {
        n_clusters.insert(std::make_pair(n_idx, std::make_pair(sensorID, pos)));
        ++n_idx;
        p_clusters.insert(std::make_pair(p_idx, std::make_pair(sensorID, pos)));
        ++p_idx;
        storeHoughCluster.appendNew(SVDHoughCluster(sensorID, pos));
        ++cluster_cnt;
      }
    } else {
      //diff = pos - last_pos;
      diff = abs_pos - last_pos;
      dist = diff.Mag();
      B2DEBUG(250, "   Distance: " << dist);

      if (fabs(dist) < 0.08 && i != (nSimHit - 1)) {
      } else {
        n_clusters.insert(std::make_pair(n_idx, std::make_pair(last_sensorID, last_pos)));
        ++n_idx;
        p_clusters.insert(std::make_pair(p_idx, std::make_pair(last_sensorID, last_pos)));
        ++p_idx;
        storeHoughCluster.appendNew(SVDHoughCluster(last_sensorID, pos));
        ++cluster_cnt;
      }
    }

    //last_pos = pos;
    last_pos = abs_pos;
    last_sensorID = sensorID;
  }

  if (m_compareMCParticleVerbose) {
    B2INFO("  Total Clusters: " << cluster_cnt << " Number of Sim Hits: " << nSimHit);
  }
}

/*
 * With background simulation we need to mix sim and true hits, in case we
 * also want the true hits inside.
 */
void
SVDHoughtrackingModule::mixTrueSimHits()
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);
  const StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  int nTrueHit;
  VxdID sensorID;
  SVDTrueHit* svdTrueHit;
  TVector3 pos, local_pos;
  TVector3 diff, abs_pos, sim_pos;
  double dist;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  TVector3 clusterDelta(0.15, 0.15, 0.15);

  if (!storeHoughCluster.isValid()) {
    storeHoughCluster.create();
  } else {
    storeHoughCluster.getPtr()->Clear();
  }

  //convertSimHits(); /* Convert Simhits first */
  convertTrueHits(); /* Convert Simhits first */

  nTrueHit = storeSVDTrueHits.getEntries();
  if (nTrueHit == 0) {
    return;
  }

  B2DEBUG(250, "mixTrueSimHits: Include True hits if necessary");

  dist = 1e+99;
  /* First convert to absolute hits and save into a map */
  /* Clear global cluser maps */
  svd_sensor_cluster_map.clear();
  for (int i = 0; i < nTrueHit; ++i) {
    svdTrueHit = storeSVDTrueHits[i];
    sensorID = svdTrueHit->getSensorID();
    local_pos.SetX(svdTrueHit->getU());
    local_pos.SetY(svdTrueHit->getV());
    local_pos.SetZ(svdTrueHit->getW());

    /* Convert local to global position */
    const SensorInfo* sensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));
    pos = sensorInfo->pointToGlobal(local_pos);

    for (int j = 0; j < storeHoughCluster.getEntries(); ++j) {
      sim_pos = storeHoughCluster[j]->getHitPos();
      diff = sim_pos - pos;

      if (diff.Mag() < dist) {
        dist = diff.Mag();
      }
    }

    if (dist > 0.060) {
      B2DEBUG(250, "  Include trueHit Local Pos: " << local_pos.X() << " " << local_pos.Y() << " " << local_pos.Z()
              << " Pos: " << pos.X() << " " << pos.Y() << " " << pos.Z() << " Sensor ID: " << sensorID);

      n_clusters.insert(std::make_pair(n_idx, std::make_pair(sensorID, pos)));
      ++n_idx;
      p_clusters.insert(std::make_pair(p_idx, std::make_pair(sensorID, pos)));
      ++p_idx;

      storeHoughCluster.appendNew(SVDHoughCluster(sensorID, pos));
    }
  }
}

/*
 * To bypass the FPGA cluster algorithm, we can directly convert
 * TrueHits into the cluster format.
 */
void
SVDHoughtrackingModule::convertTrueHits()
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);
  const StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  int nTrueHit;
  VxdID sensorID;
  SVDTrueHit* svdTrueHit;
  TVector3 pos, local_pos;
  TVector3 vec, abs_pos;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  TVector3 clusterDelta(0.15, 0.15, 0.15);

  if (!storeHoughCluster.isValid()) {
    storeHoughCluster.create();
  } else {
    storeHoughCluster.getPtr()->Clear();
  }

  nTrueHit = storeSVDTrueHits.getEntries();
  if (nTrueHit == 0) {
    return;
  }

  B2DEBUG(250, "Convert TrueHits to Clusters");

  /* First convert to absolute hits and save into a map */
  /* Clear global cluser maps */
  svd_sensor_cluster_map.clear();
  for (int i = 0; i < nTrueHit; ++i) {
    svdTrueHit = storeSVDTrueHits[i];
    sensorID = svdTrueHit->getSensorID();
    local_pos.SetX(svdTrueHit->getU());
    local_pos.SetY(svdTrueHit->getV());
    local_pos.SetZ(svdTrueHit->getW());

    /* Convert local to global position */
    const SensorInfo* sensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));
    pos = sensorInfo->pointToGlobal(local_pos);

    B2DEBUG(250, "  Local Pos: " << local_pos.X() << " " << local_pos.Y() << " " << local_pos.Z()
            << " Pos: " << pos.X() << " " << pos.Y() << " " << pos.Z() << " Sensor ID: " << sensorID);

    n_clusters.insert(std::make_pair(n_idx, std::make_pair(sensorID, pos)));
    ++n_idx;
    p_clusters.insert(std::make_pair(p_idx, std::make_pair(sensorID, pos)));
    ++p_idx;

    storeHoughCluster.appendNew(SVDHoughCluster(sensorID, pos));
  }
}

/*
 * Calculate cluster position with simple center of gravity
 * algorithm.
 */
double
SVDHoughtrackingModule::clusterPosition(double pitch, short seed_strip, int size, double offset)
{
  float hit_cor;
  float max_diff, diff;
  int max_pos, max_sample;

  max_diff = 50.0f;
  diff = 1e+11;

  hit_cor = ((double)(size - 1)) * (pitch / 2.0);

  max_sample = 0;
  max_pos = 0;
  if (size > 1) {
    /* Find peak and peak position within the peak samples */
    for (int i = 0; i < size; ++i) {
      if (peak_samples[i] > max_sample) {
        max_sample = peak_samples[i];
        max_pos = i;
      }
    }
    if (size > 2 && max_pos > 0 && max_pos < size) {
      if (peak_samples[max_pos - 1] > peak_samples[max_pos + 1]) {
        diff = (max_sample - peak_samples[max_pos - 1]);
      } else {
        diff = (peak_samples[max_pos + 1] - max_sample);
      }
    } else {
      diff = (peak_samples[1] - peak_samples[0]);
    }
  }

  if (fabsf(diff) > max_diff) {
    if (diff < 0.0) {
      diff = 0.9f;
    } else {
      diff = -0.9f;
    }
  } else {
    diff /= max_diff;
  }

  hit_cor += diff * (pitch / 2.0);

  B2DEBUG(250, "          Diff: " << diff << " hit_cor: " << hit_cor);

  return (pitch * ((double) seed_strip) - hit_cor - offset);
}

/*
 * Do coordinate translation and add cluster to cluster
 * map.
 */
void
SVDHoughtrackingModule::clusterAdd(VxdID sensorID, bool n_side, short seed_strip, int size)
{
  Sensor sensor;
  double pitch, corr;
  TVector3 pos, local_pos, first_pos;
  const SensorInfo* sensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));

  local_pos.SetX(0.0);
  local_pos.SetY(sensorInfo->getUCellPosition(0));
  local_pos.SetZ(sensorInfo->getUCellPosition(0));

  first_pos = sensorInfo->pointToGlobal(local_pos);

  pos.SetX(first_pos.X());
  if (n_side) {
    pitch = sensorInfo->getUPitch();

    if (sensorID.getLayerNumber() == 5) {
      corr = 4.8 * Unit::mm;
    } else {
      corr = 0.0;
    }

    pos.SetY(0.0);
    pos.SetZ(clusterPosition(pitch, seed_strip, size, (sensorInfo->getWidth() / 2.0)) + corr);
    n_clusters.insert(std::make_pair(n_idx, std::make_pair(sensorID, pos)));
    ++n_idx;
  } else {
    pitch = sensorInfo->getVPitch();
    pos.SetZ(0.0);
    pos.SetY(clusterPosition(pitch, seed_strip, size, (sensorInfo->getLength() / 2.0)));
    p_clusters.insert(std::make_pair(p_idx, std::make_pair(sensorID, pos)));
    ++p_idx;
  }

  B2DEBUG(250, "  -> New Cluster size: " << size << " Sensor: " << sensorID << " Side: "
          << n_side << " Seed Strip: " << seed_strip << " Pitch: " << pitch << " Pos: "
          << pos.X() << " " << pos.Y() << " " << pos.Z() << " Sensor Info: "
          << sensorInfo->getLength() << " " << sensorInfo->getWidth());
  for (int i = 0; i < size;  ++i) {
    B2DEBUG(250, "       |- Peak sample " << i << ": " << peak_samples[i]);
  }
}

/*
 * Analyse the clusters and compare the extrapolated coordinates
 * with official Clusterizer.
 */
void
SVDHoughtrackingModule::analyseClusterStrips()
{
  TVector3 pos;
  svdClusterMapIter iter;
  svdClusterPair cluster_info;
  int nSVDClusters, sensor_cnt;
  int nTrueHit;
  double closest_dist, dist;
  VxdID sensorID, svd_sensorID;
  StoreArray<SVDCluster> storeClusters(m_storeSVDClusterName);
  const StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  SVDCluster* svdCluster = NULL;
  SVDTrueHit* svdTrueHit = NULL;

  nSVDClusters = storeClusters.getEntries();
  if (nSVDClusters == 0) {
    return;
  }
  nTrueHit = storeSVDTrueHits.getEntries();
  if (nTrueHit == 0) {
    return;
  }

  sensor_cnt = 0;
  closest_dist = 1.0e+100;

  B2DEBUG(2, "List of clusters");

  B2DEBUG(2, " N-side");
  for (iter = n_clusters.begin(); iter != n_clusters.end(); ++iter) {
    cluster_info = iter->second;
    pos = cluster_info.second;
    sensorID = cluster_info.first;
    B2DEBUG(2, "  Sensor ID: " << sensorID << " Position: "
            << pos.X() << " " << pos.Y() << " " << pos.Z())

    /* Compare SVD clusters with Hough clusters */
    sensor_cnt = 0;
    closest_dist = 1.0e+100;
    for (int i = 0; i < nSVDClusters; ++i) {
      svdCluster = storeClusters[i];
      if (sensorID == svdCluster->getSensorID() && svdCluster->isUCluster()) {
        dist = (svdCluster->getPosition() -  pos.Z());
        B2DEBUG(2, "    | Compare with Sensor ID: " << svdCluster->getSensorID() << " Size "
                << svdCluster->getSize() << " z-Position: " << svdCluster->getPosition()
                << " Distance to current cluster: " << dist);
        if (fabs(closest_dist) > fabs(dist)) {
          closest_dist = dist;
        }
        ++sensor_cnt;
      }
    }
    if (closest_dist < 0.08) {
      m_histClusterU->Fill(closest_dist);
    }

    /* Compare Hough clusters with True hits */
    sensor_cnt = 0;
    closest_dist = 1.0e+100;
    for (int i = 0; i < nTrueHit; ++i) {
      svdTrueHit = storeSVDTrueHits[i];
      if (sensorID == svdCluster->getSensorID()) {
        dist = (svdTrueHit->getU() -  pos.Z());
        B2DEBUG(2, "    | Compare with True hit: " << svdTrueHit->getSensorID()
                << " z-Position: " << svdTrueHit->getU()
                << " Distance to current cluster: " << dist);
        if (fabs(closest_dist) > fabs(dist)) {
          closest_dist = dist;
        }
        ++sensor_cnt;
      }
    }
    if (closest_dist < 0.08) {
      m_histTrueClusterU->Fill(closest_dist);
    }
  }

  B2DEBUG(2, " P-side");
  for (iter = p_clusters.begin(); iter != p_clusters.end(); ++iter) {
    cluster_info = iter->second;
    pos = cluster_info.second;
    sensorID = cluster_info.first;
    B2DEBUG(2, "  Sensor ID: " << cluster_info.first << " Position: "
            << pos.X() << " " << pos.Y() << " " << pos.Z())

    sensor_cnt = 0;
    closest_dist = 1.0e+100;
    for (int i = 0; i < nSVDClusters; ++i) {
      svdCluster = storeClusters[i];
      if (sensorID == svdCluster->getSensorID() && !svdCluster->isUCluster()) {
        dist = (svdCluster->getPosition() -  pos.Y());
        B2DEBUG(2, "    | Compare with Sensor ID: " << svdCluster->getSensorID() << " Size "
                << svdCluster->getSize() << " y-Position: " << svdCluster->getPosition()
                << " Distance to current cluster: " << dist);
        if (fabs(closest_dist) > fabs(dist)) {
          closest_dist = dist;
        }
        ++sensor_cnt;
      }
    }
    if (closest_dist < 0.08) {
      m_histClusterV->Fill(closest_dist);
    }

    /* Compare Hough clusters with True hits */
    sensor_cnt = 0;
    closest_dist = 1.0e+100;
    for (int i = 0; i < nTrueHit; ++i) {
      svdTrueHit = storeSVDTrueHits[i];
      if (sensorID == svdCluster->getSensorID()) {
        dist = (svdTrueHit->getV() -  pos.Y());
        B2DEBUG(2, "    | Compare with True hit: " << svdTrueHit->getSensorID()
                << " y-Position: " << svdTrueHit->getV()
                << " Distance to current cluster: " << dist);
        if (fabs(closest_dist) > fabs(dist)) {
          closest_dist = dist;
        }
        ++sensor_cnt;
      }
    }
    if (closest_dist < 0.08) {
      m_histTrueClusterV->Fill(closest_dist);
    }
  }
}

/*
 * Top module for the Strip Cluster engine.
 */
void
SVDHoughtrackingModule::printClusters(svdClusterMap& cluster, bool n_side)
{
  svdClusterPair clusterInfo;
  TVector3 pos;
  VxdID sensorID;

  if (n_side) {
    B2DEBUG(1, "Clusters N-Side: " << cluster.size());
  } else {
    B2DEBUG(1, "Clusters P-Side: " << cluster.size());
  }

  for (auto it = cluster.begin(); it != cluster.end(); ++it) {
    clusterInfo = it->second;
    pos = clusterInfo.second;
    sensorID = clusterInfo.first;
    B2DEBUG(1, "  " << pos.X() << " "  << pos.Y() << " " <<  pos.Z() << " in Sensor " << sensorID);
  }
}

/*
 * Analyse the clusters and compare the extrapolated coordinates
 * with official Clusterizer.
 */
void
SVDHoughtrackingModule::trackAnalyseMCParticle()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  int nMCParticles, nTracks;
  //int min_pos;
  static int run = 1;
  bool primeOnly = true;
  bool usePhi = true;
  bool useTheta = true;
  bool* track_match_n;
  bool* track_match_p;
  bool* track_match;
  double charge, phi, theta, r;
  double theta_tolerance = 5.0; /* tolerance in theta */
  double phi_tolerance = 1.5; /* tolerance in phi */
  double min, min_theta, min_phi, dist, pT;
  double nPrimary, nCorrReco, nCorrRecoN, nCorrRecoP;
  double m_r = 0.0;
  double frac = 360.0;
  double projected_theta;
  TVector3 mom, prim_mom;

  nMCParticles = storeMCParticles.getEntries();
  nTracks = storeHoughTrack.getEntries();

  /* Don't analyse high occ events */
  if (nTracks > 10000) {
    B2WARNING("High occupancy event. Skipping...");
    ++validEvents; /* High occ doesn't mean it's not a valid event */
    return;
  }

  /* In Tb mode we skip this event, when there are not enough hits created in the
   * sensor to create a track.
   */
  if (m_tbMapping) {
    if (n_clusters.size() < 3 || p_clusters.size() < 3) {
      return;
    }
  }

  /* Valid event */
  ++validEvents;

  /* Disable Theta reco */
  if (m_usePhiOnly) {
    useTheta = false;
  }

  /* Disable Theta reco */
  if (m_useThetaOnly) {
    usePhi = false;
  }

  track_match_n = new bool[nMCParticles]();
  track_match_p = new bool[nMCParticles]();
  track_match = new bool[nMCParticles]();
  nPrimary = 0;
  pT = 0.0;

  if (m_compareMCParticleVerbose) {
    B2INFO("MCParticle Comparison: ");
  } else {
    B2DEBUG(1, "MCParticle Comparison: ");
  }
  for (int i = 0; i < nMCParticles; ++i) {
    charge = storeMCParticles[i]->getCharge();
    mom = storeMCParticles[i]->getMomentum();
    TVector3 vertex = storeMCParticles[i]->getVertex();
    if (storeMCParticles[i]->getMother() == NULL || !primeOnly) {
      B2DEBUG(20000, "Vertex: " << vertex.X() << " " << vertex.Y() << " " << vertex.Z());
      prim_mom = mom;
      /* Get pT of particles */
      pT = mom.Perp();
      /* Write pT in pT histogram */
      m_histPtDist->Fill(pT);
      /* Write Phi and Theta into histogram */
      m_histPhiDist->Fill(mom.Phi() / Unit::deg);
      m_histThetaDist->Fill(mom.Theta() / Unit::deg);
      //projected_theta = atan(fabs(mom.Y()) / fabs(mom.Z())); /* Calculate the projected theta */
      if (m_tbMapping) {
        projected_theta = mom.Theta();
      } else {
        projected_theta = atan(mom.Y() / mom.Z()); /* Calculate the projected theta */
      }
      m_histProjectedThetaDist->Fill(projected_theta / Unit::deg);

      B2DEBUG(250, "  MCParticleInfo: Mom Vec: " << mom.X() << " " << mom.Y() << " " << mom.Z());
      if (m_compareMCParticleVerbose) {
        cout << "  ID: " << storeMCParticles[i]->getIndex() << " PDG ID: "
             << boost::format("+%+3f") % storeMCParticles[i]->getPDG()
             << " Phi: " << boost::format("%+3.6f") % mom.Phi() << " ("
             << boost::format("%+3.6f") % (mom.Phi() / Unit::deg) << ")"
             << " Theta: " << boost::format("%+3.6f") % mom.Theta() << " ("
             << boost::format("%+3.6f") % (mom.Theta() / Unit::deg) << ") "
             << "P: " << boost::format("%+2.3f") % mom.Mag()
             << " Pt: " << boost::format("%+2.3f") % mom.Perp();
      } else {
        B2DEBUG(1, "  ID: " << storeMCParticles[i]->getIndex() << " PDG ID: " << storeMCParticles[i]->getPDG()
                << " Phi: " << mom.Phi() << " (" << mom.Phi() / Unit::deg << ")"
                << " Theta: " << mom.Theta() << " (" << mom.Theta() / Unit::deg << ")");
      }
      ++nPrimary;
      min_theta = min_phi = 2.0 * M_PI;
      min = 9E+99;
      for (int j = 0; j < nTracks; ++j) {
        r = storeHoughTrack[j]->getTrackR();
        /* XXX: Hack for determing curling direction */
        if (charge < 0) {
          phi = storeHoughTrack[j]->getTrackPhi();
        } else {
          if (r < 0.0) {
            phi = storeHoughTrack[j]->getTrackPhi() - M_PI;
          } else {
            phi = storeHoughTrack[j]->getTrackPhi() + M_PI;
          }
        }
        theta = storeHoughTrack[j]->getTrackTheta();

        dist = fabs((phi - mom.Phi())) + fabs((theta - mom.Theta()));
        if (dist < min) {
          min = dist;
          //min_pos = j;
          //min_phi = (phi - mom.Phi()) / Unit::deg;
          //min_theta = (theta - mom.Theta()) / Unit::deg;
        }

        if (fabs((phi - mom.Phi())) < fabs(min_phi)) {
          min_phi = ((phi - mom.Phi()));
          m_r = r;
        }

        //if (fabs((theta - (mom.Theta() * sin(mom.Phi())))) < fabs(min_theta)) {
        if (fabs(theta - projected_theta) < fabs(min_theta)) {
          min_theta = theta - projected_theta;
          //if (fabs((storeHoughTrack[j]->getTrackTheta() - (mom.Theta()))) < fabs(min_theta)) {
          //  min_theta = ((storeHoughTrack[j]->getTrackTheta() - (mom.Theta())));
        }

#ifdef DBG_THETA
        ofstream os_angles;
        os_angles.open("dbg/angles.dat", ofstream::out | ofstream::app);
        os_angles << mom.Theta() / Unit::deg << "\t" << projected_theta / Unit::deg << "\t"
                  << theta / Unit::deg << "\t" << mom.Phi() / Unit::deg << "\t"
                  << phi / Unit::deg << endl;
        os_angles.close();
#endif


        B2DEBUG(150, "  MCP " << i << " Mom: " << mom.X() << ", " << mom.Y() << ", " << mom.Z()
                << " with Track (" << phi << ", "
                << theta << ") " << j << " Difference: phi = "
                << min_phi << " theta = " << min_theta << " projected theta: " << projected_theta << " ("
                << projected_theta / Unit::deg << ")");
      }
      min_theta /= Unit::deg;
      min_phi /= Unit::deg;

      /* Plot spread in histogram */
      if (nTracks != 0) {
        m_histHoughDiffPhi->Fill(min_phi);
        m_histHoughDiffTheta->Fill(min_theta);
      }

      /* Missing hit plots for MCParticle track in Phi and Theta */
      if (nTracks == 0) {
        m_histMissedTheta->Fill(mom.Theta() / Unit::deg);
        m_histMissedPhi->Fill(mom.Phi() / Unit::deg);
      }

      if (usePhi) {
        if (fabs(min_phi) < phi_tolerance || (fabs(min_phi) > (frac - phi_tolerance)
                                              && fabs(min_phi) != frac)) {
          if (m_r < 0.0) {
            if (m_r > minR) {
              minR = m_r;
            }
          } else {
            if (m_r < maxR) {
              maxR = m_r;
            }
          }
          if (m_compareMCParticleVerbose) {
            cout << "\t\033[1;32m" << " found  (" << min_phi << ")\033[0m";
          }
          /* Efficiency vs pT (only in phi while reconstruction in theta is not proved) */
          m_histPtPhiRecon->Fill(pT);
          m_histPhiRecon->Fill(mom.Phi() / Unit::deg); /* Efficiency vs Phi */

          track_match_p[i] = true;
        } else if (m_compareMCParticleVerbose) {
          cout << "\t\033[1;31m" << " failed (" << min_phi << ")\033[0m";
        }
        if (!useTheta && m_compareMCParticleVerbose) {
          cout << endl;
        }
      }

      if (useTheta) {
        if (fabs(min_theta) < theta_tolerance) {
          if (m_compareMCParticleVerbose) {
            cout << "\033[1;32m" << " found (" << min_theta << ")\033[0m" << endl;
          }
          /* Efficiency vs pT (only in phi while reconstruction in theta is not proved) */
          m_histPtThetaRecon->Fill(pT);
          m_histThetaRecon->Fill(mom.Theta() / Unit::deg); /* Efficiency vs Theta */
          m_histProjectedThetaRecon->Fill(projected_theta / Unit::deg); /* Efficiency vs Theta */
          m_histThetaPhiDist->Fill(mom.Phi() / Unit::deg); /* Efficiency of Theta vs Phi */

          track_match_n[i] = true;

          /* Found Theta, check also if Phi was correctly reconstructed */
          if (track_match_p[i]) {
            m_histPtRecon->Fill(pT);
            track_match[i] = true;
          }
        } else if (m_compareMCParticleVerbose) {
          cout << "\033[1;31m" << " failed (" << min_theta << ")\033[0m" << endl;
        }
      }
      B2DEBUG(150, "\n");

    }
  }
  if (m_compareMCParticleVerbose) {
    cout << "  Total MCParticles tracks: " << nMCParticles << " Reconstructed tracks: " << nTracks << endl;
  }

  nCorrRecoN = 0;
  nCorrRecoP = 0;
  nCorrReco = 0;
  for (int i = 0; i < nPrimary; ++i) {
    if (track_match[i] == true) {
      ++nCorrReco;
    }
    if (track_match_n[i] == true) {
      ++nCorrRecoN;
    }
    if (track_match_p[i] == true) {
      ++nCorrRecoP;
    }
  }

  /* Recalculate track efficency */
  if (nCorrReco > 0) {
    curTrackEff += ((((double) nCorrReco)) / (double)(nPrimary));
  }
  if (nCorrRecoN > 0) {
    curTrackEffN += ((((double) nCorrRecoN)) / (double)(nPrimary));
  }
  if (nCorrRecoP > 0) {
    curTrackEffP += ((((double) nCorrRecoP)) / (double)(nPrimary));
  }
  //totFakeTracks += (double) (nTracks - nCorrRecoP - nCorrRecoN);
  totTracks += (unsigned int) nTracks;
  if ((nTracks - nCorrRecoP) > 0) {
    totFakeTracks += (double)(nTracks - nCorrRecoP);
    m_histPtFake->Fill(pT, (nTracks - nCorrRecoP));
  }
  B2DEBUG(1, "   Fake tracks: " << totFakeTracks << " " << nTracks << " " << nCorrRecoP);

  /* For Fake study, only when we have one primary track */
  if (nPrimary == 1) {
    m_histPtFakePhi->Fill(prim_mom.Perp(), (double)(nTracks - nCorrRecoP));
    m_histFakePhi->Fill(prim_mom.Phi() / Unit::deg, (double)(nTracks - nCorrRecoP));
    m_histFakeTheta->Fill(prim_mom.Theta() / Unit::deg, (double)(nTracks - nCorrRecoP));
  }

  ++run;

  delete[] track_match_n;
  delete[] track_match_p;
  delete[] track_match;
}

void
SVDHoughtrackingModule::printCustomSummary()
{
  B2DEBUG(1, "--------- Custom Summary ---------");
  printClusters(n_clusters, true);
  printClusters(p_clusters, false);
  B2DEBUG(1, "--------- End Custom Summary ---------");
}

/*
 * Create ghosts.
 */
void
SVDHoughtrackingModule::createGhosts()
{
  vector<TVector3> ghost;
  /* Layer 3 */
  ghost.push_back(TVector3(-3.18393, -0.573994, 0.2504667));
  ghost.push_back(TVector3(-3.18393, -0.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -0.573994, 1.7185263));
  ghost.push_back(TVector3(-3.18393, -0.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -0.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, -0.573994, 1.8941479));
  ghost.push_back(TVector3(-3.18393, -0.573994, 3.5824531));
  ghost.push_back(TVector3(-3.18393, -0.573994, -3.8714322));

  ghost.push_back(TVector3(-3.18393, -1.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -1.573994, 1.7185263));
  ghost.push_back(TVector3(-3.18393, -1.573994, 0.2504667));
  ghost.push_back(TVector3(-3.18393, -1.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -1.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, -1.573994, 1.8941479));
  ghost.push_back(TVector3(-3.18393, -1.573994, 3.5824531));
  ghost.push_back(TVector3(-3.18393, -1.573994, -3.8714322));

  ghost.push_back(TVector3(-3.18393, -2.573994, 1.7185263));
  ghost.push_back(TVector3(-3.18393, -2.573994, 0.2504667));
  ghost.push_back(TVector3(-3.18393, -2.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -2.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, -2.573994, 1.8941479));
  ghost.push_back(TVector3(-3.18393, -2.573994, -3.8714322));
  ghost.push_back(TVector3(-3.18393, -2.573994, 3.5824531));
  ghost.push_back(TVector3(-3.18393, -2.573994, 1.7185263));

  ghost.push_back(TVector3(-3.18393, 2.573994, 1.2504667));
  ghost.push_back(TVector3(-3.18393, 2.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 2.573994, 1.8941479));
  ghost.push_back(TVector3(-3.18393, 2.573994, -3.8714322));
  ghost.push_back(TVector3(-3.18393, 2.573994, 3.5824531));
  ghost.push_back(TVector3(-3.18393, 2.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 2.573994, 1.7185263));
  ghost.push_back(TVector3(-3.18393, 2.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, 2.573994, 0.2504667));

  ghost.push_back(TVector3(-3.18393, 2.133992, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 2.133992, 1.8941479));
  ghost.push_back(TVector3(-3.18393, 2.133992, -3.8714322));
  ghost.push_back(TVector3(-3.18393, 2.133992, 3.5824531));
  ghost.push_back(TVector3(-3.18393, 2.133992, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 2.133992, 1.2504667));
  ghost.push_back(TVector3(-3.18393, 2.133992, 1.7185263));
  ghost.push_back(TVector3(-3.18393, 2.133992, 1.2177692));
  ghost.push_back(TVector3(-3.18393, 2.133992, 0.2504667));

  ghost.push_back(TVector3(-3.18393, 1.231563, 1.8941479));
  ghost.push_back(TVector3(-3.18393, 1.231563, -3.8714322));
  ghost.push_back(TVector3(-3.18393, 1.231563, 3.5824531));
  ghost.push_back(TVector3(-3.18393, 1.231563, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 1.231563, 1.2504667));
  ghost.push_back(TVector3(-3.18393, 1.231563, 1.7185263));
  ghost.push_back(TVector3(-3.18393, 1.231563, 1.2177692));
  ghost.push_back(TVector3(-3.18393, 1.231563, 0.2504667));

  ghost.push_back(TVector3(-3.18393, 0.782551, -3.8714322));
  ghost.push_back(TVector3(-3.18393, 0.782551, 3.5824531));
  ghost.push_back(TVector3(-3.18393, 0.782551, 1.8941479));
  ghost.push_back(TVector3(-3.18393, 0.782551, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 0.782551, 1.2504667));
  ghost.push_back(TVector3(-3.18393, 0.782551, 1.7185263));
  ghost.push_back(TVector3(-3.18393, 0.782551, 1.2177692));
  ghost.push_back(TVector3(-3.18393, 0.782551, 0.2504667));

  ghost.push_back(TVector3(-3.18393, -1.755843, 3.5824531));
  ghost.push_back(TVector3(-3.18393, -1.755843, -3.8714322));
  ghost.push_back(TVector3(-3.18393, -1.755843, 1.8941479));
  ghost.push_back(TVector3(-3.18393, -1.755843, 2.6434211));
  ghost.push_back(TVector3(-3.18393, -1.755843, 1.2504667));
  ghost.push_back(TVector3(-3.18393, -1.755843, 1.7185263));
  ghost.push_back(TVector3(-3.18393, -1.755843, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -1.755843, 0.2504667));

  /* Layer 4 */
  ghost.push_back(TVector3(1.01896, 1.755843, 5.2504667));
  ghost.push_back(TVector3(1.01896, 1.755843, -2.2938452));
  ghost.push_back(TVector3(1.01896, 1.755843, +2.2938452));
  ghost.push_back(TVector3(1.01896, 1.755843, -1.7820537));

  ghost.push_back(TVector3(1.01896, 0.125458, -2.2938452));
  ghost.push_back(TVector3(1.01896, 0.125458, 5.2504667));
  ghost.push_back(TVector3(1.01896, 0.125458, +2.2938452));
  ghost.push_back(TVector3(1.01896, 0.125458, -1.7820537));

  ghost.push_back(TVector3(1.01896, -1.150386, +2.2938452));
  ghost.push_back(TVector3(1.01896, -1.150386, 5.2504667));
  ghost.push_back(TVector3(1.01896, -1.150386, -2.2938452));
  ghost.push_back(TVector3(1.01896, -1.150386, -1.7820537));

  ghost.push_back(TVector3(1.01896, -2.016503, -1.7820537));
  ghost.push_back(TVector3(1.01896, -2.016503, 5.2504667));
  ghost.push_back(TVector3(1.01896, -2.016503, -2.2938452));
  ghost.push_back(TVector3(1.01896, -2.016503, +2.2938452));

  /* Layer 5 */
  ghost.push_back(TVector3(3.51593, 0.855843, 5.2504667));
  ghost.push_back(TVector3(3.51593, 0.855843, -3.1234510));
  ghost.push_back(TVector3(3.51593, 0.855843, -1.1432447));
  ghost.push_back(TVector3(3.51593, 0.855843, +0.1456811));

  ghost.push_back(TVector3(3.51593, -1.257368, -3.1234510));
  ghost.push_back(TVector3(3.51593, -1.257368, 5.2504667));
  ghost.push_back(TVector3(3.51593, -1.257368, -1.1432447));
  ghost.push_back(TVector3(3.51593, -1.257368, +0.1456811));

  ghost.push_back(TVector3(3.51593, +0.145622, -1.1432447));
  ghost.push_back(TVector3(3.51593, +0.145622, 5.2504667));
  ghost.push_back(TVector3(3.51593, +0.145622, -3.1234510));
  ghost.push_back(TVector3(3.51593, +0.145622, +0.1456811));

  ghost.push_back(TVector3(3.51593, +0.023453, +0.1456811));
  ghost.push_back(TVector3(3.51593, +0.023453, -1.1432447));
  ghost.push_back(TVector3(3.51593, +0.023453, -3.1234510));
  ghost.push_back(TVector3(3.51593, +0.023453, 5.2504667));

  /* Layer 6 */
  ghost.push_back(TVector3(6.51591, 2.417032, -3.1254667));
  ghost.push_back(TVector3(6.51591, 2.417032, -0.9834291));
  ghost.push_back(TVector3(6.51591, 2.417032, -2.6748223));
  ghost.push_back(TVector3(6.51591, 2.417032, -3.1254667));
  ghost.push_back(TVector3(6.51591, 2.417032, +1.1245973));

  ghost.push_back(TVector3(6.51591, -0.185326, -0.9834291));
  ghost.push_back(TVector3(6.51591, -0.185326, -2.6748223));
  ghost.push_back(TVector3(6.51591, -0.185326, -3.1254667));
  ghost.push_back(TVector3(6.51591, -0.185326, +1.1245973));

  ghost.push_back(TVector3(6.51591, +2.732843, -2.6748223));
  ghost.push_back(TVector3(6.51591, +2.732843, +1.1245973));
  ghost.push_back(TVector3(6.51591, +2.732843, -0.9834291));
  ghost.push_back(TVector3(6.51591, +2.732843, -3.1254667));

  ghost.push_back(TVector3(6.51591, -1.039503, +1.1245973));
  ghost.push_back(TVector3(6.51591, -1.039503, -2.6748223));
  ghost.push_back(TVector3(6.51591, -1.039503, -0.9834291));
  ghost.push_back(TVector3(6.51591, -1.039503, -3.1254667));

  for (unsigned int i = 0; i < ghost.size(); ++i) {
    //cluster_map.insert(std::make_pair((iter->first) + 1 + i, ghost[i]));
  }
}

/* vim: set tabstop=2:softtabstop=2:shiftwidth=2:noexpandtab */

