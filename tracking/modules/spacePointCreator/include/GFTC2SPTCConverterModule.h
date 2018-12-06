/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

# pragma once

#include <framework/core/Module.h>
// #include <framework/datastore/StoreArray.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <genfit/TrackCand.h>
#include <genfit/TrackCandHit.h>

#include <string>
#include <utility>
#include <boost/tuple/tuple.hpp>

namespace Belle2 {
  /**
   * Module for converting genfit::TrackCands to SpacePointTrackCands
   *
   * NOTE: requires:
   * + a StoreArray with PXD only SpacePoints
   * + a StoreArray with SVD SpacePoints, where only SpacePoints with a U & a V Cluster are present
   * + a StoreArray with SVD SpacePoints, where every Cluster has its own SpacePoint (if 'useSingleClusterSP' is set to true)
   *
   * Intended Behaviour: The Module takes a genfit::TrackCand and converts it to a SpacePointTrackCand by taking the TrackCandHits of the TrackCand and searching the DataStore for the according Clusters and then look for Relations of these Clusters to SpacePoints (without loosing any information of the genfit::TrackCand, such that every hit in the genfit::TrackCand can be found in a SpacePoint in the SpacePointTrackCand). Employing a somewhat sloppy notation below regarding Clusters and TrackCandHits, since they are essentially the same for this Module.
   *
   * NOTE: There are cases where the conversion cannot be done properly, in this case the genfit::TrackCand is skipped and will not be converted (by default, there are parameters that can change this behavior to only skip problematic Clusters)!
   * Problems only occur, when a Cluster is not unambiguously related to a SpacePoint (i.e. a Cluster is related to more than one SpacePoint), because:
   * + the decision which SpacePoint to use is not trivial in most cases. If no well defined decision can be made, the TrackCand will be skipped (see below how this decision is made, and when it is considered 'well defined')
   * + If no decision can be made and every SpacePoint would be added, problems would only be postponed to the conversion of SpacePointTrackCand to genfit::TrackCand, where it is even harder to decide what is right and what is wrong.
   * + To have a handle later on SpacePointTrackCands now have some flags to check what problems occured during conversion
   *
   * Some statements on how the module works for a given (SVD) Cluster (TrackCandHit)
   * 1) Check if the Cluster (TrackCandHit) has already been used (i.e. it is already contained in the SpacePointTrackCand via a SpacePoint). If not used
   * 2) Get all SpacePoints (double Cluster) related to this Cluster
   * 2.a) If no SpacePoints (double Cluster) can be found, add this Cluster via a single Cluster SpacePoint (should always be present, if not an exception is thrown, and the genfit::TrackCand is not converted). Mark this Cluster (TrackCandHit) as used and go to next Cluster (TrackCandHit)
   * 3) For every SpacePoint (double Cluster) related to the original Cluster, get all Clusters related from this SpacePoint
   * 4) For every SpacePoint (from 3) check if its Cluster combination is valid (i.e. both Clusters are contained in the genfit::TrackCand and neither of these Clusters has been used by another SpacePoint) and also check if its Cluster combination is not valid, but existing (i.e. both Clusters are contained in the genfit::TrackCand but one has already been used by another SpacePoint)
   * 5) If ONLY ONE SpacePoint has a valid Cluster Combination, check if its Clusters appear in consecutive order in the genfit::TrackCand, if they do -> take this SpacePoint, mark the Clusters (TrackCandHits) as used, proceed with next TrackCandHit. If they do not appear in consecutive order -> throw an exception (this SpacePointTrackCand cannot be converted back to a genfit::TrackCand properly)
   * 6) If more than SpacePoint has a valid Cluster Combination -> check if there is a SpacePoint with consecutive Clusters in the genfit::TrackCand ('normally' there is a Cluster Combination, that is in consecutive order in the genfit::TrackCand) and mark its Clusters as used, proceed with next TrackCandHit, if all valid SpacePoints have Cluster Combinations that do not appear in consecutive order -> throw
   * 7) If no valid SpacePoint can be found, but SpacePoints with existing (but used) Cluster Combinations can be found, an exception is thrown and the genfit::TrackCand will not be converted, since the conversion would get ambiguous then (this happens in roughly 1 - 2 % of all cases)
   * 8) If no valid SpacePoint can be found and no SpacePoint with existing (but used) Cluster Combinations, this Cluster will be added via a single Cluster SpacePoint (and then be marked as used) if this feature is activated by parameter 'useSingleClusterSP'
   *
   * If no SpacePoint can be found for any Cluster in the genfit::TrackCand, an exception is thrown, and the conversion is skipped.
   *
   * If 'checkTrueHits' is enabled the module only checks the DataStore if there is an existing relation to a TrueHit. Depending on the parameters the conversion of the genfit::TrackCand is aborted or only the Cluster is skipped from Conversion. (The registering of the relation has to be done with the SpacePoint2TrueHitConnector)
   *
   * TODO: clean-up and bring in line with coding conventions!
   */
  class GFTC2SPTCConverterModule : public Module {

