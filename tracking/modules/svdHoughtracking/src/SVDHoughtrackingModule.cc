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

#undef B2DEBUG
#define B2DEBUG(level, streamText) \
  B2LOGMESSAGE_IFENABLED(LogConfig::c_Debug, level, streamText, PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__)



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDHoughtracking)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDHoughtrackingModule::SVDHoughtrackingModule() : Module(), curTrackEff(0.0), totFakeTracks(0.0), totTracks(0),
  m_rootFile(0), m_histROIy(0), m_histROIz(0), m_histSimHits(0)
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
  addParam("FullTrackingPipeline", m_fullTrackingPipeline,
           "Run full tracking pipelin?", bool(false));
  addParam("WriteHoughSpace", m_writeHoughSpace,
           "Write Hough space into a gnuplot file?", bool(false));
  addParam("UseSensorFilter", m_useSensorFilter,
           "Use the Sensor layer filter", bool(true));
  addParam("UseRadiusFilter", m_useRadiusFilter,
           "Use the Sensor layer filter", bool(true));
  addParam("UseHashPurify", m_useHashPurify,
           "Chose the Hash (true), or the List purifier", bool(false));
  addParam("UseTrackMerger", m_useTrackMerger,
           "Use the track merger", bool(false));
  addParam("CompareWithMCParticle", m_compareMCParticle,
           "Compare reconstructed tracks with MC Particles?", bool(false));
  addParam("CompareWithMCParticleVerbose", m_compareMCParticleVerbose,
           "Compare reconstructed tracks with MC Particles?", bool(false));
  addParam("PXDExtrapolation", m_PXDExtrapolation,
           "Run PXD extrapolation?", bool(false));
  addParam("PXDTbExtrapolation", m_PXDTbExtrapolation,
           "Use testbeam extrapolation", bool(false));
  addParam("PrintTrackInfo", m_printTrackInfo,
           "Use testbeam extrapolation", bool(false));
  addParam("PrintStatistics", m_printStatistics,
           "Use testbeam extrapolation", bool(false));
  addParam("statisticsFilename", m_statisticsFileName,
           "File name for statistics generation", string(""));
  addParam("rootFilename", m_rootFilename,
           "File name for statistics generation", string(""));


  // 4. Noise Filter
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
  addParam("RadiusThreshold", m_radiusThreshold,
           "Cut off radius threshold", (double)(5.0));
  addParam("MergeThreshold", m_mergeThreshold,
           "Merge threshold", (double)(0.01));

}

void
SVDHoughtrackingModule::initialize()
{
  TDirectory* clusterDir, *houghDir, *effDir;
  StoreArray<SVDHoughCluster>::registerPersistent(m_storeHoughCluster, DataStore::c_Event, false);
  StoreArray<SVDHoughTrack>::registerPersistent(m_storeHoughTrack, DataStore::c_Event, false);
  StoreArray<SVDHoughCluster>::registerPersistent(m_storeExtrapolatedHitsName, DataStore::c_Event, false);

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
    m_histPTDist = new TH1D("pTDist", "pT-Distribution", 195, 0.05, 2.0);
    /* Histogram for correctly reconstructed tracks in pT */
    m_histPTPhirecon = new TH1D("pTPhirecon", "pT of reconstructed tracks in Phi", 195, 0.05, 2.0);
    /* Histogram for efficiency vs pT (only in phi while reconstruction in theta is not proved) */
    m_histPTEffPhi = new TH1D("pTEffPhi", "Efficiency vs pT (in Phi)", 195, 0.05, 2.0);
    /* Histogram for Phi-distribution */
    m_histPhiDist = new TH1D("PhiDist", "Phi-Distribution", 360, -180, 180);
    /* Histogram for correctly reconstructed tracks in Phi */
    m_histPhirecon = new TH1D("Phirecon", "Phi of reconstructed tracks", 360, -180, 180);
    /* Histogram for efficiency vs phi */
    m_histEffPhi = new TH1D("EffPhi", "Efficiency vs Phi", 360, -180, 180);
    /* Histogram for Theta-distribution */
    m_histThetaDist = new TH1D("ThetaDist", "Theta-Distribution", 133, 17, 150);    // Only to fill, if theta can be reconstructed
    /* Histogram for correctly reconstructed tracks in Theta */
    m_histThetarecon = new TH1D("Thetarecon", "Theta of reconstructed tracks", 133, 17, 150);
    /* Histogram for efficiency vs Theta */
    m_histEffTheta = new TH1D("EffTheta", "Efficiency vs Theta", 133, 17, 150);     // Only to fill, if theta can be reconstructed
    /* Histogram for fake rate vs pT (only in phi while reconstruction in theta is not proved) */
    m_histPTFakePhi = new TH1D("pTFake", "Fake rate vs pT (in Phi)", 200, 0.0, 2.0);
  }

  /* Write statistics file */
  if (!m_statisticsFileName.empty()) {
    statout.open(m_statisticsFileName, ofstream::out | ofstream::app);
  }

  B2INFO("SVDHoughtracking initilized");
}

