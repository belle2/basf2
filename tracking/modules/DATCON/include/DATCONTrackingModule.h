/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/modules/DATCON/DATCONTrackCand.h>
#include <tracking/modules/DATCON/DATCONHoughCand.h>
#include <tracking/modules/DATCON/DATCONHoughSpaceClusterCand.h>

#include <tracking/dataobjects/DATCONSVDSpacePoint.h>
// #include <tracking/modules/DATCON/DATCONTrack.h>
#include <tracking/dataobjects/DATCONTrack.h>

#include <tracking/dataobjects/DATCONSVDDigit.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/modules/mcTrackCandClassifier/MCTrackCandClassifierModule.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <set>
#include <time.h>
#include <vector>
#include <unordered_map>
#include <type_traits>

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/dataobjects/RelationElement.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

#include <geometry/bfieldmap/BFieldMap.h>
#include <geometry/GeometryManager.h>

#include <mdst/dataobjects/MCParticle.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/geometry/SensorInfo.h>

#include <root/TCanvas.h>
#include <root/TEfficiency.h>
#include <root/TFile.h>
#include <root/TF1.h>
#include <root/TH1D.h>
#include <root/TH1F.h>
#include <root/TH2D.h>
#include <root/TGeoMatrix.h>
#include <root/TGraph.h>
#include <root/TMath.h>
#include <root/TRandom.h>
#include <root/TTree.h>
#include <root/TVector2.h>
#include <root/TVector3.h>
#include <TDatabasePDG.h>
#include <TGeoManager.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/reconstruction/Sample.h>
#include <svd/simulation/SVDSignal.h>

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>


namespace Belle2 {

  /** SVD Clusters */
  /** Pair containing VxdID and corresponding position vector */
  typedef std::pair<VxdID, TVector3> svdClusterPair;
  /** Map containing integer ID and svdClusterPair */
  typedef std::map<int, svdClusterPair> svdClusterMap;
  /** Iterator of svdClusterMap */
  typedef std::map<int, svdClusterPair>::iterator svdClusterMapIter;
  /** Hough Tuples */
  /** Pair containing VxdID ant TVector2 of TODO */
  typedef std::pair<VxdID, TVector2> houghPair;
  /** Map containing integer ID and corresponding houghPair for the HS TODO make this description better */
  typedef std::map<int, houghPair> houghMap;

  /** DATCONTrackingModule class description */
  class DATCONTrackingModule : public Module {
  public:
    /** Constructor.  */
    DATCONTrackingModule();

    /** Standard Deconstructor */
    virtual ~DATCONTrackingModule() = default;

    /** Initialize the module and check module parameters */
    virtual void initialize();
    /** Run tracking */
    virtual void event();
    /** Terminate the module */
    virtual void terminate();

    /** Add module parameter, put in separate function to have the Constructor clean and readable */
    void addParameter();

    /** Prepare the DATCONSVDSpacePoints for the Hough Trafo */
    void prepareDATCONSVDSpacePoints();

    /** Prepare the DATCONSVDSpacePoints for the Hough Trafo */
    void prepareSVDSpacePoints();

    /** Hough transformation */
    /** New Hough transformation of Christian Wessel with some major changes */
    void houghTrafo2d(svdClusterMap& mapClusters, bool u_side, bool conformal);


    /** Intercept Finder functions */
    /** fastInterceptFinder2d uses iterative / recursive approach, only subdividing "active"
     * sectors with at least m_minimumLines of m_minimumLines different detector layers
     * passing through the sector
     */
    /** New fastInterceptFinder2d written by Christian Wessel, up-to-date */
    int fastInterceptFinder2d(houghMap& hits, bool u_side, TVector2 v1_s,
                              TVector2 v2_s, TVector2 v3_s, TVector2 v4_s,
                              unsigned int iterations, unsigned int maxIterations);
    /** FPGA-like intercept finder with all the sectors given / "festgelegt" a priori
     * so no subdivision of sectors is needed, but thus this intercept finder is slower
     * since all sectors have to be checked and not only active ones
     */
    int slowInterceptFinder2d(houghMap& hits, bool u_side);

    /** TODO */
    void FindHoughSpaceCluster(bool u_side);

