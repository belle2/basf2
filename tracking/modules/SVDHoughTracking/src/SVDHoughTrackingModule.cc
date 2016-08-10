/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//#include <tracking/modules/SVDHoughTrackingModule.h>
#include <tracking/modules/SVDHoughTracking/SVDHoughTrackingModule.h>
//#include <tracking/modules/svdHoughtracking/basf2_tracking.h>
#include <vxd/geometry/GeoCache.h>
//#include <vxd/geometry/GeoVXDPosition.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/modules/mcTrackCandClassifier/MCTrackCandClassifierModule.h>

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
#include <time.h>

#include <root/TCanvas.h>
#include <root/TGeoMatrix.h>
#include <root/TGraph.h>
#include <root/TH1D.h>
#include <root/TH1F.h>
#include <root/TMath.h>
#include <root/TRandom.h>
#include <root/TTree.h>

#include <mdst/dataobjects/TrackFitResult.h>

#include <genfit/AbsKalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/MeasurementFactory.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/RKTrackRep.h>
#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/TGeoMaterialInterface.h>
#include <TDatabasePDG.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>
#include <TGeoManager.h>
#include <geometry/GeometryManager.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

/* Options */
//#define INJECT_GHOSTS
//#define DBG_THETA


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDHoughTracking)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDHoughTrackingModule::SVDHoughTrackingModule() : Module(), curTrackEff(0.0), ntotFakeTracks(0.0), ntotTracks(0), allTracks(0),
  curROIEff(0.0), ntotROITrueHits(0), ntotROISimHits(0), ncurHitsInROIs(0), ntotPXDactivePixel(0), ntotPXDinsideROIPixel(0),
  m_rootFile(0), m_histROIy(0), m_histROIz(0), m_histSimHits(0)
//SVDHoughTrackingModule::SVDHoughTrackingModule() : Module(), curTrackEff(0.0), ntotFakeTracks(0.0), ntotTracks(0), allTracks(0), curROIEff(0.0), ntotROITrueHits(0), ntotROISimHits(0), ncurHitsInROIs(0), ntotPXDactivePixel(0), ntotPXDinsideROIPixel(0), m_histROIy(0), m_histROIz(0), m_histSimHits(0)
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
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName,
           "Name of the RecoTracks StoreArray.", string(""));

  // 2. VXD vis
  addParam("SVDPosName", m_svdPosName,
           "VXD Position file name", string(""));
  addParam("SVDStripName", m_svdStripName,
           "VXD Position coordinate file name", string(""));

  /** Standalone Analysis of Data */
  addParam("StandaloneAnalysis", m_StandAloneAnalysis,
           "Use Standalone analysis of data without tracking, extrapolation and ROI calc", bool(false));

  // 3. Options
  /* Clustering */
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

  /* Tracking */
  addParam("FullTrackingPipeline", m_fullTrackingPipeline,
           "Run full tracking pipelin?", bool(false));
  addParam("UseRadiusFilter", m_useRadiusFilter,
           "Use radius filter", bool(true));
  addParam("UsePurifier", m_usePurifier,
           "Bool variable to decide whether to use a purifying algorithm at all", bool(true));
  addParam("UseEmptyPurify", m_useEmptyPurify,
           "Use empty purifier to convert SVDHoughCands in SVDHoughTrackCands", bool(false));
  addParam("UseClusteringPurify", m_useClusteringPurify,
           "Chose the clustering purifier", bool(false));
  addParam("MinHoughClusters", m_minHoughClusters,
           "Minimum number of clusters in HS to create TC", int(0));
  addParam("MaxHoughClustersX", m_maxClusterSizeX,
           "Maximum number of clusters in HS to create TC in x", double(4.0));
  addParam("MaxHoughClustersY", m_maxClusterSizeY,
           "Maximum number of clusters in HS to create TC in y", double(4.0));
  addParam("UseTrackMerger", m_useTrackMerger,
           "Use the track merger", bool(false));
  addParam("UseTrackMergerP", m_useTrackMergerP,
           "Use the track merger for p-side", bool(true));
  addParam("UseTrackMergerN", m_useTrackMergerN,
           "Use the track merger for n-side", bool(false));
  addParam("MergeThreshold", m_mergeThreshold,
           "Merge threshold", (double)(0.01));
  addParam("MergeThresholdP", m_mergeThresholdP,
           "Merge threshold", (double)(0.01));
  addParam("MergeThresholdN", m_mergeThresholdN,
           "Merge threshold", (double)(0.2));
  addParam("AnalyticalIntersection", m_analyticalIntersection,
           "Calculate intersections in HS analytically after first performing a sector based "
           "intercept finding with fastInterceptFinder2d or slowInterceptFinder2d", bool(false));
  addParam("UsePhiOnly", m_usePhiOnly,
           "Use Phi/radius reconstruction only", bool(false));
  addParam("UseThetaOnly", m_useThetaOnly,
           "Use Theta reconstruction only", bool(false));
  addParam("CompareWithMCParticle", m_compareMCParticle,
           "Compare reconstructed tracks with MC Particles?", bool(false));
  addParam("CompareWithMCParticleVerbose", m_compareMCParticleVerbose,
           "Verbose output of the comparator function", bool(false));
  addParam("TbMapping", m_tbMapping,
           "Use testbeam mapping", bool(false));
  addParam("StraightTracks", m_straightTracks,
           "Reconstruct only straight tracks", bool(false));
  addParam("phiTolerance", m_phiTolerance,
           "Tolerance for phi reconstruction (in deg)", (double)(1.0));
  addParam("thetaTolerance", m_thetaTolerance,
           "Tolerance for theta reconstruction (in deg)", (double)(2.0));
  addParam("pTTolerance", m_pTTolerance,
           "Tolerance for pT reconstruction (in GeV)", (double)(0.05));
  addParam("radiusTolerance", m_radiusTolerance,
           "Tolerance for radius reconstruction (in m)", (double)(0.2));

  addParam("CountStrips", m_countStrips,
           "Count Strips starting with 0 (=true) or use strip numbers", bool(true));
  addParam("UseAllStripCombinations", m_useAllStripCombinations,
           "Use all combinations of strips in fac3d", bool(false));

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
  addParam("fixedSizeUCells", m_fixedSizeUCells,
           "Number of Pixels in U for fixed Size ROIs PXD", int(70));
  addParam("fixedSizeVCells", m_fixedSizeVCells,
           "Number of Pixels in V for fixed Size ROIs PXD", int(30));

  /* Verbose */
  addParam("PrintTrackInfo", m_printTrackInfo,
           "Use testbeam extrapolation", bool(false));
  addParam("PrintStatistics", m_printStatistics,
           "Use testbeam extrapolation", bool(false));

  /* Statistics and ROOT output */
  addParam("statisticsFilename", m_statisticsFileName,
           "File name for statistics generation", string(""));
  addParam("rootFilename", m_rootFilename,
           "File name for statistics generation", string(""));

  // 4. Noise Filter
  addParam("DisableNoiseFilter", m_disableNoiseFilter,
           "Disable noise filter for clusters?", bool(false));
  addParam("noiseFactor", m_noiseFactor,
           "Factor for next neighbour detection", (unsigned int)(4));
  addParam("noiseDownThreshold", m_noiseUpThreshold,
           "Noise filter down threshold", (unsigned int)(1));
  addParam("noiseUpThreshold", m_noiseDownThreshold,
           "Noise filter upper threshold", (unsigned int)(250));

  /* 5. Hough Trafo */
  addParam("ConformalTrafoP", m_conformalTrafoP,
           "Use Conformal trafo in P", bool(true));
  addParam("ConformalTrafoN", m_conformalTrafoN,
           "Use Conformal trafo in N", bool(false));
  addParam("ProjectionRecoN", m_projectionRecoN,
           "Use projection reconstruction for n-side", bool(false));
  addParam("XYHoughPside", m_xyHoughPside,
           "If using conformal transformation, use (x',y') = (x,y)/r^2 for transformation "
           "and thus d(alpha) = 2 * (x' cos(alpha) + y' sin(alpha)) =  2/r^2 * (x cos(alpha) + y sin(alpha))", bool(false));
  addParam("RphiHoughPside", m_rphiHoughPside,
           "If using conformal transformation, use (r, phi0) for transformation and thus d(alpha = phi) = 2/r sin(phi - phi0)", bool(true));
  addParam("minimumLines", m_minimumLines,
           "Minimum lines to be found in order to continue", (unsigned int)(3));  // 4
  addParam("criticalIterationsN", m_critIterationsN,
           "Critical iterations ones a cell becomes a track", (unsigned int)(5)); // 7
  addParam("criticalIterationsP", m_critIterationsP,
           "Critical iterations ones a cell becomes a track", (unsigned int)(7)); // 7
  addParam("maxIterationsN", m_maxIterationsN,
           "Critical iterations ones a cell becomes a track", (unsigned int)(7)); // 12
  addParam("maxIterationsP", m_maxIterationsP,
           "Critical iterations ones a cell becomes a track", (unsigned int)(9)); // 12
  addParam("independentSectors", m_independentSectors,
           "Independent number of sectors for x and y in HS (FPGA-like)", bool(false));
  addParam("angleSectorsP", m_angleSectorsP,
           "Number of sectors for HT in P", (unsigned int)(256));
  addParam("angleSectorsN", m_angleSectorsN,
           "Number of sectors for HT in P", (unsigned int)(256));
  addParam("vertSectorsP", m_vertSectorsP,
           "Number of sectors for HT in P", (unsigned int)(256));
  addParam("vertSectorsN", m_vertSectorsN,
           "Number of sectors for HT in P", (unsigned int)(256));
  addParam("RadiusThreshold", m_radiusThreshold,
           "Cut off radius threshold", (double)(5.0));
  addParam("rectSizeN", m_rectSizeN,
           "Size of rectangular in N", (double)(0.1));          // 1.0 => 2.0
  addParam("rectSizeP", m_rectSizeP,
           "Size of rectangular in P", (double)(0.1));         // 1.0 => 0.05 / 0.1
  addParam("rectXP1", m_rectXP1,
           "Size of rectangular in x P", (double)(M_PI / -2.0));
  addParam("rectXP2", m_rectXP2,
           "Size of rectangular in x P", (double)(M_PI / 2.0));
  addParam("rectXN1", m_rectXN1,
           "Size of rectangular in x N", (double)(M_PI / -2.)); // (double)(-150./180. * M_PI/2.0));             //(double)(M_PI / -2.0));
  addParam("rectXN2", m_rectXN2,
           "Size of rectangular in x N", (double)(M_PI / 2.)); // (double)(  17./180. * M_PI/2.0));             //(double)(M_PI / 2.0));
  addParam("WriteHoughSpace", m_writeHoughSpace,
           "Write Hough space into a gnuplot file?", bool(false));
  addParam("WriteHoughSectors", m_writeHoughSectors,
           "Write Secotrs into a gnuplot file?", bool(false));
  addParam("UseSensorFilter", m_useSensorFilter,
           "Use the Sensor layer filter", bool(true));


}

SVDHoughTrackingModule::~SVDHoughTrackingModule()
{
}