void
SVDHoughtrackingModule::beginRun()
{
  /* Clear total clock cycles */
  totClockCycles = 0.0;

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
  if (m_printStatistics) {
    B2INFO("End of Run: Hough Statistics: ");
    B2INFO("  Total tracks found: " << totTracks);
    B2INFO("  Track total efficency: " << curTrackEff / (double) runNumber);
    B2INFO("  Track Theta efficency: " << curTrackEffN / (double) runNumber);
    B2INFO("  Track Phi efficency: " << curTrackEffP / (double) runNumber);
    B2INFO("  Fake Rate: " << totFakeTracks / (double) runNumber);
    B2INFO("  Total time: " << totClockCycles << " ms");
    B2INFO("  Average time: " << totClockCycles / (double) runNumber << " ms");
    B2INFO("  Min R: " << minR << " Max R: " << maxR);
  }

  /* Statistics output */
  if (!m_statisticsFileName.empty()) {
    if (m_useHashPurify) {
      statout << 1 << "\t" << m_critIterationsN << "\t" << m_critIterationsP << "\t"
              << (double) m_rectSizeN << "\t" << (double) m_rectSizeP << "\t"
              << curTrackEffP / (double) runNumber << "\t" << totFakeTracks / (double) runNumber << endl;
    } else {
      statout << 0 << "\t" << m_critIterationsN << "\t" << m_critIterationsP << "\t"
              << (double) m_rectSizeN << "\t" << (double) m_rectSizeP << "\t"
              << curTrackEffP / (double) runNumber << "\t" << totFakeTracks / (double) runNumber << endl;
    }

    /* Close Statistic output */
    statout.close();
  }

  /* Fill efficiency histograms as a division */
  if (!m_rootFilename.empty()) {
    *m_histPTEffPhi = (*m_histPTPhirecon) / (*m_histPTDist);
    m_histPTEffPhi->SetName("pTEffPhi");
    m_histPTEffPhi->SetTitle("Efficiency vs pT (in Phi)");
    *m_histEffPhi = (*m_histPhirecon) / (*m_histPhiDist);
    m_histEffPhi->SetName("EffPhi");
    m_histEffPhi->SetTitle("Efficiency vs Phi");
    *m_histEffTheta = (*m_histThetarecon) / (*m_histThetaDist);
    m_histEffTheta->SetName("EffTheta");
    m_histEffTheta->SetTitle("Efficiency vs theta");
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
      convertTrueHits();
    } else {
      clusterStrips();
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
  } else {
    int num_simhit = storeSVDSimHit.getEntries();
    if (m_histSimHits) {
      m_histSimHits->Fill(runNumber, num_simhit);
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

    /* Uncomment to stop here */
    return;

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
    for (inner_iter = cpy_map.begin(); inner_iter != cpy_map.end(); ++inner_iter) {
      TVector3 diff = iter->second - inner_iter->second;
      if (fabs(diff.Px()) < clusterDelta.Px() && fabs(diff.Py()) < clusterDelta.Py()
          && fabs(diff.Pz()) < clusterDelta.Pz()) {
        ++cnt;
        clus_pos += inner_iter->second;
        cluster_id = inner_iter->first;
        cpy_map.erase(inner_iter);
      } else {
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

  /* Start time measurement */
  start = clock();

  /* Hough transformation */
  houghTrafo2d(n_clusters, true, false); /* in z, no Hess transformation */
  houghTrafo2d(p_clusters, false, true); /* in y, with Hess */

  /*
   * Run hough trackign on P-Side
   */
  /* Initial rect size parameters for Hough trafo */
  rect_size = m_rectSizeP;
  scale_y = m_rectScaleP;
  /* Set start values */
  v1_s.Set((-1.0 * ((M_PI / 2)) - 0.0f), (scale_y * rect_size));
  v2_s.Set((M_PI / 2 - 0.0f), (scale_y * rect_size));
  v3_s.Set((M_PI / 2 - 0.0f), (-1.0 * scale_y * rect_size));
  v4_s.Set((-1.0 * ((M_PI / 2)) - 0.0f), (-1.0 * scale_y * rect_size));
  /* Run intercept finder */
  fastInterceptFinder2d(p_hough, false, v1_s, v2_s, v3_s, v4_s, 0, m_critIterationsP, m_maxIterationsP, p_rect);
  /* Debug */
  gplotRect("dbg/p_rect.plot", p_rect);

  /*
   * Run hough trackign on N-Side
   */
  /* Initial rect size parameters for Hough trafo */
  rect_size = m_rectSizeN;
  scale_y = m_rectScaleN;
  /* Set start values */
  v1_s.Set((-1.0 * ((M_PI / 2)) - 0.0f), (scale_y * rect_size));
  v2_s.Set((M_PI / 2 - 0.0f), (scale_y * rect_size));
  v3_s.Set((M_PI / 2 - 0.0f), (-1.0 * scale_y * rect_size));
  v4_s.Set((-1.0 * ((M_PI / 2)) - 0.0f), (-1.0 * scale_y * rect_size));
  /* Run intercept finder */
  fastInterceptFinder2d(n_hough, true, v1_s, v2_s, v3_s, v4_s, 0, m_critIterationsN, m_maxIterationsN, n_rect);
  /* Debug */
  gplotRect("dbg/n_rect.plot", n_rect);

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
  unsigned int tracks;
  TVector2 n_tc, p_tc;
  double r, phi, theta;
  bool all = true; /* combine every track in N and P */

  if (!storeHoughTrack.isValid()) {
    storeHoughTrack.create();
  } else {
    storeHoughTrack.getPtr()->Clear();
  }

  tracks = 0;
  //for (auto it = n_houghTrackCand.begin(); it != n_houghTrackCand.end(); ++it) {
  for (auto it_in = p_houghTrackCand.begin(); it_in != p_houghTrackCand.end(); ++it_in) {
    if (/*(it->getHash() == it_in->getHash() && it->getHitSize() == it_in->getHitSize()) ||*/ all) {
      ++tracks;
      //n_tc = it->getCoord();
      p_tc = it_in->getCoord();
      r = 1.0 / (2.0 * p_tc.Y());

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
        theta = n_tc.X(); // (M_PI) - n_tc.X();
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
  //}
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

  nTracks = storeHoughTrack.getEntries();

  if (m_printTrackInfo) {
    B2INFO("Found tracks: ");
  } else {
    B2DEBUG(1, "Found tracks: ");
  }
  for (int i = 0; i < nTracks; ++i) {
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
 * Take the hough tracks and to the extrapolation to the PXD
 * layer.
 */
void
SVDHoughtrackingModule::pxdExtrapolation()
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
 * analyer tool for the extrapolated hits and check for its performance
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
    B2DEBUG(200, "  Sensor: " << sensorID << " Side: " << n_side << " Strip: " << strip
            << " Charge: " << sample << " Position: " << pos);

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
  VxdID sensorID;
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
    VxdID sensorID = storeSVDSimHit[i]->getSensorID();
    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

    /* Convert local to global position */
    const VXD::SensorInfoBase& info = geo.get(sensorID);
    abs_pos = info.pointToGlobal(storeSVDSimHit[i]->getPosIn());
    pos = abs_pos;

    B2DEBUG(250, " Local sim Pos: " << vec.X() << " " << vec.Y() << " " << vec.Z()
            << " Abs pos: " << abs_pos.X() << " " << abs_pos.Y() << " " << abs_pos.Z() << " Sensor ID: " << sensorID);

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
      diff = pos - last_pos;
      dist = diff.Mag();
      B2DEBUG(250, "   Distance: " << dist);

      if (fabs(dist) < 0.01 && i != (nSimHit - 1)) {
      } else {
        n_clusters.insert(std::make_pair(n_idx, std::make_pair(sensorID, last_pos)));
        ++n_idx;
        p_clusters.insert(std::make_pair(p_idx, std::make_pair(sensorID, last_pos)));
        ++p_idx;
        storeHoughCluster.appendNew(SVDHoughCluster(sensorID, pos));
        ++cluster_cnt;
      }
    }

    last_pos = pos;
  }

  if (m_compareMCParticleVerbose) {
    B2INFO("  Total Clusters: " << cluster_cnt << " Number of Sim Hits: " << nSimHit);
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
  diff = 9E+99;

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
  double pitch;
  TVector3 pos, local_pos, first_pos;
  const SensorInfo* sensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));

  local_pos.SetX(0.0);
  local_pos.SetY(sensorInfo->getUCellPosition(0));
  local_pos.SetZ(sensorInfo->getUCellPosition(0));

  first_pos = sensorInfo->pointToGlobal(local_pos);

  pos.SetX(first_pos.X());
  if (n_side) {
    pitch = sensorInfo->getUPitch();
    pos.SetY(0.0);
    pos.SetZ(clusterPosition(pitch, seed_strip, size, (sensorInfo->getWidth() / 2.0)));
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
  SVDCluster* svdCluster;
  SVDTrueHit* svdTrueHit;

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
    B2INFO("Clusters N-Side: " << cluster.size());
  } else {
    B2INFO("Clusters P-Side: " << cluster.size());
  }

  for (auto it = cluster.begin(); it != cluster.end(); ++it) {
    clusterInfo = it->second;
    pos = clusterInfo.second;
    sensorID = clusterInfo.first;
    B2INFO("  " << pos.X() << " "  << pos.Y() << " " <<  pos.Z());
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
  bool useTheta = false;
  bool* track_match_n;
  bool* track_match_p;
  bool* track_match;
  double charge, phi, theta, r;
  double theta_tolerance = 1.5;
  double phi_tolerance = 1.5;
  double min, min_theta, min_phi, dist, pT;
  double nPrimary, nCorrReco, nCorrRecoN, nCorrRecoP;
  double m_r = 0.0;
  double frac = 360.0;
  TVector3 mom;

  nMCParticles = storeMCParticles.getEntries();
  nTracks = storeHoughTrack.getEntries();

  /* Don't analyse high occ events */
  if (nTracks > 10000) {
    B2WARNING("High occupancy event. Skipping...");
    return;
  }

  track_match_n = new bool[nMCParticles]();
  track_match_p = new bool[nMCParticles]();
  track_match = new bool[nMCParticles]();
  nPrimary = 0;

  if (m_compareMCParticleVerbose) {
    B2INFO("MCParticle Comparison: ");
  } else {
    B2DEBUG(1, "MCParticle Comparison: ");
  }
  for (int i = 0; i < nMCParticles; ++i) {
    charge = storeMCParticles[i]->getCharge();
    mom = storeMCParticles[i]->getMomentum();
    if (storeMCParticles[i]->getMother() == NULL || !primeOnly) {
      /* Get pT of particles */
      pT = mom.Perp();
      /* Write pT in pT-distribution-histogram */
      m_histPTDist->Fill(pT);
      /* Write Phi in Phi-distribution-histogram */
      m_histPhiDist->Fill(mom.Phi() / Unit::deg);

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

        if (fabs((theta - (mom.Theta() * sin(mom.Phi())))) < fabs(min_theta)) {
          min_theta = ((theta - (mom.Theta() * sin(mom.Phi()))));
          //if (fabs((storeHoughTrack[j]->getTrackTheta() - (mom.Theta()))) < fabs(min_theta)) {
          //  min_theta = ((storeHoughTrack[j]->getTrackTheta() - (mom.Theta())));
        }

        B2DEBUG(150, "  MCP " << i << " with Track (" << phi << ", "
                << theta << ") " << j << " Difference: phi = "
                << min_phi << " theta = " << min_theta);
      }
      min_theta /= Unit::deg;
      min_phi /= Unit::deg;

      /* Plot spread in histogram */
      if (nTracks != 0) {
        m_histHoughDiffPhi->Fill(min_phi);
        m_histHoughDiffTheta->Fill(min_theta);
      }
      /* Found this one */
      if (min < (M_PI / 180.0)) {
        track_match[i] = true;
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
          m_histPTPhirecon->Fill(pT);
          m_histPhirecon->Fill(mom.Phi() / Unit::deg); /* Efficiency vs Phi */

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
          track_match_n[i] = true;
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
  }
  B2DEBUG(1, "   Fake tracks: " << totFakeTracks << " " << nTracks << " " << nCorrRecoP);
  ++run;

  delete track_match;
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
    //  cluster_map.insert(std::make_pair((iter->first) + 1 + i, ghost[i]));
  }

}

/* vim: set tabstop=2:softtabstop=2:shiftwidth=2:noexpandtab */