    /** TODO */
    void DepthFirstSearch(bool u_side, int** ArrayOfActiveHoughSpaceSectors, int angleSectors, int vertSectors,
                          int* initialPositionX, int* initialPositionY, int actualPositionX, int actualPositionY,
                          int* clusterCount, int* clusterSize, TVector2* CenterOfGravity, std::vector<unsigned int>& mergedList);

    /** Layer filter, checking for hits from different SVD layers */
    bool layerFilter(bool* layer);


    /** Functions to purify track candidates */
    /** Purify track candidates by checking list of strip_id
     * (specific id calculated in this module)
     */
    void purifyTrackCandsList();

    /** Compare strip_id lists / lists of counter variable */
    bool compareList(std::vector<unsigned int>& aList, std::vector<unsigned int>& bList);

    /** Merge strip_id lists / lists of counter variables to combine
     * large lists to smaller ones and thus purify track candidates
     * and avoid (too) many fakes
     */
    void mergeIdList(std::vector<unsigned int>& mergedList, std::vector<unsigned int>& mergeme);

    /** Track merger to merge DATCONTrackCand with similar properties
     * and to avoid / reduce number of fakes
     */
    void trackCandMerger();

    /** Track merger to merge DATCONTrackCand with similar properties
     * and to avoid / reduce number of fakes
     */
    void trackMerger();

    /** Find and Combine track candidates
     * Combine found values of phi and r from u-side HS
     * with found values of theta from v-side HS into track candidates
     */
    void fac3d();

    /** Save Hits to RecoTrack */
    void saveHitsToRecoTrack(std::vector<unsigned int>& idList, TVector3 momentum);


  protected:

    /** Members holding module parameters: */

    /** 1. Collections */
    /** Name of the collection for the MCParticles */
    std::string m_storeMCParticlesName;
    /** Name of the SVDSpacePoints array */
    std::string m_storeSVDSpacePointsName;
    /** Name of the DATCONSVDDigits array */
    std::string m_storeDATCONSVDDigitsName;
    /** Name of the DATCONSVDSpacePoints array */
    std::string m_storeDATCONSVDClusterName;
    /** Name of the DATCONSVDSpacePoints array */
    std::string m_storeDATCONSVDSpacePointsName;
    /** Name of the collection for the SVD Hough tracks */
    std::string m_storeDATCONTracksName;
    /** Name of the RecoTrack array */
    std::string m_storeDATCONRecoTracksName;
    /** Name of the RecoHit StoreArray required for RecoTracks */
    std::string m_storeRecoHitInformationName;

    // 2. Use DATCONSpacePoints or SVDSpacePoints?
    /** Use DATCONSVDSpacePoints */
    bool m_useDATCONSVDSpacePoints;
    /** Use SVDSpacePoints */
    bool m_useSVDSpacePoints;

    // 3. Hough Trafo Parameter
    /** Center of the tracks (our Hough Trafo requires (0, 0, 0).
     * If the IP is much different than that, it has to be given. */
    /** Center position in X */
    double m_trackCenterX;
    /** Center position in Y */
    double m_trackCenterY;
    /** Use conformal transformation with (x',y') = (x,y)/r^2 */
    bool m_xyHoughUside;
    /** Use conformal transformation with (r, phi0) */
    bool m_rphiHoughUside;
    /** Minimum number of lines required for an active sector (default: 3) */
    unsigned short m_minimumLines;
    /** Maximum number of iterations allowed for u-side (obsolete?) (default: 12) */
    int m_maxIterationsU;
    /** Maximum number of iterations allowed for v-side (obsolete?) (default: 12) */
    int m_maxIterationsV;
    /** Use tracking with independent sectors in HS like it is possible to do on FPGA
     * (with slowInterceptFinder2d)
     * Independent means: number of sectors is not necessary equal to 2^n with
     * n=1,2,3,... number of iteration steps and number of sectors for horizontal and
     * vertical axis in HS does not have to be equal but different
     * (default: false)
     */
    bool m_independentSectors;
    /** Using independent sectors: Number of horizontal sectors for u-side (default: 256) */
    int m_nAngleSectorsU;
    /** Using independent sectors: Number of horizontal sectors for v-side (default: 256) */
    int m_nAngleSectorsV;
    /** Using independent sectors: Number of vertical sectors for u-side (default: 256) */
    int m_nVertSectorsU;
    /** Using independent sectors: Number of vertical sectors for v-side (default: 256) */
    int m_nVertSectorsV;
    /** For fastInterceptFinder2d: vertical size of HS for v-side (default: 0.1) */
    double m_rectSizeV;
    /** For fastInterceptFinder2d: vertical size of HS for u-side (default: 0.1) */
    double m_rectSizeU;