  public:

    GFTC2SPTCConverterModule(); /**< Constructor*/

    void initialize()
    override; /**< initialize module (e.g. check if all required StoreArrays are present or registering new StoreArrays) */

    void event() override; /**< event: convert genfit::TrackCands to SpacePointTrackCands */

    void terminate() override; /**< terminate: print some summary information on the processed events */

  protected:

    /**
     * enum for differentiating different reasons why a conversion failed
     * negative values mean fail!
     */
    enum conversionStatus {
      c_singleClusterSP = 1, /**< had to use a singleCluster SpacePoint (also returned if PXD is passed! */
      c_noFail = 0, /**< conversion without any problems */
      c_foundNoSpacePoint = -1, /**< conversion failed because no related SpacePoint was found to a Cluster/Hit of the GFTC */
      c_foundNoTrueHit = -2, /**< conversion failed because there was no related SpacePoint to a TrueHit */
      c_unsuitableGFTC = -3, /**< conversion failed because the GFTC is considered not suitable for conversion */
      c_unusedHits = -4, /**< conversion failed because there were unused Hits */
      c_nonSingleSP = -5, /**< conversion failed because there were more than one single Cluster SpacePoints related to a Cluster */
      c_noValidSP = -6, /**< conversion failed because there was no valid SpacePoint (only possible for double Cluster SpacePoints) SpacePoints */
      c_lowNDF = -7, /**< conversion failed because the created SpacePointTrackCand had not enough degrees of freedom */
    };

    /** get the enum representation of an integer */
    conversionStatus getFailEnum(int intToConvert)
    {
      switch (intToConvert) {
        case 1: return c_singleClusterSP;
        case 0: return c_noFail;
        case -1: return c_foundNoSpacePoint;
        case -2: return c_foundNoTrueHit;
        case -3: return c_unsuitableGFTC;
        case -4: return c_unusedHits;
        case -5: return c_nonSingleSP;
        case -6: return c_noValidSP;
        case -7: return c_lowNDF;
        default: return c_foundNoSpacePoint;
      }
    }

    // ========================================================== PARAMETERS ============================================================================
    std::string m_PXDClusterName; /**< PXDCluster collection name */

    std::string m_SVDClusterName; /**< SVDCluster collection name */

    std::string m_SingleClusterSVDSPName; /**< Single Cluster SVD SpacePoints collection name */

    std::string m_NoSingleClusterSVDSPName; /**< Non SingleCluster SVD SpacePoints collection name */

    std::string m_PXDClusterSPName; /**< PXDCluster SpacePoints collection name */

    std::string m_genfitTCName; /**< Name of collection of genfit::TrackCand StoreArray */

    std::string m_SPTCName; /**< Name of collection under which SpacePointTrackCands will be stored in the StoreArray */

    bool m_PARAMcheckTrueHits; /**< Parameter Indicating if the TrueHits related from the Clusters forming a SpacePoint should be checked for equality */

    bool m_PARAMuseSingleClusterSP; /**< Parameter Indicating if SingleCluster SVD SpacePoints should be used if no double Cluster SVD SpacePoint can be found */