void
SVDHoughTrackingModule::initialize()
{
  int theta_bins = 180; /* Number of bins in theta */
  int phi_bins = 360; /* Number of bins in phi */
  int pt_bins = 600; /* Number of bins pT */
  TDirectory* clusterDir, *houghDir, *effDir, *fakeDir, *roiDir;
  StoreArray<SVDHoughCluster>::registerPersistent(m_storeHoughCluster, DataStore::c_Event, false);
  StoreArray<SVDHoughTrack>::registerPersistent(m_storeHoughTrack, DataStore::c_Event, false);
  StoreArray<SVDHoughCluster>::registerPersistent(m_storeExtrapolatedHitsName, DataStore::c_Event, false);

  StoreArray<MCParticle>::required();
  //StoreArray<Track>::required();
  //StoreArray<RecoTrack>::required(m_recoTracksStoreArrayName);
  //StoreArray<TrackFitResult>::required();

  /* Axis description */
  TString xAxisPt = "pT / GeV";
  TString xAxisPhi = "#varphi / deg";
  TString xAxisTheta = "#theta / deg";

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
    B2WARNING("Both UsePhi and UseThetaOnly set. Disable Theta..");
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

    /** Tree for the MCData */
    m_rootTreeMCData = new TTree("MCDataTree", "MC particle data tree");
    m_rootTreeMCData->Branch("EvtNo",     &m_treeMCDataEventNo,         "m_treeMCDataEventNo/I");
    m_rootTreeMCData->Branch("EvtPartNo", &m_treeMCDataEventParticleNo, "m_treeMCDataEventParticleNo/I");
    m_rootTreeMCData->Branch("PDG",       &m_treeMCDataPDG,             "m_treeMCDataPDG/I");
    m_rootTreeMCData->Branch("Charge",    &m_treeMCDataCharge,          "m_treeMCDataCharge/I");
    m_rootTreeMCData->Branch("Phi",       &m_treeMCDataPhi,             "m_treeMCDataPhi/D");
    m_rootTreeMCData->Branch("Theta",     &m_treeMCDataTheta,           "m_treeMCDataTheta/D");
    m_rootTreeMCData->Branch("Px",        &m_treeMCDataPx,              "m_treeMCDataPx/D");
    m_rootTreeMCData->Branch("Py",        &m_treeMCDataPy,              "m_treeMCDataPy/D");
    m_rootTreeMCData->Branch("Pz",        &m_treeMCDataPz,              "m_treeMCDataPz/D");
    m_rootTreeMCData->Branch("PT",        &m_treeMCDataPT,              "m_treeMCDataPT/D");
    m_rootTreeMCData->Branch("Vx",        &m_treeMCDataVx,              "m_treeMCDataVx/D");
    m_rootTreeMCData->Branch("Vy",        &m_treeMCDataVy,              "m_treeMCDataVy/D");
    m_rootTreeMCData->Branch("Vz",        &m_treeMCDataVz,              "m_treeMCDataVz/D");

    /** Tree for the results of the SVDHoughTracking */
    m_rootTreeHoughData = new TTree("HoughDataTree", "Hough particle data tree");
    m_rootTreeHoughData->Branch("EvtNo",      &m_treeHoughDataEventNo,      "m_treeHoughDataEventNo/I");
    m_rootTreeHoughData->Branch("EvtTrackNo", &m_treeHoughDataEventTrackNo, "m_treeHoughDataEventTrackNo/I");
    //m_rootTreeHoughData->Branch("PDG",        &m_treeHoughDataPDG,              "m_treeHoughDataPDG/I");    // unsinnig, da nicht bekannt
    m_rootTreeHoughData->Branch("Charge",     &m_treeHoughDataCharge,       "m_treeHoughDataCharge/I");
    m_rootTreeHoughData->Branch("CurvSign",   &m_treeHoughDataCurvSign,     "m_treeHoughDataCurvSign/I");
    m_rootTreeHoughData->Branch("Phi",        &m_treeHoughDataPhi,          "m_treeHoughDataPhi/D");
    m_rootTreeHoughData->Branch("Theta",      &m_treeHoughDataTheta,        "m_treeHoughDataTheta/D");
    m_rootTreeHoughData->Branch("Radius",     &m_treeHoughDataRadius,       "m_treeHoughDataRadius/D");

    /** Tree for the extrapolated Hits */
    m_rootTreeExtrapolatedHits = new TTree("ExtrapolatedHits", "Tree containing the data of the extrapolated hit (MPH)");
    m_rootTreeExtrapolatedHits->Branch("EvtNo", &m_treeExtHitEventNo, "m_treeExtHitEventNo/I");
    m_rootTreeExtrapolatedHits->Branch("HitNo", &m_treeExtHitHitNo,   "m_treeExtHitNo/I");
    m_rootTreeExtrapolatedHits->Branch("Layer", &m_treeExtHitLayerNo, "m_treeExtHitLayerNo/I");

    m_rootTreeExtrapolatedHits->Branch("Theta", &m_treeExtHitTheta, "m_treeExtHitTheta/D");
    m_rootTreeExtrapolatedHits->Branch("xExt", &m_treeExtHitX, "m_treeExtHitX/D");
    m_rootTreeExtrapolatedHits->Branch("yExt", &m_treeExtHitY, "m_treeExtHitY/D");
    m_rootTreeExtrapolatedHits->Branch("Ladder", &m_treeExtHitLadderNo, "m_treeExtHitLadderNo/I");
    m_rootTreeExtrapolatedHits->Branch("Sensor", &m_treeExtHitSensorNo, "m_treeExtHitSensorNo/I");
    m_rootTreeExtrapolatedHits->Branch("PosX",  &m_treeExtHitPosX,    "m_treeExtHitPosX/D");
    m_rootTreeExtrapolatedHits->Branch("PosY",  &m_treeExtHitPosY,    "m_treeExtHitPosY/D");
    m_rootTreeExtrapolatedHits->Branch("PosZ",  &m_treeExtHitPosZ,    "m_treeExtHitPosZ/D");
    m_rootTreeExtrapolatedHits->Branch("PosU",  &m_treeExtHitPosU,    "m_treeExtHitPosU/D");
    m_rootTreeExtrapolatedHits->Branch("PosV",  &m_treeExtHitPosV,    "m_treeExtHitPosV/D");
    m_rootTreeExtrapolatedHits->Branch("UCell", &m_treeExtHitUCell,   "m_treeExtHitUCell/I");
    m_rootTreeExtrapolatedHits->Branch("VCell", &m_treeExtHitVCell,   "m_treeExtHitVCell/I");

    m_rootTreeExtrapolatedHits->Branch("Theta_l1", &m_treeExtHitTheta_l1, "m_treeExtHitTheta_l1/D");
    m_rootTreeExtrapolatedHits->Branch("xExt_l1", &m_treeExtHitX_l1, "m_treeExtHitX_l1/D");
    m_rootTreeExtrapolatedHits->Branch("yExt_l1", &m_treeExtHitY_l1, "m_treeExtHitY_l1/D");
    m_rootTreeExtrapolatedHits->Branch("Ladder_l1", &m_treeExtHitLadderNo_l1, "m_treeExtHitLadderNo_l1/I");
    m_rootTreeExtrapolatedHits->Branch("Sensor_l1", &m_treeExtHitSensorNo_l1, "m_treeExtHitSensorNo_l1/I");
    m_rootTreeExtrapolatedHits->Branch("PosX_l1",  &m_treeExtHitPosX_l1,    "m_treeExtHitPosX_l1/D");
    m_rootTreeExtrapolatedHits->Branch("PosY_l1",  &m_treeExtHitPosY_l1,    "m_treeExtHitPosY_l1/D");
    m_rootTreeExtrapolatedHits->Branch("PosZ_l1",  &m_treeExtHitPosZ_l1,    "m_treeExtHitPosZ_l1/D");
    m_rootTreeExtrapolatedHits->Branch("PosU_l1",  &m_treeExtHitPosU_l1,    "m_treeExtHitPosU_l1/D");
    m_rootTreeExtrapolatedHits->Branch("PosV_l1",  &m_treeExtHitPosV_l1,    "m_treeExtHitPosV_l1/D");
    m_rootTreeExtrapolatedHits->Branch("UCell_l1", &m_treeExtHitUCell_l1,   "m_treeExtHitUCell_l1/I");
    m_rootTreeExtrapolatedHits->Branch("VCell_l1", &m_treeExtHitVCell_l1,   "m_treeExtHitVCell_l1/I");

    m_rootTreeExtrapolatedHits->Branch("Theta_l2", &m_treeExtHitTheta_l2, "m_treeExtHitTheta_l2/D");
    m_rootTreeExtrapolatedHits->Branch("xExt_l2", &m_treeExtHitX_l2, "m_treeExtHitX_l2/D");
    m_rootTreeExtrapolatedHits->Branch("yExt_l2", &m_treeExtHitY_l2, "m_treeExtHitY_l2/D");
    m_rootTreeExtrapolatedHits->Branch("Ladder_l2", &m_treeExtHitLadderNo_l2, "m_treeExtHitLadderNo_l2/I");
    m_rootTreeExtrapolatedHits->Branch("Sensor_l2", &m_treeExtHitSensorNo_l2, "m_treeExtHitSensorNo_l2/I");
    m_rootTreeExtrapolatedHits->Branch("PosX_l2",  &m_treeExtHitPosX_l2,    "m_treeExtHitPosX_l2/D");
    m_rootTreeExtrapolatedHits->Branch("PosY_l2",  &m_treeExtHitPosY_l2,    "m_treeExtHitPosY_l2/D");
    m_rootTreeExtrapolatedHits->Branch("PosZ_l2",  &m_treeExtHitPosZ_l2,    "m_treeExtHitPosZ_l2/D");
    m_rootTreeExtrapolatedHits->Branch("PosU_l2",  &m_treeExtHitPosU_l2,    "m_treeExtHitPosU_l2/D");
    m_rootTreeExtrapolatedHits->Branch("PosV_l2",  &m_treeExtHitPosV_l2,    "m_treeExtHitPosV_l2/D");
    m_rootTreeExtrapolatedHits->Branch("UCell_l2", &m_treeExtHitUCell_l2,   "m_treeExtHitUCell_l2/I");
    m_rootTreeExtrapolatedHits->Branch("VCell_l2", &m_treeExtHitVCell_l2,   "m_treeExtHitVCell_l2/I");


    /** Tree for ROI */
    m_rootTreeROI = new TTree("SVDHoughROITree", "ROI from SVDHoughTracking Algorithm");
    m_rootTreeROI->Branch("EvNo", &m_treeEventEventNo, "m_treeEventEventNo/I");
    /** Event ROI efficency */
    m_rootTreeROI->Branch("EventROIEfficiency", &m_treeEventROIEfficiency, "m_treeEventROIEfficiency/D");
    /** Event DRF */
    m_rootTreeROI->Branch("EventDRF", &m_treeEventDRF, "m_treeEventDRF/D");


    m_rootTreeEfficiency = new TTree("SVDHoughTrackingEfficiencyTree", "Tree containing the SVDHoughTracking Efficiency");
    m_rootTreeEfficiency->Branch("EvNo", &m_treeEventEventNo, "m_treeEventEventNo/I");
    /** Event efficiency */
    m_rootTreeEfficiency->Branch("EventEfficiency", &m_treeEventEfficiency, "m_treeEventEfficiency/D");
    /** Event fake rate */
    m_rootTreeEfficiency->Branch("EventFakeRate", &m_treeEventFakeRate, "m_treeEventFakeRate/D");


    /** Tree for SVDHoughTracking Data and Analysis */
    m_rootTreeTracking = new TTree("SVDHoughTree", "SVD Hough Tracking data");
    m_rootTreeTracking->Branch("EvNo", &m_treeTrackingEventNo, "m_treeTrackingEventNo/I");
    //m_rootTreeTracking->Branch("ClusterU", &m_treeClusterU, "m_treeClusterU/D");
    //m_rootTreeTracking->Branch("ClusterV", &m_treeClusterV, "m_treeClusterV/D");
    //m_rootTreeTracking->Branch("TrueClusterU", &m_treeTrueClusterU, "m_treeTrueClusterU/D");
    //m_rootTreeTracking->Branch("TrueClusterV", &m_treeTrueClusterV, "m_treeTrueClusterV/D");
    m_rootTreeTracking->Branch("HoughDiffPhi", &m_treeHoughDiffPhi, "m_treeHoughDiffPhi/D");
    m_rootTreeTracking->Branch("HoughDiffTheta", &m_treeHoughDiffTheta, "m_treeHoughDiffTheta/D");

    m_rootTreeTracking->Branch("MCPDG_all", &m_treeMCPDG_all, "m_treeMCPDG_all/I");
    m_rootTreeTracking->Branch("MCVertexX_all", &m_treeMCVertexX_all, "m_treeMCVertexX_all/D");
    m_rootTreeTracking->Branch("MCVertexY_all", &m_treeMCVertexY_all, "m_treeMCVertexY_all/D");
    m_rootTreeTracking->Branch("MCVertexZ_all", &m_treeMCVertexZ_all, "m_treeMCVertexZ_all/D");
    m_rootTreeTracking->Branch("MCMomentumX_all", &m_treeMCMomentumX_all, "m_treeMCMomentumX_all/D");
    m_rootTreeTracking->Branch("MCMomentumY_all", &m_treeMCMomentumY_all, "m_treeMCMomentumY_all/D");
    m_rootTreeTracking->Branch("MCMomentumZ_all", &m_treeMCMomentumZ_all, "m_treeMCMomentumZ_all/D");

    m_rootTreeTracking->Branch("MCPDG", &m_treeMCPDG, "m_treeMCPDG/I");
    m_rootTreeTracking->Branch("MCVertexX", &m_treeMCVertexX, "m_treeMCVertexX/D");
    m_rootTreeTracking->Branch("MCVertexY", &m_treeMCVertexY, "m_treeMCVertexY/D");
    m_rootTreeTracking->Branch("MCVertexZ", &m_treeMCVertexZ, "m_treeMCVertexZ/D");
    m_rootTreeTracking->Branch("MCMomentumX", &m_treeMCMomentumX, "m_treeMCMomentumX/D");
    m_rootTreeTracking->Branch("MCMomentumY", &m_treeMCMomentumY, "m_treeMCMomentumY/D");
    m_rootTreeTracking->Branch("MCMomentumZ", &m_treeMCMomentumZ, "m_treeMCMomentumZ/D");

    m_rootTreeTracking->Branch("MCTrackRadius", &m_treeMCTrackR, "m_treeMCTrackR/D");
    m_rootTreeTracking->Branch("HoughTrackRadius", &m_treeHoughTrackR, "m_treeHoughTrackR/D");
    m_rootTreeTracking->Branch("HoughTrackRCorrectRecon", &m_treeHoughTrackRCorrectRecon, "m_treeHoughTrackRCorrectRecon/D");
    m_rootTreeTracking->Branch("CorrectReconHoughTrackR", &m_treeCorrectReconHoughTrackR, "m_treeCorrectReconHoughTrackR/D");
    m_rootTreeTracking->Branch("CorrectReconMCTrackR", &m_treeCorrectReconMCTrackR, "m_treeCorrectReconMCTrackR/D");
    m_rootTreeTracking->Branch("CorrectReconTrackR", &m_treeCorrectReconTrackR, "m_treeCorrectReconTrackR/D");


    m_rootTreeTracking->Branch("MCCurvatureSign", &m_treeMCCurvatureSign, "m_treeMCCurvatureSign/I");
    m_rootTreeTracking->Branch("HoughCurvatureSign", &m_treeHoughCurvatureSign, "m_treeHoughCurvatureSign/I");
    m_rootTreeTracking->Branch("CorrectReconMCCurvatureSign", &m_treeCorrectReconMCCurvatureSign,
                               "m_treeCorrectReconMCCurvatureSign/I");
    m_rootTreeTracking->Branch("CorrectReconHoughCurvatureSign", &m_treeCorrectReconHoughCurvatureSign,
                               "m_treeCorrectReconHoughCurvatureSign/I");
    m_rootTreeTracking->Branch("CorrectReconCurvatureSign", &m_treeCorrectReconCurvatureSign, "m_treeCorrectReconCurvatureSign/I");

    m_rootTreeTracking->Branch("MCCharge", &m_treeMCCharge, "m_treeMCCharge/I");
    m_rootTreeTracking->Branch("HoughCharge", &m_treeHoughCharge, "m_treeHoughCharge/I");
    m_rootTreeTracking->Branch("CorrectReconMCCharge", &m_treeCorrectReconMCCharge, "m_treeCorrectReconMCCharge/I");
    m_rootTreeTracking->Branch("CorrectReconHoughCharge", &m_treeCorrectReconHoughCharge, "m_treeCorrectReconHoughCharge/I");
    m_rootTreeTracking->Branch("CorrectReconCharge", &m_treeCorrectReconCharge, "m_treeCorrectReconCharge/I");

    m_rootTreeTracking->Branch("MCPTDist", &m_treeMCPTDist, "m_treeMCPTDist/D");
    m_rootTreeTracking->Branch("HoughPTDist", &m_treeHoughPTDist, "m_treeHoughPTDist/D");
    m_rootTreeTracking->Branch("CorrectReconMCPTDist", &m_treeCorrectReconMCPTDist, "m_treeCorrectReconMCPTDist/D");
    m_rootTreeTracking->Branch("CorrectReconHoughPTDist", &m_treeCorrectReconHoughPTDist, "m_treeCorrectReconHoughPTDist/D");
    m_rootTreeTracking->Branch("CorrectReconPTDist", &m_treeCorrectReconPTDist, "m_treeCorrectReconPTDist/D");

    m_rootTreeTracking->Branch("MCPhiDist", &m_treeMCPhiDist, "m_treeMCPhiDist/D");
    m_rootTreeTracking->Branch("HoughPhiDist", &m_treeHoughPhiDist, "m_treeHoughPhiDist/D");
    m_rootTreeTracking->Branch("CorrectReconMCPhiDist", &m_treeCorrectReconMCPhiDist, "m_treeCorrectReconMCPhiDist/D");
    m_rootTreeTracking->Branch("CorrectReconHoughPhiDist", &m_treeCorrectReconHoughPhiDist, "m_treeCorrectReconHoughPhiDist/D");
    m_rootTreeTracking->Branch("CorrectReconPhiDist", &m_treeCorrectReconPhiDist, "m_treeCorrectReconPhiDist/D");
    m_rootTreeTracking->Branch("HoughThetavsMCPhiDist", &m_treeHoughThetavsMCPhiDist, "m_treeHoughThetavsMCPhiDist/D");
    m_rootTreeTracking->Branch("CorrectReconHoughThetavsMCPhiDist", &m_treeCorrectReconHoughThetavsMCPhiDist,
                               "m_treeCorrectReconHoughThetavsMCPhiDist/D");

    m_rootTreeTracking->Branch("MCThetaDist", &m_treeMCThetaDist, "m_treeMCThetaDist/D");
    m_rootTreeTracking->Branch("HoughThetaDist", &m_treeHoughThetaDist, "m_treeHoughThetaDist/D");
    m_rootTreeTracking->Branch("CorrectReconMCThetaDist", &m_treeCorrectReconMCThetaDist, "m_treeCorrectReconMCThetaDist/D");
    m_rootTreeTracking->Branch("CorrectReconHoughThetaDist", &m_treeCorrectReconHoughThetaDist, "m_treeCorrectReconHoughThetaDist/D");
    m_rootTreeTracking->Branch("CorrectReconThetaDist", &m_treeCorrectReconThetaDist, "m_treeCorrectReconThetaDist/D");
    m_rootTreeTracking->Branch("ProjectedThetaDist", &m_treeProjectedThetaDist, "m_treeProjectedThetaDist/D");

    m_rootTreeTracking->Branch("MissedPT", &m_treeMissedPT, "m_treeMissedPT/D");
    m_rootTreeTracking->Branch("MissedPhi", &m_treeMissedPhi, "m_treeMissedPhi/D");
    m_rootTreeTracking->Branch("MissedTheta", &m_treeMissedTheta, "m_treeMissedTheta/D");

    m_rootTreeTracking->Branch("TrackRRecon", &m_treeTrackRRecon, "m_treeTrackRRecon/D");
    m_rootTreeTracking->Branch("TrackRCorrectRecon", &m_treeTrackRCorrectRecon, "m_treeTrackRCorrectRecon/D");
    m_rootTreeTracking->Branch("PTRecon", &m_treePTRecon, "m_treePTRecon/D");
    m_rootTreeTracking->Branch("PTPhiRecon", &m_treePTPhiRecon, "m_treePTPhiRecon/D");
    m_rootTreeTracking->Branch("PTThetaRecon", &m_treePTThetaRecon, "m_treePTThetaRecon/D");
    m_rootTreeTracking->Branch("PhiRecon", &m_treePhiRecon, "m_treePhiRecon/D");
    m_rootTreeTracking->Branch("ThetaRecon", &m_treeThetaRecon, "m_treeThetaRecon/D");

    m_rootTreeTracking->Branch("Fakes", &m_treeFakes, "m_treeFakes/D");
    m_rootTreeTracking->Branch("PTFake", &m_treePTFake, "m_treePTFake/D");
    m_rootTreeTracking->Branch("PTFakePhi", &m_treePTFakePhi, "m_treePTFakePhi/D");
    m_rootTreeTracking->Branch("PTFakeTheta", &m_treePTFakeTheta, "m_treePTFakeTheta/D");
    m_rootTreeTracking->Branch("FakePhi", &m_treeFakePhi, "m_treeFakePhi/D");
    m_rootTreeTracking->Branch("FakeTheta", &m_treeFakeTheta, "m_treeFakeTheta/D");

    m_rootTreeTracking->Branch("AverageFakes", &m_treeAverageFakes, "m_treeAverageFakes/D");
    m_rootTreeTracking->Branch("AverageFakesPT", &m_treeAverageFakesPT, "m_treeAverageFakesPT/D");
    m_rootTreeTracking->Branch("AverageFakesPhi", &m_treeAverageFakesPhi, "m_treeAverageFakesPhi/D");
    m_rootTreeTracking->Branch("AverageFakesTheta", &m_treeAverageFakesTheta, "m_treeAverageFakesTheta/D");

    /** Tree for analysis of extrapolated hits */
    m_rootTreeExtHitAna = new TTree("SVDExtHitAnaTree", "SVD ExtHitAna Calculation data");
    m_rootTreeExtHitAna->Branch("ExtHitAnaEventNo", &m_treeExtHitAnaEventNo, "m_treeExtHitAnaEventNo/I");
    /** Total */
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPhi", &m_treeExtHitAnaDiffPhi, "m_treeExtHitAnaDiffPhi/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPhiPx", &m_treeExtHitAnaDiffPhiPx, "m_treeExtHitAnaDiffPhiPx/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffTheta", &m_treeExtHitAnaDiffTheta, "m_treeExtHitAnaDiffTheta/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffThetaPx", &m_treeExtHitAnaDiffThetaPx, "m_treeExtHitAnaDiffThetaPx/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaPTDist", &m_treeExtHitAnaPTDist, "m_treeExtHitAnaPTDist/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTPhi", &m_treeExtHitAnaDiffPTPhi, "m_treeExtHitAnaDiffPTPhi/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTPhiPx", &m_treeExtHitAnaDiffPTPhiPx, "m_treeExtHitAnaDiffPTPhiPx/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTTheta", &m_treeExtHitAnaDiffPTTheta, "m_treeExtHitAnaDiffPTTheta/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTThetaPx", &m_treeExtHitAnaDiffPTThetaPx, "m_treeExtHitAnaDiffPTThetaPx/D");
    /** PXD Layer 1 */
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPhi_pxdl1", &m_treeExtHitAnaDiffPhi_pxdl1, "m_treeExtHitAnaDiffPhi_pxdl1/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPhiPx_pxdl1", &m_treeExtHitAnaDiffPhiPx_pxdl1, "m_treeExtHitAnaDiffPhiPx_pxdl1/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffTheta_pxdl1", &m_treeExtHitAnaDiffTheta_pxdl1, "m_treeExtHitAnaDiffTheta_pxdl1/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffThetaPx_pxdl1", &m_treeExtHitAnaDiffThetaPx_pxdl1, "m_treeExtHitAnaDiffThetaPx_pxdl1/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaPTDist_pxdl1", &m_treeExtHitAnaPTDist_pxdl1, "m_treeExtHitAnaPTDist_pxdl1/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTPhi_pxdl1", &m_treeExtHitAnaDiffPTPhi_pxdl1, "m_treeExtHitAnaDiffPTPhi_pxdl1/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTPhiPx_pxdl1", &m_treeExtHitAnaDiffPTPhiPx_pxdl1, "m_treeExtHitAnaDiffPTPhiPx_pxdl1/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTTheta_pxdl1", &m_treeExtHitAnaDiffPTTheta_pxdl1, "m_treeExtHitAnaDiffPTTheta_pxdl1/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTThetaPx_pxdl1", &m_treeExtHitAnaDiffPTThetaPx_pxdl1,
                                "m_treeExtHitAnaDiffPTThetaPx_pxdl1/D");
    /** PXD Layer 2 */
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPhi_pxdl2", &m_treeExtHitAnaDiffPhi_pxdl2, "m_treeExtHitAnaDiffPhi_pxdl2/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPhiPx_pxdl2", &m_treeExtHitAnaDiffPhiPx_pxdl2, "m_treeExtHitAnaDiffPhiPx_pxdl2/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffTheta_pxdl2", &m_treeExtHitAnaDiffTheta_pxdl2, "m_treeExtHitAnaDiffTheta_pxdl2/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffThetaPx_pxdl2", &m_treeExtHitAnaDiffThetaPx_pxdl2, "m_treeExtHitAnaDiffThetaPx_pxdl2/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaPTDist_pxdl2", &m_treeExtHitAnaPTDist_pxdl2, "m_treeExtHitAnaPTDist_pxdl2/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTPhi_pxdl2", &m_treeExtHitAnaDiffPTPhi_pxdl2, "m_treeExtHitAnaDiffPTPhi_pxdl2/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTPhiPx_pxdl2", &m_treeExtHitAnaDiffPTPhiPx_pxdl2, "m_treeExtHitAnaDiffPTPhiPx_pxdl2/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTTheta_pxdl2", &m_treeExtHitAnaDiffPTTheta_pxdl2, "m_treeExtHitAnaDiffPTTheta_pxdl2/D");
    m_rootTreeExtHitAna->Branch("ExtHitAnaDiffPTThetaPx_pxdl2", &m_treeExtHitAnaDiffPTThetaPx_pxdl2,
                                "m_treeExtHitAnaDiffPTThetaPx_pxdl2/D");


    /* Clustering engine */
    if (m_analyseFPGAClusters) {
      clusterDir = m_rootFile->mkdir("Cluster");
      clusterDir->cd();
      m_histClusterU = new TH1D("DeltaU", "Cluster spread in U", 101, -0.0155, 0.0155);
      m_histClusterV = new TH1D("DeltaV", "Cluster spread in V", 101, -0.0155, 0.0155);
      m_histTrueClusterU = new TH1D("DeltaTrueU", "Spread in U between Clusters and TrueHits", 101, -0.1005, 0.1005);
      m_histTrueClusterV = new TH1D("DeltaTrueV", "Spread in V between Clusters and TrueHits", 101, -0.1005, 0.1005);
    }

    /* Hough debugging */
    if (m_compareMCParticle) {
      houghDir = m_rootFile->mkdir("Hough");
      houghDir->cd();
      //m_histHoughDiffPhi = new TH1D("DeltaPhi", "Spread in Phi", 1001, -5.005, 5.005);
      //m_histHoughDiffTheta = new TH1D("DeltaTheta", "Spread in Theta", 1001, -5.005, 5.005);
      m_histHoughDiffPhi = new TH1D("DeltaPhi", "Spread in Phi (Resolution in Phi)", 36100, -180.5, 180.5);
      m_histHoughDiffTheta = new TH1D("DeltaTheta", "Spread in Theta (Resolution in Theta)", 36100, -180.5, 180.5);
      //m_histHoughDiffPhivsPhi = new TH2D("DeltaPhivsPhi", "Spread in Phi", phi_bins+1, -180.5, 180.5, 501, -5.01, 5.01);
      //m_histHoughDiffThetavsTheta = new TH2D("DeltaThetavsTheta", "Spread in Theta", theta_bins+1, 16.5, 150.5, 501, -5.01, 5.01);
      m_histHoughDiffPhivsPhi = new TH2D("DeltaPhivsPhi", "Spread in Phi", phi_bins + 1, -180.5, 180.5, 18001, -180.01, 180.01);
      m_histHoughDiffThetavsTheta = new TH2D("DeltaThetavsTheta", "Spread in Theta", theta_bins + 1, -0.5, 180.5, 18001, -180.01, 180.01);
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

    /** Event efficiency */
    m_histEventEfficiency = new TH1D("EventEfficiency", "Overview of Reconstruction Efficiency", 101, 0.0, 1.01);
    m_histEventEfficiency->GetXaxis()->SetTitle("Reconstruction Efficiency");
    m_histEventEfficiency->GetYaxis()->SetTitle("Counts");
    /** Event fake rate */
    m_histEventFakeRate = new TH1D("EventFakeRate", "Overview of Fake Rate", 300, 0.0, 300);
    m_histEventFakeRate->GetXaxis()->SetTitle("Fake Rate");
    m_histEventFakeRate->GetYaxis()->SetTitle("Counts");

    /** Histogram for pT-distribution */
    m_histMCTrackR = new TH1D("MCTrackRadius", "Radius of MCParticle Track", 961, -12.0125, 12.0125);
    m_histMCTrackR->GetXaxis()->SetTitle("Track Radius / m");
    m_histMCTrackR->GetYaxis()->SetTitle("# tracks");
    m_histHoughTrackR = new TH1D("HoughTrackRadius", "Radius of Hough-Track", 961, -12.0125, 12.0125);
    m_histHoughTrackR->GetXaxis()->SetTitle("Track Radius / m");
    m_histHoughTrackR->GetYaxis()->SetTitle("# tracks");
    m_histHoughTrackRCorrectRecon = new TH1D("HoughTrackRadiusReconstructed", "Radius of Hough-Track (correct reconstructed radius)",
                                             961, -12.0125, 12.0125);
    m_histHoughTrackRCorrectRecon->GetXaxis()->SetTitle("Track Radius / m");
    m_histHoughTrackRCorrectRecon->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconHoughTrackR = new TH1D("CorrectReconHoughTrackRadius", "Radius of correctly reconstructed Hough-Track", 961,
                                             -12.0125, 12.0125);
    m_histCorrectReconHoughTrackR->GetXaxis()->SetTitle("Track Radius / m");
    m_histCorrectReconMCTrackR = new TH1D("CorrectReconMCTrackR", "Radius of correctly reconstructed Hough-Track", 961, -12.0125,
                                          12.0125);
    m_histCorrectReconMCTrackR->GetXaxis()->SetTitle("Track Radius / m");
    m_histCorrectReconMCTrackR->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconTrackR = new TH1D("CorrectReconTrackR", "Radius of correctly reconstructed Hough-Track", 961, -12.0125, 12.0125);
    m_histCorrectReconTrackR->GetXaxis()->SetTitle("Track Radius / m");
    m_histCorrectReconTrackR->GetYaxis()->SetTitle("# tracks");

    //m_histTrackRRecon = new TH1D("TrackRadiusecon", "Reconstructed Radius of Track", 961, -12.0125, 12.0125);
    //m_histTrackRRecon->GetXaxis()->SetTitle("Track Radius / m");

    m_histMCTrackREff = new TH1D("MCTrackRadiusEff", "Efficiency vs Radius of MCParticle Track", 961, -12.0125, 12.0125);
    m_histMCTrackREff->GetXaxis()->SetTitle("Track Radius / m");
    m_histHoughTrackREff = new TH1D("HoughTrackRadiusEff", "Efficiency vs reconstructed Radius of Track", 961, -12.0125, 12.0125);
    m_histHoughTrackREff->GetXaxis()->SetTitle("Track Radius / m");

    //m_histTrackRCorrectRecon = new TH1D("CorrReconTrackR", "Radius of correctly reconstructed Tracks", 961, -12.0125, 12.0125);
    //m_histTrackRCorrectRecon->GetXaxis()->SetTitle("Track Radius / m");
    m_histCorrectReconTrackREff = new TH1D("CorrectReconTrackREff", "Reconstruction Efficiency vs Radius of MCParticle Track", 961,
                                           -12.0125, 12.0125);
    m_histCorrectReconTrackREff->GetXaxis()->SetTitle("Track Radius / m");


    m_histMCCurvatureSign = new TH1D("MCCurvatureSign", "Sign of the Track-Curvature of the MCParticle", 3, -1.5, 1.5);
    m_histMCCurvatureSign->GetXaxis()->SetTitle("Sign of Track-Curvature");
    m_histMCCurvatureSign->GetYaxis()->SetTitle("# tracks");
    m_histHoughCurvatureSign = new TH1D("HoughCurvatureSign", "Sign of the Track-Curvature of the Hough-Particle", 3, -1.5, 1.5);
    m_histHoughCurvatureSign->GetXaxis()->SetTitle("Sign of Track-Curvature");
    m_histHoughCurvatureSign->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconMCCurvatureSign = new TH1D("CorrectReconMCCurvatureSign", "Sign of the Track-Curvature of the MCParticle", 3,
                                                 -1.5, 1.5);
    m_histCorrectReconMCCurvatureSign->GetXaxis()->SetTitle("Sign of Track-Curvature");
    m_histCorrectReconMCCurvatureSign->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconHoughCurvatureSign = new TH1D("CorrectReconHoughCurvatureSign",
                                                    "Sign of the Track-Curvature of the Hough-Particle", 3, -1.5, 1.5);
    m_histCorrectReconHoughCurvatureSign->GetXaxis()->SetTitle("Sign of Track-Curvature");
    m_histCorrectReconHoughCurvatureSign->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconCurvatureSign = new TH1D("CorrectReconCurvatureSign", "Sign of the Track-Curvature of the Hough-Particle", 3,
                                               -1.5, 1.5);
    m_histCorrectReconCurvatureSign->GetXaxis()->SetTitle("Sign of Track-Curvature");
    m_histCorrectReconCurvatureSign->GetYaxis()->SetTitle("# tracks");
    m_histCurvatureSignEff = new TH1D("TrackCurvatureSignEff", "Efficiency vs Sign of Track-Curvature", 3, -1.5, 1.5);
    m_histCurvatureSignEff->GetXaxis()->SetTitle("Sign of Track-Curvature");
    m_histCurvatureSignEff->GetYaxis()->SetTitle("#epsilon");


    m_histMCCharge = new TH1D("MCCharge", "Charge of MCParticle", 3, -1.5, 1.5);
    m_histMCCharge->GetXaxis()->SetTitle("Charge / e");
    m_histMCCharge->GetYaxis()->SetTitle("# tracks");
    m_histHoughCharge = new TH1D("HoughCharge", "Charge estimated by Hough-algorithm", 3, -1.5, 1.5);
    m_histHoughCharge->GetXaxis()->SetTitle("Charge / e");
    m_histHoughCharge->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconMCCharge = new TH1D("CorrectReconMCCharge", "Charge of MCParticle", 3, -1.5, 1.5);
    m_histCorrectReconMCCharge->GetXaxis()->SetTitle("Charge / e");
    m_histCorrectReconMCCharge->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconHoughCharge = new TH1D("CorrectReconHoughCharge", "Charge estimated by Hough-algorithm", 3, -1.5, 1.5);
    m_histCorrectReconHoughCharge->GetXaxis()->SetTitle("Charge / e");
    m_histCorrectReconHoughCharge->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconCharge = new TH1D("CorrectReconCharge", "Charge estimated by Hough-algorithm", 3, -1.5, 1.5);
    m_histCorrectReconCharge->GetXaxis()->SetTitle("Charge / e");
    m_histCorrectReconCharge->GetYaxis()->SetTitle("# tracks");
    m_histChargeEff = new TH1D("ChargeEff", "Efficiency of Charge Reconstruction", 3, -1.5, 1.5);
    m_histChargeEff->GetXaxis()->SetTitle("Charge / e");
    m_histChargeEff->GetYaxis()->SetTitle("#epsilon");


    /* Histogram for correctly reconstructed tracks in pT */
    m_histMCPTDist = new TH1D("MCPTDist", "pT-Distribution of MCParticle", pt_bins, 0.0, 3.0);
    m_histMCPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histMCPTDist->GetYaxis()->SetTitle("# tracks");
    m_histHoughPTDist = new TH1D("HoughPTDist", "pT-Distribution of Hough-Tracks", pt_bins, 0.0, 3.0);
    m_histHoughPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histHoughPTDist->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconMCPTDist = new TH1D("CorrectReconMCPTDist", "pT-Distribution of correctly reconstructed Hough-Tracks", pt_bins,
                                          0.0, 3.0);
    m_histCorrectReconMCPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histCorrectReconMCPTDist->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconHoughPTDist = new TH1D("CorrectReconHoughPTDist", "pT-Distribution of correctly reconstructed Hough-Tracks",
                                             pt_bins, 0.0, 3.0);
    m_histCorrectReconHoughPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histCorrectReconHoughPTDist->GetYaxis()->SetTitle("# tracks");
    m_histCorrectReconPTDist = new TH1D("CorrectReconPTDist", "pT-Distribution of correctly reconstructed Hough-Tracks", pt_bins, 0.0,
                                        3.0);
    m_histCorrectReconPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histCorrectReconPTDist->GetYaxis()->SetTitle("# tracks");
    m_histEffHoughPTvsMCPT = new TH1D("EffHoughPTvsMCPT", "pT-Distribution", pt_bins, 0.00, 3.0);
    m_histEffHoughPTvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histEffHoughPTvsMCPT->GetYaxis()->SetTitle("#epsilon");


    m_histHoughPhivsMCPTDist = new TH1D("HoughPhivsMCPTDist", "pT of reconstructed tracks in Phi", pt_bins, 0.0, 3.0);
    m_histHoughPhivsMCPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histHoughPhivsMCPTDist->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histHoughThetavsMCPTDist = new TH1D("HoughThetavsMCPTDist", "pT of reconstructed tracks in Theta", pt_bins, 0.0, 3.0);
    m_histHoughThetavsMCPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histHoughThetavsMCPTDist->GetYaxis()->SetTitle("# reconstructed tracks");

//    m_histCorrectReconHoughvsMCPTDist = new TH1D("CorrectReconHoughvsMCPTDist", "pT of correctly reconstructed tracks", pt_bins, 0.02, 3.0);
//    m_histCorrectReconHoughvsMCPTDist->GetXaxis()->SetTitle(xAxisPt);
//    m_histCorrectReconHoughvsMCPTDist->GetYaxis()->SetTitle("# reconstructed tracks");
//    m_histCorrectReconHoughPhivsMCPTDist = new TH1D("CorrectReconHoughPhivsMCPTDist", "pT of correctly reconstructed tracks in Phi", pt_bins, 0.02, 3.0);
//    m_histCorrectReconHoughPhivsMCPTDist->GetXaxis()->SetTitle(xAxisPt);
//    m_histCorrectReconHoughPhivsMCPTDist->GetYaxis()->SetTitle("# reconstructed tracks");
//    m_histCorrectReconHoughThetavsMCPTDist = new TH1D("CorrectReconHoughThetavsMCPTDist", "pT of correctly reconstructed tracks in Theta", pt_bins, 0.02, 3.0);
//    m_histCorrectReconHoughThetavsMCPTDist->GetXaxis()->SetTitle(xAxisPt);
//    m_histCorrectReconHoughThetavsMCPTDist->GetYaxis()->SetTitle("# reconstructed tracks");

    m_histCorrectReconvsMCPTDist = new TH1D("CorrectReconvsMCPTDist", "pT of correctly reconstructed tracks", pt_bins, 0.0, 3.0);
    m_histCorrectReconvsMCPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histCorrectReconvsMCPTDist->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histCorrectReconPhivsMCPTDist = new TH1D("CorrectReconPhivsMCPTDist", "pT of correctly reconstructed tracks in Phi", pt_bins,
                                               0.0, 3.0);
    m_histCorrectReconPhivsMCPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histCorrectReconPhivsMCPTDist->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histCorrectReconThetavsMCPTDist = new TH1D("CorrectReconThetavsMCPTDist", "pT of correctly reconstructed tracks in Theta",
                                                 pt_bins, 0.0, 3.0);
    m_histCorrectReconThetavsMCPTDist->GetXaxis()->SetTitle(xAxisPt);
    m_histCorrectReconThetavsMCPTDist->GetYaxis()->SetTitle("# reconstructed tracks");

    /* Histogram for efficiency vs pT (only in phi while reconstruction in theta is not proved) */
    m_histPTEffvsMCPT = new TH1D("PTEffvsMCPT", "Efficiency vs pT", pt_bins, 0.0, 3.0);
    m_histPTEffvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histPTEffvsMCPT->GetYaxis()->SetTitle("#epsilon");
//    m_histHoughPhiEffvsMCPT = new TH1D("HoughPhiEffvsMCPT", "Efficiency vs pT (in Phi)", pt_bins, 0.02, 3.0);
//    m_histHoughPhiEffvsMCPT->GetXaxis()->SetTitle(xAxisPt);
//    m_histHoughPhiEffvsMCPT->GetYaxis()->SetTitle("#epsilon");
//    m_histHoughThetaEffvsMCPT = new TH1D("m_histHoughThetaEffvsMCPT", "Efficiency vs pT (in Theta)", pt_bins, 0.02, 3.0);
//    m_histHoughThetaEffvsMCPT->GetXaxis()->SetTitle(xAxisPt);
//    m_histHoughThetaEffvsMCPT->GetYaxis()->SetTitle("#epsilon");

    m_histPhiEffvsMCPT = new TH1D("PhiEffvsMCPT", "Efficiency vs pT (in Phi)", pt_bins, 0.0, 3.0);
    m_histPhiEffvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histPhiEffvsMCPT->GetYaxis()->SetTitle("#epsilon");
    m_histThetaEffvsMCPT = new TH1D("ThetaEffvsMCPT", "Efficiency vs pT (in Theta)", pt_bins, 0.0, 3.0);
    m_histThetaEffvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histThetaEffvsMCPT->GetYaxis()->SetTitle("#epsilon");


    /* Histogram for Phi-distribution */
    m_histMCPhiDist = new TH1D("MCPhiDist", "MCPhi-Distribution", phi_bins + 1, -180.5, 180.5);
    m_histMCPhiDist->GetXaxis()->SetTitle(xAxisPhi);
    m_histMCPhiDist->GetYaxis()->SetTitle("# tracks");
    /* Histogram for correctly reconstructed tracks in Phi */
    m_histHoughPhiDist = new TH1D("HoughPhiDist", "Phi of reconstructed tracks", phi_bins + 1, -180.5, 180.5);
    m_histHoughPhiDist->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughPhiDist->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histCorrectReconMCPhiDist = new TH1D("CorrectReconMCPhiDist", "Phi of correctly reconstructed tracks", phi_bins + 1, -180.5,
                                           180.5);
    m_histCorrectReconMCPhiDist->GetXaxis()->SetTitle(xAxisPhi);
    m_histCorrectReconMCPhiDist->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histCorrectReconHoughPhiDist = new TH1D("CorrectReconHoughPhiDist", "Phi of correctly reconstructed tracks", phi_bins + 1, -180.5,
                                              180.5);
    m_histCorrectReconHoughPhiDist->GetXaxis()->SetTitle(xAxisPhi);
    m_histCorrectReconHoughPhiDist->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histCorrectReconPhiDist = new TH1D("CorrectReconPhiDist", "Phi of correctly reconstructed tracks", phi_bins + 1, -180.5, 180.5);
    m_histCorrectReconPhiDist->GetXaxis()->SetTitle(xAxisPhi);
    m_histCorrectReconPhiDist->GetYaxis()->SetTitle("# reconstructed tracks");
    /* Histogram for efficiency vs phi */
    //m_histEffHoughPhivsMCPhi = new TH1D("EffHoughPhivsMCPhi", "Efficiency vs Phi", phi_bins+1, -180.5, 180.5);
    m_histEffHoughPhivsMCPhi = new TH1D("EffHoughPhivsMCPhi", "Efficiency of Reconstruction of Phi", phi_bins + 1, -180.5, 180.5);
    m_histEffHoughPhivsMCPhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histEffHoughPhivsMCPhi->GetYaxis()->SetTitle("#epsilon");



    /* Histogram for Theta-distribution */
    //m_histMCThetaDist = new TH1D("MCThetaDist", "MCTheta-Distribution", theta_bins+1, 16.5, 150.5);
    m_histMCThetaDist = new TH1D("MCThetaDist", "MCTheta-Distribution", theta_bins + 1, -0.5, 180.5);
    m_histMCThetaDist->GetXaxis()->SetTitle(xAxisTheta);
    m_histMCThetaDist->GetYaxis()->SetTitle("# tracks");
    //m_histHoughThetaDist = new TH1D("HoughThetaDist", "Theta of reconstructed", theta_bins+1, 16.5, 150.5);
    m_histHoughThetaDist = new TH1D("HoughThetaDist", "Theta of reconstructed", theta_bins + 1, -0.5, 180.5);
    m_histHoughThetaDist->GetXaxis()->SetTitle(xAxisTheta);
    m_histHoughThetaDist->GetYaxis()->SetTitle("# tracks");
    //m_histCorrectReconMCThetaDist = new TH1D("CorrectReconMCThetaDist", "Theta of correctly reconstructed tracks", theta_bins+1, 16.5, 150.5);
    m_histCorrectReconMCThetaDist = new TH1D("CorrectReconMCThetaDist", "Theta of correctly reconstructed tracks",  theta_bins + 1,
                                             -0.5, 180.5);
    m_histCorrectReconMCThetaDist->GetXaxis()->SetTitle(xAxisTheta);
    m_histCorrectReconMCThetaDist->GetYaxis()->SetTitle("# tracks");
    //m_histCorrectReconHoughThetaDist = new TH1D("CorrectReconHoughThetaDist", "Theta of correctly reconstructed tracks", theta_bins+1, 16.5, 150.5);
    m_histCorrectReconHoughThetaDist = new TH1D("CorrectReconHoughThetaDist", "Theta of correctly reconstructed tracks", theta_bins + 1,
                                                -0.5,
                                                180.5);
    m_histCorrectReconHoughThetaDist->GetXaxis()->SetTitle(xAxisTheta);
    m_histCorrectReconHoughThetaDist->GetYaxis()->SetTitle("# tracks");
    //m_histCorrectReconThetaDist = new TH1D("CorrectReconThetaDist", "Theta of correctly reconstructed tracks", theta_bins+1, 16.5, 150.5);
    m_histCorrectReconThetaDist = new TH1D("CorrectReconThetaDist", "Theta of correctly reconstructed tracks", theta_bins + 1, -0.5,
                                           180.5);
    m_histCorrectReconThetaDist->GetXaxis()->SetTitle(xAxisTheta);
    m_histCorrectReconThetaDist->GetYaxis()->SetTitle("# tracks");
    //m_histEffHoughThetavsMCTheta = new TH1D("EffHoughThetavsMCTheta", "Efficiency of Reconstruction of Theta", theta_bins+1, 16.5, 150.5);
    m_histEffHoughThetavsMCTheta = new TH1D("EffHoughThetavsMCTheta", "Efficiency of Reconstruction of Theta", theta_bins + 1, -0.5,
                                            180.5);
    m_histEffHoughThetavsMCTheta->GetXaxis()->SetTitle(xAxisTheta);
    m_histEffHoughThetavsMCTheta->GetYaxis()->SetTitle("# tracks");

    m_histProjectedThetaDist = new TH1D("ProjThetaDist", "Projected Theta Distribution", theta_bins + 1, -0.5, 180.5);
    m_histProjectedThetaDist->GetXaxis()->SetTitle(xAxisTheta);
    m_histProjectedThetaDist->GetYaxis()->SetTitle("# reconstructed tracks");
    //m_histProjectedThetaRecon = new TH1D("ProjThetarecon", "Projected Theta of reconstructed tracks", 181, 0, 180);
    m_histProjectedThetaRecon = new TH1D("ProjThetarecon", "Projected Theta of reconstructed tracks", theta_bins + 1, -0.5, 180.5);
    m_histProjectedThetaRecon->GetXaxis()->SetTitle(xAxisTheta);
    m_histProjectedThetaRecon->GetYaxis()->SetTitle("# reconstructed tracks");
    //m_histEffProjectedTheta = new TH1D("EffProjTheta", "Efficiency vs Projected Theta", 181, 0, 180);
    m_histEffProjectedTheta = new TH1D("EffProjTheta", "Efficiency vs Projected Theta", theta_bins + 1,  -0.5, 180.5);
    m_histEffProjectedTheta->GetXaxis()->SetTitle(xAxisTheta);
    m_histEffProjectedTheta->GetYaxis()->SetTitle("#epsilon");



    m_histHoughThetavsMCPhiDist = new TH1D("HoughThetavsMCPhiDist", "Theta Eff vs Phi", phi_bins + 1, -180.5, 180.5);
    m_histHoughThetavsMCPhiDist->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughThetavsMCPhiDist->GetYaxis()->SetTitle("# reconstructed tracks");
    m_histCorrectReconHoughThetavsMCPhiDist = new TH1D("CorrectReconHoughThetavsMCPhiDist", "Theta Eff vs Phi", phi_bins + 1, -180.5,
                                                       180.5);
    m_histCorrectReconHoughThetavsMCPhiDist->GetXaxis()->SetTitle(xAxisPhi);
    m_histCorrectReconHoughThetavsMCPhiDist->GetYaxis()->SetTitle("# reconstructed tracks");
    /* Histogram for Theta-reconstruction-efficiency vs Phi */
    m_histHoughThetaEffvsMCPhi = new TH1D("HoughThetaEffvsMCPhi", "Theta Efficiency vs Phi", phi_bins + 1, -180.5, 180.5);
    m_histHoughThetaEffvsMCPhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughThetaEffvsMCPhi->GetYaxis()->SetTitle("#epsilon");


    /* Hough angle alpha vs MC angle beta or vice versa */
    m_histHoughPhivsMCPhi2D_all = new TH2D("HoughPhiMCPhi_all", "HoughPhivsMCPhi all", phi_bins + 1, -180.5, 180.5, phi_bins + 1,
                                           -180.5, 180.5);
    m_histHoughPhivsMCPhi2D_all->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughPhivsMCPhi2D_all->GetYaxis()->SetTitle(xAxisPhi);
    m_histHoughPhivsMCPhi2D_reco = new TH2D("HoughPhiMCPhi_reco", "HoughPhivsMCPhi reco", phi_bins + 1, -180.5, 180.5, phi_bins + 1,
                                            -180.5, 180.5);
    m_histHoughPhivsMCPhi2D_reco->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughPhivsMCPhi2D_reco->GetYaxis()->SetTitle(xAxisPhi);
    //m_histHoughThetavsMCTheta = new TH2D("HoughThetaMCTheta", "HoughThetavsMCTheta", theta_bins+1, 16.5, 150.5, theta_bins+1, 16.5, 150.5);
    //m_histHoughThetavsMCTheta2D_all = new TH2D("HoughThetaMCTheta_all", "HoughThetavsMCTheta all", theta_bins+1, 16.5, 150.5, 720, -360., 360.);
    m_histHoughThetavsMCTheta2D_all = new TH2D("HoughThetaMCTheta_all", "HoughThetavsMCTheta all",  theta_bins + 1, -0.5, 180.5, 720,
                                               -360., 360.);
    m_histHoughThetavsMCTheta2D_all->GetXaxis()->SetTitle(xAxisTheta);
    m_histHoughThetavsMCTheta2D_all->GetYaxis()->SetTitle(xAxisTheta);
    //m_histHoughThetavsMCTheta2D_reco = new TH2D("HoughThetaMCTheta_reco", "HoughThetavsMCTheta reco", theta_bins+1, 16.5, 150.5, 720, -360., 360.);
    m_histHoughThetavsMCTheta2D_reco = new TH2D("HoughThetaMCTheta_reco", "HoughThetavsMCTheta reco",  theta_bins + 1, -0.5, 180.5, 720,
                                                -360.,
                                                360.);
    m_histHoughThetavsMCTheta2D_reco->GetXaxis()->SetTitle(xAxisTheta);
    m_histHoughThetavsMCTheta2D_reco->GetYaxis()->SetTitle(xAxisTheta);
    //m_histHoughPhivsMCTheta2D_all = new TH2D("HoughPhiMCTheta_all", "HoughPhivsMCTheta all", theta_bins+1, 16.5, 150.5, 720, -360., 360.);
    m_histHoughPhivsMCTheta2D_all = new TH2D("HoughPhiMCTheta_all", "HoughPhivsMCTheta all",  theta_bins + 1, -0.5, 180.5, 720, -360.,
                                             360.);
    m_histHoughPhivsMCTheta2D_all->GetXaxis()->SetTitle(xAxisTheta);
    m_histHoughPhivsMCTheta2D_all->GetYaxis()->SetTitle(xAxisPhi);
    //m_histHoughPhivsMCTheta2D_reco = new TH2D("HoughPhiMCTheta_reco", "HoughPhivsMCTheta reco", theta_bins+1, 16.5, 150.5, 720, -360., 360.);
    m_histHoughPhivsMCTheta2D_reco = new TH2D("HoughPhiMCTheta_reco", "HoughPhivsMCTheta reco", theta_bins + 1, -0.5, 180.5, 720, -360.,
                                              360.);
    m_histHoughPhivsMCTheta2D_reco->GetXaxis()->SetTitle(xAxisTheta);
    m_histHoughPhivsMCTheta2D_reco->GetYaxis()->SetTitle(xAxisPhi);
    m_histHoughThetavsMCPhi2D_all = new TH2D("HoughThetaMCPhi_all", "HoughThetavsMCPhi all", phi_bins + 1, -180.5, 180.5, 720, -360.,
                                             360.);
    m_histHoughThetavsMCPhi2D_all->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughThetavsMCPhi2D_all->GetYaxis()->SetTitle(xAxisTheta);
    m_histHoughThetavsMCPhi2D_reco = new TH2D("HoughThetaMCPhi_reco", "HoughThetavsMCPhi reco", phi_bins + 1, -180.5, 180.5, 720, -360.,
                                              360.);
    m_histHoughThetavsMCPhi2D_reco->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughThetavsMCPhi2D_reco->GetYaxis()->SetTitle(xAxisTheta);

    /** Histogram for 2D distribution and effficiency of Theta vs Phi */
    //m_histHoughThetaDistvsMCPhiDist2D = new TH2D("HoughThetaDistvsMCPhiDist2D", "MCTheta vs MCPhi Distribution ", phi_bins+1, -180.5, 180.5, theta_bins+1, 16.5, 150.5);
    m_histHoughThetaDistvsMCPhiDist2D = new TH2D("HoughThetaDistvsMCPhiDist2D", "MCTheta vs MCPhi Distribution ", phi_bins + 1, -180.5,
                                                 180.5, theta_bins + 1, -0.5, 180.5);
    m_histHoughThetaDistvsMCPhiDist2D->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughThetaDistvsMCPhiDist2D->GetYaxis()->SetTitle(xAxisTheta);
    //m_histMCThetaCorrectReconvsMCPhi2D = new TH2D("MCThetaCorrectReconvsMCPhi2D", "MCTheta vs MCPhi Distribution ", phi_bins+1, -180.5, 180.5, theta_bins+1, 16.5, 150.5);
    m_histMCThetaCorrectReconvsMCPhi2D = new TH2D("MCThetaCorrectReconvsMCPhi2D", "MCTheta vs MCPhi Distribution ", phi_bins + 1,
                                                  -180.5, 180.5, theta_bins + 1, -0.5, 180.5);
    m_histMCThetaCorrectReconvsMCPhi2D->GetXaxis()->SetTitle(xAxisPhi);
    m_histMCThetaCorrectReconvsMCPhi2D->GetYaxis()->SetTitle(xAxisTheta);
    //m_histHoughThetaCorrectReconvsMCPhi2D = new TH2D("HoughThetaCorrectReconvsMCPhi2D", "Reconstructed Theta vs MCPhi", phi_bins+1, -180.5, 180.5, theta_bins+1, 16.5, 150.5);
    m_histHoughThetaCorrectReconvsMCPhi2D = new TH2D("HoughThetaCorrectReconvsMCPhi2D", "Reconstructed Theta vs MCPhi", phi_bins + 1,
                                                     -180.5, 180.5, theta_bins + 1, -0.5, 180.5);
    m_histHoughThetaCorrectReconvsMCPhi2D->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughThetaCorrectReconvsMCPhi2D->GetYaxis()->SetTitle(xAxisTheta);
    //m_histThetaCorrectReconvsMCPhi2D = new TH2D("ThetaCorrectReconvsMCPhi2D", "Efficiency of Reconstruction of Theta vs MCPhi", phi_bins+1, -180.5, 180.5, theta_bins+1, 16.5, 150.5);
    m_histThetaCorrectReconvsMCPhi2D = new TH2D("ThetaCorrectReconvsMCPhi2D", "Efficiency of Reconstruction of Theta vs MCPhi",
                                                phi_bins + 1, -180.5, 180.5, theta_bins + 1, -0.5, 180.5);
    m_histThetaCorrectReconvsMCPhi2D->GetXaxis()->SetTitle(xAxisPhi);
    m_histThetaCorrectReconvsMCPhi2D->GetYaxis()->SetTitle(xAxisTheta);
    //m_histEffMCThetavsMCPhi2D = new TH2D("EffMCThetavsMCPhi2D", "Efficiency of Reconstruction of Theta vs MCPhi", phi_bins+1, -180.5, 180.5, theta_bins+1, 16.5, 150.5);
    m_histEffMCThetavsMCPhi2D = new TH2D("EffMCThetavsMCPhi2D", "Efficiency of Reconstruction of Theta vs MCPhi", phi_bins + 1, -180.5,
                                         180.5, theta_bins + 1, -0.5, 180.5);
    m_histEffMCThetavsMCPhi2D->GetXaxis()->SetTitle(xAxisPhi);
    m_histEffMCThetavsMCPhi2D->GetYaxis()->SetTitle(xAxisTheta);
    //m_histEffHoughThetavsMCPhi2D = new TH2D("EffHoughThetavsMCPhi2D", "Efficiency of Reconstruction of Theta vs MCPhi", phi_bins+1, -180.5, 180.5, theta_bins+1, 16.5, 150.5);
    m_histEffHoughThetavsMCPhi2D = new TH2D("EffHoughThetavsMCPhi2D", "Efficiency of Reconstruction of Theta vs MCPhi", phi_bins + 1,
                                            -180.5, 180.5, theta_bins + 1, -0.5, 180.5);
    m_histEffHoughThetavsMCPhi2D->GetXaxis()->SetTitle(xAxisPhi);
    m_histEffHoughThetavsMCPhi2D->GetYaxis()->SetTitle(xAxisTheta);



    /* Fake plots */
    fakeDir = m_rootFile->mkdir("Fake");
    fakeDir->cd();
    /* Histogram for fake rate vs pT (only in phi while reconstruction in theta is not proved) */
//    m_histPtFakesPhi = new TH1D("pTFakePhi", "Fake rate vs pT (in Phi)", pt_bins, 0.00, 3.0);
//    m_histPtFakesPhi->GetXaxis()->SetTitle(xAxisPt);
//    m_histPtFakesPhi->GetYaxis()->SetTitle("# fake tracks");
    /* Histogram for fake rate vs Phi */
//    m_histFakesPhi = new TH1D("PhiFake", "Fake rate vs Phi", phi_bins+1, -180.5, 180.5);
//    m_histFakesPhi->GetXaxis()->SetTitle(xAxisPhi);
//    m_histFakesPhi->GetYaxis()->SetTitle("Number of Fake Tracks");
    /* Histogram for fake rate vs Theta */
//    m_histFakesTheta = new TH1D("ThetaFake", "Fake rate vs Theta", theta_bins+1, 16.5, 150.5);
//    m_histFakesTheta->GetXaxis()->SetTitle(xAxisTheta);
//    m_histFakesTheta->GetYaxis()->SetTitle("# fake tracks");
    /* Histogram for average fake rate */
//    m_histPtAverageFakes = new TH1D("AverageFakepT", "Average # fake tracks vs pT", pt_bins, 0.00, 3.0);
//    m_histPtAverageFakes->GetXaxis()->SetTitle(xAxisPt);
//    m_histPtAverageFakes->GetYaxis()->SetTitle("Average # fake tracks");
//    m_histPhiAverageFakes = new TH1D("AverageFakePhi", "Average Number of Fake Tracks vs Phi", phi_bins+1, -180.5, 180.5);
//    m_histPhiAverageFakes->GetXaxis()->SetTitle(xAxisPhi);
//    m_histPhiAverageFakes->GetYaxis()->SetTitle("Average # fake tracks");
//    m_histThetaAverageFakes = new TH1D("AverageFakeTheta", "Average Number of Fake Tracks vs Theta", theta_bins+1, 16.5, 150.5);
//    m_histThetaAverageFakes->GetXaxis()->SetTitle(xAxisTheta);
//    m_histThetaAverageFakes->GetYaxis()->SetTitle("Average # fake tracks");
    /* Histogram for fake rate vs pT (only in phi while reconstruction in theta is not proved) */
//    m_histPtFakes = new TH1D("pTFake", "Fake rate vs pT", pt_bins, 0.00, 3.0);
//    m_histPtFakes->GetXaxis()->SetTitle(xAxisPt);
//    m_histPtFakes->GetYaxis()->SetTitle("Fake");

    m_histFakesinPTvsMCPT = new TH1D("FakesinPTvsMCPT", "Fake Hits vs PT", pt_bins, 0.00, 3.0);
    m_histFakesinPTvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histFakesinPTvsMCPT->GetYaxis()->SetTitle("# Fakes");
    m_histFakesinPhivsMCPhi = new TH1D("FakesinPhivsMCPhi", "Fake Hits vs Phi", phi_bins + 1, -180.5, 180.5);
    m_histFakesinPhivsMCPhi->GetXaxis()->SetTitle(xAxisPt);
    m_histFakesinPhivsMCPhi->GetYaxis()->SetTitle("# Fakes");
    m_histFakesinThetavsMCTheta = new TH1D("FakesinThetavsMCTheta", "Fake Hits vs Theta", theta_bins + 1, -0.5, 180.5);
    m_histFakesinThetavsMCTheta->GetXaxis()->SetTitle(xAxisPt);
    m_histFakesinThetavsMCTheta->GetYaxis()->SetTitle("# Fakes");

    m_histAverageFakesvsMCPT = new TH1D("AverageFakesvsMCPT", "Average Number of Fake Hits vs PT", pt_bins, 0.00, 3.0);
    m_histAverageFakesvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histAverageFakesvsMCPT->GetYaxis()->SetTitle("# Fakes");
    m_histAverageFakesvsMCPhi = new TH1D("AverageFakesvsMCPhi", "Average Number of Fake Hits vs Phi", phi_bins + 1, -180.5, 180.5);
    m_histAverageFakesvsMCPhi->GetXaxis()->SetTitle(xAxisPt);
    m_histAverageFakesvsMCPhi->GetYaxis()->SetTitle("# Fakes");
    //m_histAverageFakesvsMCTheta = new TH1D("AverageFakesvsMCTheta", "Average Number of Fake Hits vs Theta", theta_bins+1, 16.5, 150.5);
    m_histAverageFakesvsMCTheta = new TH1D("AverageFakesvsMCTheta", "Average Number of Fake Hits vs Theta", theta_bins + 1, -0.5,
                                           180.5);
    m_histAverageFakesvsMCTheta->GetXaxis()->SetTitle(xAxisPt);
    m_histAverageFakesvsMCTheta->GetYaxis()->SetTitle("# Fakes");


    /* Missed hit distribution */
    //m_histMissedPt = new TH1D("MissedHitPt", "Missing Hits vs PT", pt_bins, 0.02, 3.0);
    //m_histMissedPhi = new TH1D("MissedHitPhi", "Missing Hits vs Phi", phi_bins+1, -180.5, 180.5);
    //m_histMissedTheta = new TH1D("MissedHitTheta", "Missing Hits vs Theta", theta_bins+1, 16.5, 150.5);
    m_histMissedHitsvsMCPT = new TH1D("MissedHitsvsMCPT", "Missing Hits vs PT", pt_bins, 0.00, 3.0);
    m_histMissedHitsvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histMissedHitsvsMCPT->GetYaxis()->SetTitle("counts");
    m_histMissedHitsvsMCPhi = new TH1D("MissedHitsvsMCPhi", "Missing Hits vs Phi", phi_bins + 1, -180.5, 180.5);
    m_histMissedHitsvsMCPhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histMissedHitsvsMCPhi->GetYaxis()->SetTitle("counts");
    //m_histMissedHitsvsMCTheta = new TH1D("MissedHitsvsMCTheta", "Missing Hits vs Theta", theta_bins+1, 16.5, 150.5);
    m_histMissedHitsvsMCTheta = new TH1D("MissedHitsvsMCTheta", "Missing Hits vs Theta", theta_bins + 1, -0.5, 180.5);
    m_histMissedHitsvsMCTheta->GetXaxis()->SetTitle(xAxisTheta);
    m_histMissedHitsvsMCTheta->GetYaxis()->SetTitle("counts");


    /* ROI plots */
    roiDir = m_rootFile->mkdir("ROI");
    roiDir->cd();

    /** Event ROI efficency */
    m_histEventROIEfficiency = new TH1D("EventROIEfficiency", "Overview of ROI Efficiency", 101, 0.0, 1.01);
    m_histEventROIEfficiency->GetXaxis()->SetTitle("ROI Efficiency");
    m_histEventROIEfficiency->GetYaxis()->SetTitle("Counts");
    /** Event DRF */
    m_histEventDRF = new TH1D("EventDRF", "Overview of Data Reduction Factor (DRF)", 300, 0.0, 150.0);
    m_histEventDRF->GetXaxis()->SetTitle("DRF");
    m_histEventDRF->GetYaxis()->SetTitle("Counts");

    /* Signed deviation histograms */
    /* Distance difference in Phi */
    m_histROIDiffPhi = new TH1D("ROIDiffPhi", "Difference in Phi", 1001, -0.3003, 0.3003);
    m_histROIDiffPhi->GetXaxis()->SetTitle("distance / cm");
    m_histROIDiffPhi->GetYaxis()->SetTitle("# tracks");
    m_histROIDiffPhiPx = new TH1D("ROIDiffPhiPx", "Difference in Phi", 121, -60.5, 60.5);
    m_histROIDiffPhiPx->GetXaxis()->SetTitle("distance / px");
    m_histROIDiffPhiPx->GetYaxis()->SetTitle("# tracks");
    m_histROIDiffTheta = new TH1D("ROIDiffTheta", "Difference in Theta", 401, -1.0025, 1.0025);
    m_histROIDiffTheta->GetXaxis()->SetTitle("distance / cm");
    m_histROIDiffTheta->GetYaxis()->SetTitle("# tracks");
    m_histROIDiffThetaPx = new TH1D("ROIDiffThetaPx", "Difference in Theta", 121, -60.5, 60.5);
    m_histROIDiffThetaPx->GetXaxis()->SetTitle("distance / px");
    m_histROIDiffThetaPx->GetYaxis()->SetTitle("# tracks");

    m_histROIDiffPhiTheta = new TH2D("ROIDiffPhiTheta", "Difference in Phi and Theta", 1001, -0.3003, 0.3003, 401, -1.0025, 1.0025);
    m_histROIDiffPhiTheta->GetXaxis()->SetTitle("distance in #varphi / cm");
    m_histROIDiffPhiTheta->GetYaxis()->SetTitle("distance in #theta / cm");
    m_histROIDiffPhiThetaPx = new TH2D("ROIDiffPhiThetaPx", "Difference in Phi and Theta", 121, -60.5, 60.5, 121, -60.5, 60.5);
    m_histROIDiffPhiThetaPx->GetXaxis()->SetTitle("distance in #varphi / px");
    m_histROIDiffPhiThetaPx->GetYaxis()->SetTitle("distance in #theta / px");

    //m_histROIDiffPT = new TH2D("ROIDiffPT", "Absolute MPH deviation compared to p_{T}", 300, 0.0, 3.0, 500, 0.0 , 2.0);
    //m_histROIDiffPT->GetXaxis()->SetTitle(xAxisPT);
    //m_histROIDiffPT->GetYaxis()->SetTitle("distance / cm");
    m_histROIDiffPTPhi = new TH2D("ROIDiffPTPhi", "Difference in #varphi vs p_{T} (in cm)", 301, -0.005, 3.005, 1001, -0.3003, 0.3003);
    m_histROIDiffPTPhi->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTPhi->GetYaxis()->SetTitle("distance / cm");
    m_histROIDiffPTPhiPx = new TH2D("ROIDiffPTPhiPx", "Difference in #varphi vs p_{T} (in pixels)", 301, -0.005, 3.005, 121, -60.5,
                                    60.5);
    m_histROIDiffPTPhiPx->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTPhiPx->GetYaxis()->SetTitle("distance / px");
    m_histROIDiffPTTheta = new TH2D("ROIDiffPTTheta", "Difference in #theta vs p_{T} (in cm)", 301, -0.005, 3.005, 401, -1.0025,
                                    1.0025);
    m_histROIDiffPTTheta->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTTheta->GetYaxis()->SetTitle("distance / cm");
    m_histROIDiffPTThetaPx = new TH2D("ROIDiffPTThetaPx", "Difference in #theta vs p_{T} (in pixels)", 301, -0.005, 3.005, 121, -60.5,
                                      60.5);
    m_histROIDiffPTThetaPx->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTThetaPx->GetYaxis()->SetTitle("distance / px");

    /** PXD Layer 1 */
    m_histROIDiffPhi_pxdl1 = new TH1D("ROIDiffPhi_pxdl1", "Difference in Phi", 1001, -0.3003, 0.3003);
    m_histROIDiffPhi_pxdl1->GetXaxis()->SetTitle("distance / cm");
    m_histROIDiffPhi_pxdl1->GetYaxis()->SetTitle("# tracks");
    m_histROIDiffPhiPx_pxdl1 = new TH1D("ROIDiffPhiPx_pxdl1", "Difference in Phi", 121, -60.5, 60.5);
    m_histROIDiffPhiPx_pxdl1->GetXaxis()->SetTitle("distance / px");
    m_histROIDiffPhiPx_pxdl1->GetYaxis()->SetTitle("# tracks");
    m_histROIDiffTheta_pxdl1 = new TH1D("ROIDiffTheta_pxdl1", "Difference in Theta", 401, -1.0025, 1.0025);
    m_histROIDiffTheta_pxdl1->GetXaxis()->SetTitle("distance / cm");
    m_histROIDiffTheta_pxdl1->GetYaxis()->SetTitle("# tracks");
    m_histROIDiffThetaPx_pxdl1 = new TH1D("ROIDiffThetaPx_pxdl1", "Difference in Theta", 121, -60.5, 60.5);
    m_histROIDiffThetaPx_pxdl1->GetXaxis()->SetTitle("distance / px");
    m_histROIDiffThetaPx_pxdl1->GetYaxis()->SetTitle("# tracks");

    m_histROIDiffPhiTheta_pxdl1 = new TH2D("ROIDiffPhiTheta_pxdl1", "Difference in Phi and Theta", 1001, -0.3003, 0.3003, 401, -1.0025,
                                           1.0025);
    m_histROIDiffPhiTheta_pxdl1->GetXaxis()->SetTitle("distance in #varphi / cm");
    m_histROIDiffPhiTheta_pxdl1->GetYaxis()->SetTitle("distance in #theta / cm");
    m_histROIDiffPhiThetaPx_pxdl1 = new TH2D("ROIDiffPhiThetaPx_pxdl1", "Difference in Phi and Theta", 121, -60.5, 60.5, 121, -60.5,
                                             60.5);
    m_histROIDiffPhiThetaPx_pxdl1->GetXaxis()->SetTitle("distance in #varphi / px");
    m_histROIDiffPhiThetaPx_pxdl1->GetYaxis()->SetTitle("distance in #theta / px");

    m_histROIDiffPTPhi_pxdl1 = new TH2D("ROIDiffPTPhi_pxdl1", "Difference in #varphi vs p_{T} (in cm)", 301, -0.005, 3.005, 1001,
                                        -0.3003, 0.3003);
    m_histROIDiffPTPhi_pxdl1->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTPhi_pxdl1->GetYaxis()->SetTitle("distance / cm");
    m_histROIDiffPTPhiPx_pxdl1 = new TH2D("ROIDiffPTPhiPx_pxdl1", "Difference in #varphi vs p_{T} (in pixels)", 301, -0.005, 3.005, 121,
                                          -60.5, 60.5);
    m_histROIDiffPTPhiPx_pxdl1->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTPhiPx_pxdl1->GetYaxis()->SetTitle("distance / px");
    m_histROIDiffPTTheta_pxdl1 = new TH2D("ROIDiffPTTheta_pxdl1", "Difference in #theta vs p_{T} (in cm)", 301, -0.005, 3.005, 401,
                                          -1.0025, 1.0025);
    m_histROIDiffPTTheta_pxdl1->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTTheta_pxdl1->GetYaxis()->SetTitle("distance / cm");
    m_histROIDiffPTThetaPx_pxdl1 = new TH2D("ROIDiffPTThetaPx_pxdl1", "Difference in #theta vs p_{T} (in pixels)", 301, -0.005, 3.005,
                                            121, -60.5, 60.5);
    m_histROIDiffPTThetaPx_pxdl1->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTThetaPx_pxdl1->GetYaxis()->SetTitle("distance / px");

    /** PXD Layer 2 */
    m_histROIDiffPhi_pxdl2 = new TH1D("ROIDiffPhi_pxdl2", "Difference in Phi", 1001, -0.3003, 0.3003);
    m_histROIDiffPhi_pxdl2->GetXaxis()->SetTitle("distance / cm");
    m_histROIDiffPhi_pxdl2->GetYaxis()->SetTitle("# tracks");
    m_histROIDiffPhiPx_pxdl2 = new TH1D("ROIDiffPhiPx_pxdl2", "Difference in Phi", 121, -60.5, 60.5);
    m_histROIDiffPhiPx_pxdl2->GetXaxis()->SetTitle("distance / px");
    m_histROIDiffPhiPx_pxdl2->GetYaxis()->SetTitle("# tracks");
    m_histROIDiffTheta_pxdl2 = new TH1D("ROIDiffTheta_pxdl2", "Difference in Theta", 401, -1.0025, 1.0025);
    m_histROIDiffTheta_pxdl2->GetXaxis()->SetTitle("distance / cm");
    m_histROIDiffTheta_pxdl2->GetYaxis()->SetTitle("# tracks");
    m_histROIDiffThetaPx_pxdl2 = new TH1D("ROIDiffThetaPx_pxdl2", "Difference in Theta", 121, -60.5, 60.5);
    m_histROIDiffThetaPx_pxdl2->GetXaxis()->SetTitle("distance / px");
    m_histROIDiffThetaPx_pxdl2->GetYaxis()->SetTitle("# tracks");

    m_histROIDiffPhiTheta_pxdl2 = new TH2D("ROIDiffPhiTheta_pxdl2", "Difference in Phi and Theta", 1001, -0.3003, 0.3003, 401, -1.0025,
                                           1.0025);
    m_histROIDiffPhiTheta_pxdl2->GetXaxis()->SetTitle("distance in #varphi / cm");
    m_histROIDiffPhiTheta_pxdl2->GetYaxis()->SetTitle("distance in #theta / cm");
    m_histROIDiffPhiThetaPx_pxdl2 = new TH2D("ROIDiffPhiThetaPx_pxdl2", "Difference in Phi and Theta", 121, -60.5, 60.5, 121, -60.5,
                                             60.5);
    m_histROIDiffPhiThetaPx_pxdl2->GetXaxis()->SetTitle("distance in #varphi / px");
    m_histROIDiffPhiThetaPx_pxdl2->GetYaxis()->SetTitle("distance in #theta / px");

    m_histROIDiffPTPhi_pxdl2 = new TH2D("ROIDiffPTPhi_pxdl2", "Difference in #varphi vs p_{T} (in cm)", 301, -0.005, 3.005, 1001,
                                        -0.3003, 0.3003);
    m_histROIDiffPTPhi_pxdl2->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTPhi_pxdl2->GetYaxis()->SetTitle("distance / cm");
    m_histROIDiffPTPhiPx_pxdl2 = new TH2D("ROIDiffPTPhiPx_pxdl2", "Difference in #varphi vs p_{T} (in pixels)", 301, -0.005, 3.005, 121,
                                          -60.5, 60.5);
    m_histROIDiffPTPhiPx_pxdl2->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTPhiPx_pxdl2->GetYaxis()->SetTitle("distance / px");
    m_histROIDiffPTTheta_pxdl2 = new TH2D("ROIDiffPTTheta_pxdl2", "Difference in #theta vs p_{T} (in cm)", 301, -0.005, 3.005, 401,
                                          -1.0025, 1.0025);
    m_histROIDiffPTTheta_pxdl2->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTTheta_pxdl2->GetYaxis()->SetTitle("distance / cm");
    m_histROIDiffPTThetaPx_pxdl2 = new TH2D("ROIDiffPTThetaPx_pxdl2", "Difference in #theta vs p_{T} (in pixels)", 301, -0.005, 3.005,
                                            121, -60.5, 60.5);
    m_histROIDiffPTThetaPx_pxdl2->GetXaxis()->SetTitle(xAxisPt);
    m_histROIDiffPTThetaPx_pxdl2->GetYaxis()->SetTitle("distance / px");



    /* Unsigned / absolute deviation histograms */
    //m_histROIDiffPhiabs = new TH1D("ROIDiffPhiabs", "Difference in Phi", 501, -0.0003, 0.3003);
    //m_histROIDiffPhiabs->GetXaxis()->SetTitle("distance / cm");
    //m_histROIDiffPhiPxabs = new TH1D("ROIDiffPhiPxabs", "Difference in Phi", 61, -0.5, 60.5);
    //m_histROIDiffPhiPxabs->GetXaxis()->SetTitle("distance / px");
    //m_histROIDiffThetaabs = new TH1D("ROIDiffThetaabs", "Difference in Theta", 201, -0.0025, 1.0025);
    //m_histROIDiffThetaabs->GetXaxis()->SetTitle("distance / cm");
    //m_histROIDiffThetaPxabs = new TH1D("ROIDiffThetaPxabs", "Difference in Theta", 61, -0.5, 60.5);
    //m_histROIDiffThetaPxabs->GetXaxis()->SetTitle("distance / px");

    //m_histROIDiffPhiThetaabs = new TH2D("ROIDiffPhiThetaabs", "Difference in Phi and Theta", 501, -0.0003, 0.3003, 201, -0.0025, 1.0025);
    //m_histROIDiffPhiThetaabs->GetXaxis()->SetTitle("distance in #varphi / cm");
    //m_histROIDiffPhiThetaabs->GetYaxis()->SetTitle("distance in #theta / cm");
    //m_histROIDiffPhiThetaPxabs = new TH2D("ROIDiffPhiThetaPxabs", "Difference in Phi and Theta", 61, -0.5, 60.5, 61, -0.5, 60.5);
    //m_histROIDiffPhiThetaPxabs->GetXaxis()->SetTitle("distance in #varphi / px");
    //m_histROIDiffPhiThetaPxabs->GetYaxis()->SetTitle("distance in #theta / px");

    //m_histROIDiffPTabs = new TH2D("ROIDiffPTabs", "Absolute MPH deviation compared to p_{T}", 301, -0.005, 3.005, 501, -0.002 , 2.002);
    //m_histROIDiffPTabs->GetXaxis()->SetTitle(xAxisPT);
    //m_histROIDiffPTabs->GetYaxis()->SetTitle("distance / cm");
    //m_histROIDiffPTPhiabs = new TH2D("ROIDiffPTPhiabs", "Difference in #varphi vs p_{T} (in cm)", 301, -0.005, 3.005, 501, -0.0003, 0.3003);
    //m_histROIDiffPTPhiabs->GetXaxis()->SetTitle(xAxisPT);
    //m_histROIDiffPTPhiabs->GetYaxis()->SetTitle("distance / cm");
    //m_histROIDiffPTPhiPxabs = new TH2D("ROIDiffPTPhiPxabs", "Difference in #varphi vs p_{T} (in pixels)", 301, -0.005, 3.005, 61, -0.5, 60.5);
    //m_histROIDiffPTPhiPxabs->GetXaxis()->SetTitle(xAxisPT);
    //m_histROIDiffPTPhiPxabs->GetYaxis()->SetTitle("distance / px");
    //m_histROIDiffPTThetaabs = new TH2D("ROIDiffPTThetaabs", "Difference in #theta vs p_{T} (in cm)", 301, -0.005, 3.005, 201, -0.0025, 1.0025);
    //m_histROIDiffPTThetaabs->GetXaxis()->SetTitle(xAxisPT);
    //m_histROIDiffPTThetaabs->GetYaxis()->SetTitle("distance / cm");
    //m_histROIDiffPTThetaPxabs = new TH2D("ROIDiffPTThetaPxabs", "Difference in #theta vs p_{T} (in pixels)", 301, -0.005, 3.005, 61, -0.5, 60.5);
    //m_histROIDiffPTThetaPxabs->GetXaxis()->SetTitle(xAxisPT);
    //m_histROIDiffPTThetaPxabs->GetYaxis()->SetTitle("distance / px");

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
SVDHoughTrackingModule::beginRun()
{
  /* Clear total clock cycles */
  totClockCycles = 0.0;

  /* Init some event variables */
  runNumber = 0;
  validEvents = 0;

  evtNumber = 0;
  MCParticleNumber = 0;
  HoughTrackNumber = 0;
  ExtrapolatedHitNumber = 0;

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
SVDHoughTrackingModule::endRun()
{
  TString xAxisPt = "pT [GeV]";
  TString xAxisPhi = "#varphi [deg]";
  TString xAxisTheta = "#theta [deg]";

  if (m_printStatistics) {
    B2INFO("End of Run: Hough Statistics: ");
    B2INFO("  Events processed: " << runNumber);
    B2INFO("  Reconstructed tracks: " << allTracks);
    B2INFO("  Total tracks found: " << ntotTracks);
    B2INFO("  Track total efficiency: " << curTrackEff / ((double) validEvents));
    B2INFO("  Track Theta efficiency: " << curTrackEffN / ((double) validEvents));
    B2INFO("  Track Phi efficiency: " << curTrackEffP / ((double) validEvents));
    B2INFO("  Fake Rate: " << ntotFakeTracks / ((double) validEvents));
    B2INFO("  Total time: " << totClockCycles << " ms");
    B2INFO("  Average time: " << totClockCycles / ((double) validEvents) << " ms");
    B2INFO("  Min R: " << minR << " Max R: " << maxR);

    if (m_PXDExtrapolation) {
      //B2INFO("  Average hits found: " << (ntotROITrueHits+totROISimHits) / ((double) validEvents));
      B2INFO("  Average hits found: " << ((double) ntotPXDactivePixel) / ((double) validEvents));
      //B2INFO("  Average Hits in ROIs: " << ((double) ncurHitsInROIs) / ((double) validEvents));
      B2INFO("  Average Hits in ROIs: " << ((double) ntotPXDinsideROIPixel) / ((double) validEvents));
      B2INFO("  ROI efficiency: " << curROIEff / ((double) validEvents));
      //B2INFO("  DRF: " << ((double) (ntotROITrueHits + ntotROISimHits)) / ((double) ncurHitsInROIs));
      B2INFO("  DRF: " << ((double) ntotPXDactivePixel) / ((double) ntotPXDinsideROIPixel));
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

    if (!m_independentSectors) {
      /* Check if stat file already exists, if not write legend */
      if (statout.tellp() == 0) {
        statout << "mergeN" << "\t" << "mergeP" << "\t" << "CritN" << "\t" << "CritP" << "\t" << "RectN" << "\t" << "RectP"
                << "\t" << "EffP" << "\t" << "EffN" << "\t" << "EffTot" << "\t" << "Fake" << "\t" << "ROI-Eff"
                << "\t" << "DRF" << endl;
      }


      statout << m_mergeThresholdN << "\t" << m_mergeThresholdP << "\t"
              << m_critIterationsN << "\t" << m_critIterationsP << "\t"
              << boost::format("%1.3f") % (double) m_rectSizeN << "\t"
              << boost::format("%1.3f") % (double) m_rectSizeP << "\t"
              << boost::format("%1.3f") % (curTrackEffP / (double) validEvents) << "\t"
              << boost::format("%1.3f") % (curTrackEffN / (double) validEvents) << "\t"
              << boost::format("%1.3f") % (curTrackEff / (double) validEvents) << "\t"
              << (ntotFakeTracks / ((double) validEvents)) << "\t"
              << boost::format("%1.3f") % (curROIEff / ((double) validEvents)) << "\t"
              //<< boost::format("%1.3f") % (((double) (ntotROITrueHits + ntotROISimHits)) / ((double) ncurHitsInROIs)) << "\t"
              << boost::format("%1.3f") % (((double) ntotPXDactivePixel) / ((double) ntotPXDinsideROIPixel)) << "\t"
              << endl;

    } else {
      /* Check if stat file already exists, if not write legend */
      if (statout.tellp() == 0) {
        statout << "mergeN" << "\t" << "mergeP" << "\t" << "xsecN"   << "\t" << "xsecP" << "\t" << "ysecN" << "\t"
                << "ysecP"  << "\t" << "RectN"  << "\t" << "RectP"   << "\t" << "EffP"  << "\t" << "EffN"  << "\t"
                << "EffTot" << "\t" << "Fake"   << "\t" << "ROI-Eff" << "\t" << "DRF"   << endl;
      }


      statout << m_mergeThresholdN << "\t" << m_mergeThresholdP << "\t"
              << m_angleSectorsN   << "\t" << m_angleSectorsP   << "\t"
              << m_vertSectorsN    << "\t" << m_vertSectorsP    << "\t"
              << boost::format("%1.4f") % (double) m_rectSizeN  << "\t"
              << boost::format("%1.4f") % (double) m_rectSizeP  << "\t"
              << boost::format("%1.4f") % (curTrackEffP / (double) validEvents) << "\t"
              << boost::format("%1.4f") % (curTrackEffN / (double) validEvents) << "\t"
              << boost::format("%1.4f") % (curTrackEff / (double) validEvents)  << "\t"
              << (ntotFakeTracks / ((double) validEvents)) << "\t"
              << boost::format("%1.4f") % (curROIEff / ((double) validEvents)) << "\t"
              //<< boost::format("%1.3f") % (((double) (ntotROITrueHits+ntotROISimHits)) / ((double) ncurHitsInROIs)) << "\t"
              << boost::format("%1.3f") % (((double) ntotPXDactivePixel) / ((double) ntotPXDinsideROIPixel)) << "\t"
              << endl;
    }

    /* Close Statistic output */
    statout.close();

  }

  /* Fill efficiency histograms */
  if (!m_rootFilename.empty()) {
    m_rootFile->cd("Efficiency"); /* Change to Efficiency directory */

    *m_histMCTrackREff = (*m_histHoughTrackR) / (*m_histMCTrackR);
    m_histMCTrackREff->SetName("MCTrackRadiusEff");
    m_histMCTrackREff->GetXaxis()->SetTitle("Track Radius / m");
    m_histMCTrackREff->GetYaxis()->SetTitle("#epsilon");
    m_histMCTrackREff->GetXaxis()->SetRangeUser(-12.0125, 12.0125);
    m_histMCTrackREff->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histHoughTrackREff = (*m_histCorrectReconHoughTrackR) / (*m_histHoughTrackR);
    m_histHoughTrackREff->SetName("HoughTrackRadiusEff");
    m_histHoughTrackREff->GetXaxis()->SetTitle("Track Radius / m");
    m_histHoughTrackREff->GetYaxis()->SetTitle("#epsilon");
    m_histHoughTrackREff->GetXaxis()->SetRangeUser(-12.0125, 12.0125);
    m_histHoughTrackREff->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histCorrectReconTrackREff = (*m_histCorrectReconTrackR) / (*m_histMCTrackR);
    m_histCorrectReconTrackREff->SetName("CorrectReconTrackREff");
    m_histCorrectReconTrackREff->GetXaxis()->SetTitle("Track Radius / m");
    m_histCorrectReconTrackREff->GetYaxis()->SetTitle("#epsilon");
    m_histCorrectReconTrackREff->GetXaxis()->SetRangeUser(-12.0125, 12.0125);
    m_histCorrectReconTrackREff->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histCurvatureSignEff = (*m_histCorrectReconCurvatureSign) / (*m_histMCCurvatureSign);
    m_histCurvatureSignEff->SetName("RecoEffvsTrackCurv");
    m_histCurvatureSignEff->GetXaxis()->SetTitle("Efficiency vs MCParticle Track-Curvature");
    m_histCurvatureSignEff->GetXaxis()->SetRangeUser(-1.5, 1.5);
    m_histCurvatureSignEff->GetYaxis()->SetTitle("#epsilon");
    m_histCurvatureSignEff->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histChargeEff = (*m_histCorrectReconCharge) / (*m_histMCCharge);
    m_histChargeEff->SetName("RecoEffvsCharge");
    m_histChargeEff->SetTitle("Efficiency of Reconstruction vs MCParticle Charge");
    m_histChargeEff->GetXaxis()->SetTitle("Charge of MC Particle / e");
    m_histChargeEff->GetXaxis()->SetRangeUser(-1.5, 1.5);
    m_histChargeEff->GetYaxis()->SetTitle("#epsilon");
    m_histChargeEff->GetYaxis()->SetRangeUser(0.0, 1.0);

//    *m_histPTEffvsMCPT = (*m_histCorrectReconHoughvsMCPTDist) / (*m_histMCPTDist);
    *m_histPTEffvsMCPT = (*m_histCorrectReconvsMCPTDist) / (*m_histMCPTDist);
    m_histPTEffvsMCPT->SetName("PTEffvsMCPT");
    m_histPTEffvsMCPT->SetTitle("Efficiency vs pT");
    m_histPTEffvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histPTEffvsMCPT->GetYaxis()->SetTitle("#epsilon");
    m_histPTEffvsMCPT->GetXaxis()->SetRangeUser(0.0, 3.0);
    m_histPTEffvsMCPT->GetYaxis()->SetRangeUser(0.0, 1.0);

//    *m_histHoughPhiEffvsMCPT = (*m_histCorrectReconHoughPhivsMCPTDist) / (*m_histMCPTDist);
    *m_histPhiEffvsMCPT = (*m_histCorrectReconPhivsMCPTDist) / (*m_histMCPTDist);
    m_histPhiEffvsMCPT->SetName("PhiEffvsMCPT");
    m_histPhiEffvsMCPT->SetTitle("Efficiency vs pT (of Phi-Reconstruction)");
    m_histPhiEffvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histPhiEffvsMCPT->GetYaxis()->SetTitle("#epsilon");
    m_histPhiEffvsMCPT->GetXaxis()->SetRangeUser(0.0, 3.0);
    m_histPhiEffvsMCPT->GetYaxis()->SetRangeUser(0.0, 1.0);

//    *m_histHoughThetaEffvsMCPT = (*m_histCorrectReconHoughThetavsMCPTDist) / (*m_histMCPTDist);
    *m_histThetaEffvsMCPT = (*m_histCorrectReconThetavsMCPTDist) / (*m_histMCPTDist);
    m_histThetaEffvsMCPT->SetName("ThetaEffvsMCPT");
    m_histThetaEffvsMCPT->SetTitle("Efficiency vs pT (of Theta-Reconstruction)");
    m_histThetaEffvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histThetaEffvsMCPT->GetYaxis()->SetTitle("#epsilon");
    m_histThetaEffvsMCPT->GetXaxis()->SetRangeUser(0.0, 3.0);
    m_histThetaEffvsMCPT->GetYaxis()->SetRangeUser(0.0, 1.0);

    //*m_histEffHoughPhivsMCPhi = (*m_histCorrectReconPhiDist) / (*m_histMCPhiDist);
    *m_histEffHoughPhivsMCPhi = (*m_histCorrectReconMCPhiDist) / (*m_histMCPhiDist);
    m_histEffHoughPhivsMCPhi->SetName("EffHoughPhivsMCPhi");
    m_histEffHoughPhivsMCPhi->SetTitle("Efficiency vs Phi");
    m_histEffHoughPhivsMCPhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histEffHoughPhivsMCPhi->GetYaxis()->SetTitle("#epsilon");
    m_histEffHoughPhivsMCPhi->GetYaxis()->SetRangeUser(0.0, 1.0);

    //*m_histEffHoughThetavsMCTheta = (*m_histCorrectReconThetaDist) / (*m_histMCThetaDist);
    *m_histEffHoughThetavsMCTheta = (*m_histCorrectReconMCThetaDist) / (*m_histMCThetaDist);
    m_histEffHoughThetavsMCTheta->SetName("EffHoughThetavsMCTheta");
    m_histEffHoughThetavsMCTheta->SetTitle("Efficiency vs Theta");
    m_histEffHoughThetavsMCTheta->GetXaxis()->SetTitle(xAxisTheta);
    m_histEffHoughThetavsMCTheta->GetYaxis()->SetTitle("#epsilon");
    m_histEffHoughThetavsMCTheta->GetYaxis()->SetRangeUser(0.0, 1.0);

    //*m_histHoughThetaEffvsMCPhi = (*m_histCorrectReconHoughThetavsMCPhiDist) / (*m_histMCPhiDist);
    *m_histHoughThetaEffvsMCPhi = (*m_histCorrectReconHoughThetavsMCPhiDist) / (*m_histMCPhiDist); //(*m_histHoughThetavsMCPhiDist);
    m_histHoughThetaEffvsMCPhi->SetName("HoughThetaEffvsMCPhi");
    m_histHoughThetaEffvsMCPhi->SetTitle("Theta Efficiency vs Phi");
    m_histHoughThetaEffvsMCPhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histHoughThetaEffvsMCPhi->GetYaxis()->SetTitle("#epsilon");
    m_histHoughThetaEffvsMCPhi->GetYaxis()->SetRangeUser(0.0, 1.0);

    *m_histEffProjectedTheta = (*m_histProjectedThetaRecon) / (*m_histProjectedThetaDist);
    m_histEffProjectedTheta->SetName("EffProjTheta");
    m_histEffProjectedTheta->SetTitle("Efficiency vs projected theta");
    m_histEffProjectedTheta->GetXaxis()->SetTitle("Projected #theta [deg]");

    *m_histEffHoughThetavsMCPhi2D = (*m_histHoughThetaCorrectReconvsMCPhi2D) / (*m_histHoughThetaDistvsMCPhiDist2D);
    m_histEffHoughThetavsMCPhi2D->SetName("EffHoughThetavsMCPhi2D");
    m_histEffHoughThetavsMCPhi2D->SetTitle("Theta Efficiency vs Phi in 2D");
    m_histEffHoughThetavsMCPhi2D->GetXaxis()->SetTitle(xAxisPhi);
    m_histEffHoughThetavsMCPhi2D->GetYaxis()->SetTitle(xAxisTheta);


    m_rootFile->cd("Fake"); /* Change to Fake directory */
    *m_histAverageFakesvsMCPT = (*m_histFakesinPTvsMCPT) / (*m_histMCPTDist);
    m_histAverageFakesvsMCPT->SetName("AverageFakesvsMCPT");
    m_histAverageFakesvsMCPT->SetTitle("Average # fake tracks vs pT");
    m_histAverageFakesvsMCPT->GetXaxis()->SetTitle(xAxisPt);
    m_histAverageFakesvsMCPT->GetYaxis()->SetTitle("Average # fake tracks");
    m_histAverageFakesvsMCPT->GetXaxis()->SetRangeUser(0.0, 3.0);

    *m_histAverageFakesvsMCPhi = (*m_histFakesinPhivsMCPhi) / (*m_histMCPhiDist);
    m_histAverageFakesvsMCPhi->SetName("AverageFakesvsMCPhi");
    m_histAverageFakesvsMCPhi->SetTitle("Average Number of Fake Tracks vs Phi");
    m_histAverageFakesvsMCPhi->GetXaxis()->SetTitle(xAxisPhi);
    m_histAverageFakesvsMCPhi->GetYaxis()->SetTitle("Average # fake tracks");

    *m_histAverageFakesvsMCTheta = (*m_histFakesinThetavsMCTheta) / (*m_histMCThetaDist);
    m_histAverageFakesvsMCTheta->SetName("AverageFakesvsMCTheta");
    m_histAverageFakesvsMCTheta->SetTitle("Average Number of Fake Tracks vs Theta");
    m_histAverageFakesvsMCTheta->GetXaxis()->SetTitle(xAxisTheta);
    m_histAverageFakesvsMCTheta->GetYaxis()->SetTitle("Average # of fake tracks");

  }
}

void
SVDHoughTrackingModule::event()
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<SVDCluster> storeSVDCluster(m_storeSVDClusterName);
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);

  TVector3 vec, abs_pos;
  float u, v;
  std::map<int, TVector3>::iterator iter;

  /** Do analysis of tracks w/o tracking and extrapolation */
  if (m_StandAloneAnalysis) {
    trackAnalyseMCParticle();
    analyseExtrapolatedFull();

    createROI();
    analyseROI();

    return;
  }



  if (!storeHoughCluster.isValid()) {
    storeHoughCluster.create();
  } else {
    storeHoughCluster.getPtr()->Clear();
  }

  /* Reset cluster index variables */
  p_idx = 0;
  n_idx = 0;

  /* Increase / set variables for event numbers and efficency calculations */
  ++runNumber;
  ++evtNumber;
  MCParticleNumber = 0;
  HoughTrackNumber = 0;
  ExtrapolatedHitNumber = 0;
  eventROIEff = 0;
  eventDRF = 0;

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
    /** ATTENTION!!!! Old part of code, no longer used, will be deleted soon! */
    /** Looks like this part is for the old SectorNeighbourFinder algorithm, which is not used
     * any longer. So probably this part is obsolete and will be deleted in (near) future to
     * make the code slimmer and more readable.
     */

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
    create_sector_map();    // Function belonging to the SectorNeighbourFinder
    /* Use straight PXD extrapolation */
    /* ATTENTION: since this part of the algorithm belongs to the old SectorNeighbourFinder
     * the pxdStraightExtrapolation function was deleted!
     */
//    pxdStraightExtrapolation();
    /* Run the analyzer */
//    analyseExtrapolatedHits();


    B2DEBUG(200, "Number of SVD Cluster candidates " << cluster_map.size());
    for (iter = cluster_map.begin(); iter != cluster_map.end(); ++iter) {
      storeHoughCluster.appendNew(SVDHoughCluster(iter->first, iter->second));
      B2DEBUG(200, "Cluster candidate " << iter->first << ": " << iter->second.Px()
              << " " << iter->second.Py()
              << " " << iter->second.Pz());
    }

#ifdef INJECT_GHOSTS
    /* Add ghosts from dead strips */
    iter = cluster_map.end();

    B2DEBUG(200, "Number of SVD Cluster candidates " << cluster_map.size());
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


void
SVDHoughTrackingModule::terminate()
{
  B2DEBUG(250, "End of SVDHoughTrack");
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }

  of_pos.close();
  of_strip.close();
}


/* vim: set tabstop=2:softtabstop=2:shiftwidth=2:noexpandtab */