    // 3.1 What to do in Simulations for Phase 2:
    bool m_usePhase2Simulation;


    // 4. Extracting Information from the Hough Space
    // 4.1 Use Purifier
    /** Use purifying algortihm at all */
    bool m_usePurifier;

    // 4.2 Use Hough Space Clustering
    bool m_useHoughSpaceClustering;
    int m_MinimumPhiHSClusterSize;
    int m_MaximumPhiHSClusterSize;
    int m_MaximumPhiHSClusterSizeX;
    int m_MaximumPhiHSClusterSizeY;
    int m_MinimumThetaHSClusterSize;
    int m_MaximumThetaHSClusterSize;
    int m_MaximumThetaHSClusterSizeX;
    int m_MaximumThetaHSClusterSizeY;

    // 5. Merge TrackCandidates or Tracks?/** Use TrackMerger to merge found tracks (candidates) to avoid / reduce fakes */
    bool m_useTrackCandMerger;
    /** Use TrackMerger for u-side tracks (candidates) */
    bool m_useTrackCandMergerU;
    /** Use TrackMerger for v-side tracks (candidates) */
    bool m_useTrackCandMergerV;
    /** Global track merger merging threshold */
    double m_mergeThreshold;
    /** Merging threshold for u-side trackCandMerger */
    double m_mergeThresholdU;
    /** Merging threshold for v-side trackCandMerger */
    double m_mergeThresholdV;
    /** Use TrackMerger to merge found tracks (candidates) to avoid / reduce fakes */
    bool m_useTrackMerger;
    /** Merging threshold for phi trackMerger */
    double m_mergeThresholdPhi;
    /** Merging threshold for theta trackMerger */
    double m_mergeThresholdTheta;

    bool m_combineAllTrackCands;

    StoreArray<MCParticle> storeMCParticles;
    StoreArray<RecoTrack> storeDATCONRecoTracks;
    StoreArray<SpacePoint> storeSVDSpacePoints;

    StoreArray<SVDCluster> storeDATCONSVDCluster;
    StoreArray<DATCONSVDSpacePoint> storeDATCONSVDSpacePoints;
    StoreArray<DATCONTrack> storeDATCONTracks;
//     StoreArray<DATCONHoughCluster> storeHoughCluster;
    StoreArray<DATCONSVDDigit> storeDATCONSVDDigits;

    StoreArray<RecoHitInformation> storeRecoHitInformation;

    /** Tracking */

    /** TODO */
    int m_minHoughClusters;
    /** TODO */
    double m_maxClusterSizeX;
    /** TODO */
    double m_maxClusterSizeY;

    /** 5. Hough Trafo Variables */

    /** Clusters */
    /** SVD u-side clusters */
    svdClusterMap uClusters;
    /** SVD v-side clusters */
    svdClusterMap vClusters;

    /** Hough Map */
    /** u-side HoughMap */
    houghMap uHough;
    /** v-side HoughMap */
    houghMap vHough;

    /** Hough Candidates */
    /** u-side */
    std::vector<DATCONHoughCand> uHoughCand;
    /** v-side */
    std::vector<DATCONHoughCand> vHoughCand;

    /** u-side */
    std::vector<DATCONHoughSpaceClusterCand> uHoughSpaceClusterCand;
    /** v-side */
    std::vector<DATCONHoughSpaceClusterCand> vHoughSpaceClusterCand;

    /** Purified Hough Candidates */
    /** u-side */
    std::vector<DATCONTrackCand> uTrackCand;
    /** v-side */
    std::vector<DATCONTrackCand> vTrackCand;

    /** DATCON Track */
    std::vector<DATCONTrack> DATCONTracks;

    std::vector<bool> activeSectorVectorPhi;
    std::vector<bool> activeSectorVectorTheta;

    int** ArrayOfActiveSectorsPhiHS;
    int** ArrayOfActiveSectorsThetaHS;

  };//end class declaration
} // end namespace Belle2