    bool m_PARAMcheckNoSingleSVDSP; /**< Switch for checking the StoreArray of non-single cluster SVD SpacePoints in initialize. Need this for cases, where only single Cluster SVD SpacePoints are presented to the module */

    bool m_PARAMskipCluster; /**< Switch for controlling the behavior of the converter, when for one or more Clusters no appropriate SpacePoint can be found */

    int m_PARAMminNDF; /**< parameter for specifying a minimal number of degrees of freedom a SpacePointTrackCand has to have in order to be registered in the DataStore */

    // ============================================================= COUNTER VARIABLES ====================================================================
    unsigned int
    m_SpacePointTCCtr; /**< Counter for SpacePointTrackCands which were converted (if a curling track is split up, this counter will still be only increased by 1!) */

    unsigned int m_genfitTCCtr; /**< Counter for genfit::TrackCands which were presented to the module */

    unsigned int m_abortedTrueHitCtr; /**< Counting discarded conversions due to check for TrueHits not good */

    unsigned int m_abortedUnsuitableTCCtr; /**< Counter for aborted conversions due to unsuitable genfit::TrackCand */

    unsigned int m_abortedNoSPCtr; /**< Counter for aborted conversions because no SpacePoint has been found */

    unsigned int
    m_noTwoClusterSPCtr; /**< Counter for cases where no related two Cluster could be found for a Cluster. NOTE: Counter counts cases where there really was no SpacePoint for a Cluster but does not count the cases where a found two Cluster SP was rejected later on in the process! */

    unsigned int
    m_abortedLowNDFCtr; /**< Counter for SpacePointTrackCands that were not stored due to a too small number of degrees of freedom */

    unsigned int m_skippedPXDnoSPCtr; /**< Counter for skipped PXD Clusters, due to no found SpacePoint */

    unsigned int m_skippedPXDnoTHCtr; /**< Counter for skipped PXD Clusters, due to no related TrueHit to a SpacePoint */

    unsigned int m_skippedSVDnoSPCtr; /**< Counter for skipped SVD Clusters, due to no found SpacePoint */

    unsigned int m_skippedSVDnoTHCtr; /**< Counter for skipped SVD Clusters, due to no related TrueHit to a SpacePoint */

    unsigned int m_singleClusterSPCtr; /**< Counter for single cluster SVD SpacePoints */

    unsigned int
    m_abortedMiscCtr; /**< temporary counter used for counting all failed conversions for which the reason cannot be deduced at the moment */

    unsigned int m_skippedCluster; /**< Counter for skipped Cluster */

    unsigned int
    m_skippedPXDunsuitableCtr; /**< Counter for skipped PXD Clusters due to unsuitable GFTC. NOTE: this can actually not happen */

    unsigned int m_skippedSVDunsuitableCtr; /**< Counter for skipped SVD Clusters due to unsuitable GFTC. */

    unsigned int
    m_skippedPXDnoValidSPCtr; /**< Counter for skipped PXD Clusters due to no found valid SpacePoint. NOTE: this can actually not happen */

    unsigned int m_skippedSVDnoValidSPCtr; /**< Counter for skipped SVD Clusters due to no found valid SpacePoint */

    unsigned int m_nonSingleSPCtr; /**< Counter for cases where there is more than one single Cluster SpacePoint related to a Cluster */

    unsigned int
    m_abortedNoValidSPCtr; /**< Counter for aborted conversions due to no found valid SpacePoint to any Cluster of the GFTC */

    /** reset counters to 0 to avoid indeterministic behaviour */
    void initializeCounters()
    {
      m_SpacePointTCCtr = 0;
      m_genfitTCCtr = 0;
      m_abortedTrueHitCtr = 0;
      m_abortedUnsuitableTCCtr = 0;
      m_abortedNoSPCtr = 0;
      m_abortedMiscCtr = 0;
      m_abortedLowNDFCtr = 0;
      m_abortedNoValidSPCtr = 0;

      m_noTwoClusterSPCtr = 0;
      m_nonSingleSPCtr = 0;

      m_skippedPXDnoSPCtr = 0;
      m_skippedPXDnoTHCtr = 0;
      m_skippedSVDnoSPCtr = 0;
      m_skippedSVDnoTHCtr = 0;
      m_singleClusterSPCtr = 0;

      m_skippedCluster = 0;
      m_skippedPXDunsuitableCtr = 0;
      m_skippedSVDunsuitableCtr = 0;

      m_skippedPXDnoValidSPCtr = 0;
      m_skippedSVDnoValidSPCtr = 0;

      m_NDF = 0; // (cppcheck complaining about not being initialized in constructor)
    }

