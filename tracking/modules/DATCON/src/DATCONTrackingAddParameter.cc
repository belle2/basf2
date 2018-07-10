/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONTrackingModule.h>

using namespace std;
using namespace Belle2;

void DATCONTrackingModule::addParameter()
{
// Define module parameters

  // 1. Collections
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticle collection name", string(""));
  addParam("SVDSpacePoints", m_storeSVDSpacePointsName,
           "Name of the SVDSpacePoints StoreArray.", string(""));
  addParam("DATCONSVDDigits", m_storeDATCONSVDDigitsName,
           "Name of the DATCONSVDDigits StoreArray", string("DATCONSVDDigits"));
  addParam("DATCONSSVDCluster", m_storeDATCONSVDClusterName,
           "Name of the DATCONSVDCluster StoreArray.", string("DATCONSVDCluster"));
  addParam("DATCONSVDSpacePoints", m_storeDATCONSVDSpacePointsName,
           "Name of the DATCONSVDSpacePoints StoreArray.", string("DATCONSVDSpacePoints"));
  addParam("DATCONTracks", m_storeDATCONTracksName,
           "DATCONTracks Collection", string(""));
  addParam("DATCONRecoTracks", m_storeDATCONRecoTracksName,
           "Name of the RecoTracks StoreArray.", string(""));

  // 2. Use DATCONSpacePoints or SVDSpacePoints?
  addParam("UseDATCONSVDSpacePoints", m_useDATCONSVDSpacePoints,
           "Use DATCONSVDSpacePoints for the DATCON", bool(true));
  addParam("UseSVDSpacePoints", m_useSVDSpacePoints,
           "Use SVDSpacePoints for the DATCON", bool(false));

  // 3. Hough Trafo Parameter
  addParam("trackCenterX", m_trackCenterX,
           "x coordinate of the track center (for conformal transformation)", double(0.0));
  addParam("trackCenterY", m_trackCenterY,
           "y coordinate of the track center (for conformal transformation)", double(0.0));
  addParam("minimumLines", m_minimumLines,
           "Minimum lines to be found in order to continue", (unsigned short)(3));
  addParam("maxIterationsU", m_maxIterationsU,
           "Critical iterations ones a cell becomes a track", int(6));
  addParam("maxIterationsV", m_maxIterationsV,
           "Critical iterations ones a cell becomes a track", int(5));
  addParam("independentSectors", m_independentSectors,
           "Independent number of sectors for x and y in HS (FPGA-like)", bool(false));
  addParam("nAngleSectorsU", m_nAngleSectorsU,
           "Number of sectors for HT in U", int(128));
  addParam("nAngleSectorsV", m_nAngleSectorsV,
           "Number of sectors for HT in V", int(64));
  addParam("nVertSectorsU", m_nVertSectorsU,
           "Number of sectors for HT in U", int(128));
  addParam("nVertSectorsV", m_nVertSectorsV,
           "Number of sectors for HT in V", int(64));
  addParam("rectSizeU", m_rectSizeU,
           "Size of rectangular in U", (double)(0.1));
  addParam("rectSizeV", m_rectSizeV,
           "Size of rectangular in V", (double)(2.0));

  // 3.1 What to do in Simulations for Phase 2:
  addParam("usePhase2Simulation", m_usePhase2Simulation,
           "Use Phase 2 Simulation setup and writ out strips for testing", bool(false));

  // 4. Extracting Information from the Hough Space
  // 4.1 Use Purifier
  addParam("UsePurifier", m_usePurifier,
           "Bool variable to decide whether to use a purifying algorithm at all", bool(true));

  // 4.2 Use Hough Space Clustering
  addParam("UseHoughSpaceClustering", m_useHoughSpaceClustering,
           "Use clustering of the HS instead of purifier algorithms", bool(false));
  addParam("MinimumPhiHSClusterSize", m_MinimumPhiHSClusterSize,
           "Minimum size of cluster in HS", int(1));
  addParam("MaximumPhiHSClusterSize", m_MaximumPhiHSClusterSize,
           "Maximum size of cluster in HS", int(100));
  addParam("MaximumPhiHSClusterSizeX", m_MaximumPhiHSClusterSizeX,
           "Maximum size of cluster in HS in angular direction", int(100));
  addParam("MaximumPhiHSClusterSizeY", m_MaximumPhiHSClusterSizeY,
           "Maximum size of cluster in HS in vertical direction", int(100));
  addParam("MinimumThetaHSClusterSize", m_MinimumThetaHSClusterSize,
           "Minimum size of cluster in HS", int(1));
  addParam("MaximumThetaHSClusterSize", m_MaximumThetaHSClusterSize,
           "Maximum size of cluster in HS", int(100));
  addParam("MaximumThetaHSClusterSizeX", m_MaximumThetaHSClusterSizeX,
           "Maximum size of cluster in HS in angular direction", int(100));
  addParam("MaximumThetaHSClusterSizeY", m_MaximumThetaHSClusterSizeY,
           "Maximum size of cluster in HS in vertical direction", int(100));

  // 5. Merge TrackCandidates or Tracks?
  addParam("UseTrackCandMerger", m_useTrackCandMerger,
           "Use the track merger", bool(true));
  addParam("UseTrackCandMergerU", m_useTrackCandMergerU,
           "Use the track merger for u-side", bool(true));
  addParam("UseTrackCandMergerV", m_useTrackCandMergerV,
           "Use the track merger for v-side", bool(true));
  addParam("MergeThreshold", m_mergeThreshold,
           "Merge threshold", (double)(0.01));
  addParam("MergeThresholdU", m_mergeThresholdU,
           "Merge threshold", (double)(0.01));
  addParam("MergeThresholdV", m_mergeThresholdV,
           "Merge threshold", (double)(0.01));
  addParam("UseTrackMerger", m_useTrackMerger,
           "Use the track merger", bool(true));
  addParam("MergeThresholdPhi", m_mergeThresholdPhi,
           "Merge threshold for phi", (double)(0.01));
  addParam("MergeThresholdTheta", m_mergeThresholdTheta,
           "Merge threshold for theta", (double)(0.01));

}
