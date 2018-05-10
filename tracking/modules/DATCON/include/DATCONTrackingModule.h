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
#include <tracking/dataobjects/DATCONTrack.h>
#include <tracking/dataobjects/DATCONSVDDigit.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>

#include <root/TVector2.h>
#include <root/TVector3.h>

#include <svd/dataobjects/SVDCluster.h>

#include <vxd/dataobjects/VxdID.h>


namespace Belle2 {

  /** The DATCON Tracking Module performs track finding / pattern recognition
    * based on SVD hit information. The SVD hit information undergo a Hough
    * Transformation (HT) and based on the sinoidal curves obtained by this, interceptions
    * of these sinoidal curves are searched in two Hough Spaces (HS), one HS
    * to obtain (r-phi) and (z-theta) information each. The intercept coordinates
    * of the (r-phi) / (z-theta) combinations found are combined in clustering
    * to reduce the number of fake / clone combinations. Afterwards the remaining
    * 2 x 2D information are combined to 3D tracks, assuming the originating vertex
    * of these tracks to be at (x=0, y=0, z=0). These tracks can again be merged
    * and are finally stored as DATCONTracks and as RecoTracks.
    * Tracks found by this module are the input for the DATCONMPHCalculationModule
    * and afterwards for the DATCONROICalculationModule.
    */

  /** Useful typedefs for easier data access */
  /** Pair containing VxdID and corresponding position vector */
  typedef std::pair<VxdID, TVector3> svdHitPair;
  /** Map containing integer ID and svdHitPair */
  typedef std::map<int, svdHitPair> svdHitMap;
  /** Hough Tuples */
  /** Pair containing VxdID ant TVector2 of TODO */
  typedef std::pair<VxdID, TVector2> houghPair;
  /** Map containing integer ID and corresponding houghPair for the HS TODO make this description better */
  typedef std::map<int, houghPair> houghMap;

  /** DATCONTrackingModule class */
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

    /** Hough transformation function */
    void houghTrafo2d(svdHitMap& mapClusters, bool u_side);

    /** Intercept Finder functions */
    /** fastInterceptFinder2d uses iterative / recursive approach, only subdividing "active"
     * sectors with at least m_minimum lines of m_minimumLines different SVD layers
     * passing through the "active" sector, using a "Divide & Conquer" approach.
     */
    /** New fastInterceptFinder2d written by Christian Wessel, up-to-date */
    int fastInterceptFinder2d(houghMap& hits, bool u_side, TVector2 v1_s,
                              TVector2 v2_s, TVector2 v3_s, TVector2 v4_s,
                              unsigned int iterations, unsigned int maxIterations);
    /** FPGA-like intercept finder with all the sectors defined a priori,
     * so no subdivision of sectors is needed. This makes this intercept finder slower
     * since all sectors have to be checked and not only active ones
     */
    int slowInterceptFinder2d(houghMap& hits, bool u_side);

    /** Cluster finding in the Hough Space using a "depth first search" algorithm */
    void FindHoughSpaceCluster(bool u_side);

    /** Depth First Search algorithm, compare with
      * https://en.wikipedia.org/wiki/Depth-first_search (06. May 2018)
      * https://en.wikipedia.org/wiki/Connected_component_(graph_theory) (06. May 2018)
      */
    void DepthFirstSearch(bool u_side, int** ArrayOfActiveHoughSpaceSectors, int angleSectors, int vertSectors,
                          int* initialPositionX, int* initialPositionY, int actualPositionX, int actualPositionY,
                          int* clusterCount, int* clusterSize, TVector2* CenterOfGravity, std::vector<unsigned int>& mergedList);

    /** Layer filter, checking for hits from different SVD layers */
    bool layerFilter(bool* layer);

    /* Functions to purify track candidates */
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

    /** MCParticles StoreArray */
    StoreArray<MCParticle> storeMCParticles;
    /** SVDSpacePoints StoreArray */
    StoreArray<SpacePoint> storeSVDSpacePoints;

    /** DATCONSVDDigit StoreArray */
    StoreArray<DATCONSVDDigit> storeDATCONSVDDigits;
    /** DATCONSVDCluster StoreArray */
    StoreArray<SVDCluster> storeDATCONSVDCluster;
    /** DATCONSVDSpacePoint StoreArray */
    StoreArray<DATCONSVDSpacePoint> storeDATCONSVDSpacePoints;

    StoreArray<RecoHitInformation> storeRecoHitInformation;
    /** DATCONRecoTracks StoreArray */
    StoreArray<RecoTrack> storeDATCONRecoTracks;
    /** DATCONTracks StoreArray */
    StoreArray<DATCONTrack> storeDATCONTracks;

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
    /** Use the simulation for phase 2 geometry and FPGA setup? */
    bool m_usePhase2Simulation;

    // 4. Extracting Information from the Hough Space
    // 4.1 Use Purifier
    /** Use purifying algortihm at all */
    bool m_usePurifier;

    // 4.2 Use Hough Space Clustering
    /** Use FindHoughSpaceCluster algorithm to extract track information from the
      * Hough Spaces?
      */
    bool m_useHoughSpaceClustering;
    /** Minimum cluster size in the Phi HS */
    int m_MinimumPhiHSClusterSize;
    /** Maximum cluster size in the Phi HS */
    int m_MaximumPhiHSClusterSize;
    /** Maximum cluster size in horizontal direction in the Phi HS */
    int m_MaximumPhiHSClusterSizeX;
    /** Maximum cluster size in vertical direction in the Phi HS */
    int m_MaximumPhiHSClusterSizeY;
    /** Minimum cluster size of the Theta HS */
    int m_MinimumThetaHSClusterSize;
    /** Maximum cluster size in the Theta HS */
    int m_MaximumThetaHSClusterSize;
    /** Maximum cluster size in horizontal direction in the Theta HS */
    int m_MaximumThetaHSClusterSizeX;
    /** Maximum cluster size in vertical direction in the Theta HS */
    int m_MaximumThetaHSClusterSizeY;

    // 5. Merge TrackCandidates or Tracks?
    /** Use TrackMerger to merge found tracks (candidates) to avoid / reduce fakes */
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

    /** Clusters */
    /** SVD u-side clusters */
    svdHitMap uClusters;
    /** SVD v-side clusters */
    svdHitMap vClusters;

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

    /** Vector of bools containing information about the active
     * sectors of the Phi Hough Space in a 1-D representation
     */
    std::vector<bool> activeSectorVectorPhi;
    /** Analogue vector for the Theta Hough Space */
    std::vector<bool> activeSectorVectorTheta;

    /** "2D-Array" of the sectors of the Phi Hough Space
     * containing information whether or not a sector is
     * "active", meaning it is at crossed by at least three
     * rising curves of three different layers.
     */
    int** ArrayOfActiveSectorsPhiHS;
    /** Analogue array for the Theta Hough Space */
    int** ArrayOfActiveSectorsThetaHS;

  };//end class declaration
} // end namespace Belle2