    /** increase the counter that 'belongs' to the conversionStatus */
    void increaseFailCounter(conversionStatus status)
    {
      switch (status) {
        case c_foundNoSpacePoint: m_abortedNoSPCtr++; break;
        case c_foundNoTrueHit: m_abortedTrueHitCtr++; break;
        case c_unsuitableGFTC: m_abortedUnsuitableTCCtr++; break;
        case c_lowNDF: m_abortedLowNDFCtr++; break;
        case c_noValidSP: m_abortedNoValidSPCtr++; break;
        default: m_abortedMiscCtr++; break;
      }
      return;
    }

    /**
     * increase the appropriate counter variable if a Cluster is skipped (i.e. only called if 'skipClusters' is set to true!)
     * WARNING: decides which counter to increase (i.e. which ClusterType) via the layerNumber (hardcoded values!)
     */
    template<typename ClusterType>
    void increaseSkippedCounter(conversionStatus status, ClusterType* cluster)
    {
      short unsigned int layerNumber = cluster->getSensorID().getLayerNumber();
      if (layerNumber > 6) throw SpacePointTrackCand::UnsupportedDetType();
      switch (status) {
        case c_noFail:
          break;
        case c_singleClusterSP:
          break;
        case c_foundNoSpacePoint:
          if (layerNumber < 3) m_skippedPXDnoSPCtr++;
          else m_skippedSVDnoSPCtr++;
          break;
        case c_foundNoTrueHit:
          if (layerNumber < 3) m_skippedPXDnoTHCtr++;
          else m_skippedSVDnoTHCtr++;
          break;
        case c_unsuitableGFTC:
          if (layerNumber < 3) m_skippedPXDunsuitableCtr++;
          else m_skippedSVDunsuitableCtr++;
          break;
        case c_noValidSP:
          if (layerNumber < 3) m_skippedPXDnoValidSPCtr++;
          else m_skippedSVDnoValidSPCtr++;
          break;
        default:
          m_skippedCluster++; break;
      }
      return;
    }
    // ================================================================ INTERNALLY USED VARIABLES ===========================================================
    std::string svdMainArrayName; /**< Name of the Array of SVD SpacePoints that shall be searched first */

    int m_NDF; /**< number of degrees of freedom. counted during creation of SpacePointTrackCand */

    // ================================================= TYPEDEFS ============================================================================================================

// #ifndef __CINT__ // was once needed, when it was defined in SpacePointTrackCand.h
    template<typename HitType> using HitInfo =
      std::pair<double, const HitType*>; /**< container used for storing information, that is then put into the SpacePointTrackCand */
// #endif

    template<typename T> using flaggedPair =
      boost::tuple<bool, T, T>; /**< typdef, for avoiding having a vector<bool> and a vector<pair<T,T>> */

    // ============================================================================ MODULE METHODS ==============================================================================


    void markHitAsUsed(std::vector<flaggedPair<int> >& flaggedHitIDs,
                       int hitToMark); /**< mark a hit as used, i.e. change its boolean value to true. Code readability reasons mainly, Output hardcoded to debug level 150 */

    /**
     * create a SpacePointTrackCand from the genfit::TrackCand
     * @returns .first is the SPTC, .second is the conversion status, if < 0, something went wrong
     */
    std::pair<const Belle2::SpacePointTrackCand, conversionStatus>
    createSpacePointTC(const genfit::TrackCand* genfitTC, const StoreArray<PXDCluster>& pxdClusters,
                       const StoreArray<SVDCluster>& svdClusters);

