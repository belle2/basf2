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
#include <framework/datastore/StoreArray.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <string>
#include <utility>


namespace Belle2 {
  /**
   * Module for converting genfit::TrackCands to SpacePointTrackCands
   *
   * NOTE: requires:
   * + a StoreArray with PXD only SpacePoints
   * + a StoreArray with SVD SpacePoints, where only SpacePoints with a U & a V Cluster are present
   * + a StoreArray with SVD SpacePoints, where every Cluster has its own SpacePoint
   *
   * Intended Behaviour: The Module takes a genfit::TrackCand and converts it to a SpacePointTrackCand by taking the TrackCandHits of the TrackCand and searching the DataStore for the according Clusters and then look for Relations of these Clusters to SpacePoints (without loosing any information of the genfit::TrackCand, such that every hit in the genfit::TrackCand can be found in a SpacePoint in the SpacePointTrackCand). Employing a somewhat sloppy notation below regarding Clusters and TrackCandHits, since they are essentially the same for this Module.
   *
   * NOTE: There are cases where the conversion cannot be done properly, in this case the genfit::TrackCand is skipped and will not be converted!
   * Problems only occur, when a Cluster is not unambiguously related to a SpacePoint (i.e. a Cluster is related to more than one SpacePoint), because:
   * + the decision which SpacePoint to use is not trivial in most cases. If no well defined decision can be made, the TrackCand will be skipped (see below how this decision is made, and when it is considered 'well defined')
   * + If no decision can be made and every SpacePoint would be added, problems would only be postponed to the conversion of SpacePointTrackCand to genfit::TrackCand, where it is even harder to decide what is right and what is wrong.
   *
   * Some statements on how the module works for a given (SVD) Cluster (TrackCandHit)
   * 1) Check if the Cluster (TrackCandHit) has already been used (i.e. it is already contained in the SpacePointTrackCand via a SpacePoint). If not used
   * 2) Get all SpacePoints (double Cluster) related to this Cluster
   * 2.a) If no SpacePoints (double Cluster) can be found, add this Cluster via a single Cluster SpacePoint (should always be present, if not an exception is thrown, and the genfit::TrackCand is not converted). Mark this Cluster (TrackCandHit) as used and go to next Cluster (TrackCandHit)
   * 3) For every SpacePoint (double Cluster) related to the original Cluster, get all Clusters related from this SpacePoint
   * 4) For every SpacePoint (from 3) check if its Cluster combination is valid (i.e. both Clusters are contained in the genfit::TrackCand and neither of these Clusters has been used by another SpacePoint) and also check if its Cluster combination is not valid, but existing (i.e. both Clusters are contained in the genfit::TrackCand but one has already been used by another SpacePoint)
   * 5) If ONLY ONE SpacePoint has a valid Cluster Combination -> take this SpacePoint, mark the Clusters (TrackCandHits) as used, proceed with next TrackCandHit
   * 6) If more than SpacePoint has a valid Cluster Combination -> take the SpacePoint with the smallest position difference in the genfit::TrackCand ('normally' there is a Cluster Combination, that is in consecutive order in the genfit::TrackCand) and mark its Clusters as used, proceed with next TrackCandHit
   * 7) If no valid SpacePoint can be found, but SpacePoints with existing (but used) Cluster Combinations can be found, an exception is thrown and the genfit::TrackCand will not be converted, since the conversion would get ambiguous then (this happens in roughly 1 % of all cases)
   * 8) If no valid SpacePoint can be found and no SpacePoint with existing (but used) Cluster Combinations, this Cluster will be added via a single Cluster SpacePoint (and then be marked as used)
   *
   * If no SpacePoint can be found for any Cluster in the genfit::TrackCand, an exception is thrown, and the conversion is skipped.
   */
  class GFTC2SPTCConverterModule : public Module {

  public:

    GFTC2SPTCConverterModule(); /**< Constructor*/

    virtual void initialize(); /**< initialize module (e.g. check if all required StoreArrays are present or registering new StoreArrays) */

    virtual void event(); /**< event: convert genfit::TrackCands to SpacePointTrackCands */

    virtual void terminate(); /**< terminate: print some summary information on the processed events */

    BELLE2_DEFINE_EXCEPTION(UnsuitableGFTrackCand, "The genfit::TrackCand cannot be unambiguously converted to a SpacePointTrackCand.");

  protected:

    std::string m_PXDClusterName; /**< PXDCluster collection name */

    std::string m_SVDClusterName; /**< SVDCluster collection name */

    std::string m_SingleClusterSVDSPName; /**< Single Cluster SVD SpacePoints collection name */

    std::string m_NoSingleClusterSVDSPName; /**< Non SingleCluster SVD SpacePoints collection name */

    std::string m_PXDClusterSPName; /**< PXDCluster SpacePoints collection name */

    std::string m_genfitTCName; /**< Name of collection of genfit::TrackCand StoreArray */

    std::string m_SPTCName; /**< Name of collection under which SpacePointTrackCands will be stored in the StoreArray */

    // some counters for testing
    unsigned int m_SpacePointTCCtr; /**< Counter for SpacePointTrackCands which were actually created by module*/

    unsigned int m_genfitTCCtr; /**< Counter for genfit::TrackCands which were presented to the module */

// #ifndef __CINT__ // was once needed, when it was defined in SpacePointTrackCand.h
    template<typename HitType> using HitInfo = std::pair<unsigned int, const HitType*>;
// #endif

    template<typename T> using fourTuple = boost::tuple<T, T, T, T>; /**< typdef, for some less writing effort in the code */

    template<typename T> using flaggedPair = boost::tuple<bool, T, T>; /**< typdef, for avoiding haveing a vector<bool> and a vector<pair<T,T>> */

    void initializeCounters(); /**< reset counters to 0 to avoid indeterministic behaviour */

    void markHitAsUsed(std::vector<flaggedPair<int> >& flaggedHitIDs, int hitToMark); /**< mark a hit as used, i.e. change its boolean value to true. Code readability reasons mainly, Output hardcoded to debug level 150 */

    const Belle2::SpacePointTrackCand createSpacePointTC(const genfit::TrackCand* genfitTC); /**< create a SpacePointTrackCand from the genfit::TrackCand */

    const Belle2::SpacePoint* getPXDSpacePoint(const PXDCluster* pxdCluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit); /**< get the SpacePoint related to the passed PXDCluster */

    const Belle2::SpacePoint* getSVDSpacePoint(const SVDCluster* svdCluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit); /**< get the appropriate SpacePoint of the passed SVD Cluster */

    const Belle2::SpacePoint* getSingleClusterSVDSpacePoint(const SVDCluster* svdCluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit); /**< get the single cluster SVD SpacePoint */

    BELLE2_DEFINE_EXCEPTION(UnusedHits, "Not all hits of the genfit::TrackCand have been marked as used. This indicates that not all hits have been used to create a SpacePointTrackCand.");

    BELLE2_DEFINE_EXCEPTION(FoundNoSpacePoint, "Found no relation between Cluster and SpacePoint. This hit would not be in SpacePointTrackCand, therefore skipping TrackCand from conversion!");
  };

}