    /**
     * process a TrackCandHit (i.e. do the handling of the different ClusterTypes), this is essentially nothing more than a wrapper, that directly returns whats returned from getSpacePoint(args) that is called within!
     */
    std::pair<Belle2::SpacePoint*, conversionStatus>
    processTrackCandHit(genfit::TrackCandHit* hit, const StoreArray<PXDCluster>& pxdClusters, const StoreArray<SVDCluster>& svdClusters,
                        std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit);

    /**
     * templated version to get a SpacePoint from a Cluster
     * @param arrayName name of the StoreArray to be searched, defaults to empty string
     * @param singleCluster singleCluster SpacePoint? (Can probably be done in another way as well)
     * NOTE: returns NULL if no (appropriate) SpacePoint can be found!
     * @returns .first is a pointer to the SpacePoint, .second is the status, if this is < 0, .first is NULL! (i.e. check .second first!)
     */
    template<typename ClusterType, typename TrueHitType>
    std::pair<Belle2::SpacePoint*, conversionStatus>
    getSpacePoint(const ClusterType* cluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit, bool singleCluster,
                  std::string arrayName = ""); /**< get the SpacePoint related to a Cluster */

    /**
     * given a RelationVector with SpacePoints in it, it tries to get the appropriate one (see main documentation of module)
     * NOTE: marks hits as used!
     * NOTE: if no appropriate SpacePoint can be found, returns a NULL pointer!
     * templated for easier handling of other ClusterTypes later, at the moment only SVDCluster needed!
     * @returns .first is a pointer to the appropriate SpacePoint, .second is the conversion status, if this is <0 .first is NULL! (i.e. check .second first)
     */
    template<typename ClusterType>
    std::pair<Belle2::SpacePoint*, conversionStatus>
    findAppropriateSpacePoint(const Belle2::RelationVector<Belle2::SpacePoint>& spacePoints,
                              std::vector<flaggedPair<int> >& flaggedHitIDs);

    /**
     * check if the Cluster (of a SpacePoint) is valid and/or exists in a genfit::TrackCand
     * @param clusterInd index of Cluster to be checked in StoreArray
     * @param detID detector id
     * @param flaggedHitIDs (detId, hitId) of genfit::TrackCand with flag if hit has already been used
     * @returns .first is validPos, .second is existingPos, if one is not found a negative number is returned!
     */
    std::pair<int, int> checkExistAndValid(int clusterInd, int detID, std::vector<flaggedPair<int> >& flaggedHitIDs);

    /**
     * get the position of the appropriate SpacePoint inside the RelationVector
     * NOTE: returns negative index if no SpacePoint fitting the criteria can be found!
     */
    int getAppropriateSpacePointIndex(const std::vector<std::pair<bool, bool> >& existAndValidSPs,
                                      const std::vector<std::pair<int, int> >& clusterPositions);

    /**
     * get the indices of the Clusters related to the SpacePoint. size of returned vector is <= 2!
     * NOTE: only the StoreArray of Clusters with name storeArrayName will be searched!
     */
    template<typename ClusterType>
    std::vector<int> getClusterIndices(const Belle2::SpacePoint* spacePoint, std::string storeArrayName);

    /**
     * get the NDF of a SpacePoint
     */
    int getNDF(Belle2::SpacePoint* spacePoint);

    /**
     * check if all hits have been used (i.e. if all flags in the vector of flaggedPairs is set to true)
     */
    bool checkUsedAllHits(std::vector<flaggedPair<int> >& flaggedHitIDs);

    /** check if there is a related TrueHit for a given SpacePoint. Possibility to pass an optional argument on the maximum number of allowed relations (defautls to 1) */
    template <typename TrueHitType>
    bool foundRelatedTrueHit(const Belle2::SpacePoint* spacePoint, unsigned int allowedRelations = 1);

    /**
     * Exception thrown, when not all hits of a genfit::TrackCand have been used for conversion.
     */
    BELLE2_DEFINE_EXCEPTION(UnusedHits,
                            "Not all hits of the genfit::TrackCand have been marked as used. This indicates that not all hits have been used to create a SpacePointTrackCand.");
  };
}
