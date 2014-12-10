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

#include <TVector3.h>

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
   * 5) If ONLY ONE SpacePoint has a valid Cluster Combination, check if its Clusters appear in consecutive order in the genfit::TrackCand, if they do -> take this SpacePoint, mark the Clusters (TrackCandHits) as used, proceed with next TrackCandHit. If they do not appear in consecutive order -> throw an exception (this SpacePointTrackCand cannot be converted back to a genfit::TrackCand properly)
   * 6) If more than SpacePoint has a valid Cluster Combination -> check if there is a SpacePoint with consecutive Clusters in the genfit::TrackCand ('normally' there is a Cluster Combination, that is in consecutive order in the genfit::TrackCand) and mark its Clusters as used, proceed with next TrackCandHit, if all valid SpacePoints have Cluster Combinations that do not appear in consecutive order -> throw
   * 7) If no valid SpacePoint can be found, but SpacePoints with existing (but used) Cluster Combinations can be found, an exception is thrown and the genfit::TrackCand will not be converted, since the conversion would get ambiguous then (this happens in roughly 1 - 2 % of all cases)
   * 8) If no valid SpacePoint can be found and no SpacePoint with existing (but used) Cluster Combinations, this Cluster will be added via a single Cluster SpacePoint (and then be marked as used)
   *
   * If no SpacePoint can be found for any Cluster in the genfit::TrackCand, an exception is thrown, and the conversion is skipped.
   *
   * Some statements on how the module works on checking if a SpacePointTrackCand is curling:
   * 1) convert genfit::TrackCand to SpacePointTrackCand
   * 2) for every SpacePoint in SpacePointTrackCand get Cluster(s) and from them get the according TrueHit(s). If there is more than one TrueHit for a SpacePoint (e.g. SVD) check if they are the same, if not throw
   * 3) From TrueHit get position and momentum of hit (in global coordinates) and decide with this information if the particles direction of flight is inwards our outwards (i.e. towards or away from set origin)
   * 4) If Direction changes from one SpacePoint to another -> split SpacePointTrackCand
   *
   * NOTE: If the SpacePointTrackCand is checked for curling behaviour (set 'splitCurlers' to true), it is possible (at the moment) that the conversion is aborted, because of some further checks on the SpacePoints (and especially their relation to TrueHits). This abortion leads to another 1-2 % of failed conversions, although these SpacePointTrackCands could pass (there are some cases, where they would not) the current tests.
   *
   * TODO: clean-up and bring in line with coding conventions!
   */
  class GFTC2SPTCConverterModule : public Module {

  public:

    GFTC2SPTCConverterModule(); /**< Constructor*/

    virtual void initialize(); /**< initialize module (e.g. check if all required StoreArrays are present or registering new StoreArrays) */

    virtual void event(); /**< event: convert genfit::TrackCands to SpacePointTrackCands */

    virtual void terminate(); /**< terminate: print some summary information on the processed events */

    BELLE2_DEFINE_EXCEPTION(UnsuitableGFTrackCand, "The genfit::TrackCand cannot be unambiguously converted to a SpacePointTrackCand."); /**< Exception thrown, when a genfit::TrackCand occurs, that cannot be converted to a SpacePointTrackCand unambiguously (see Module documentation for more information) */

    BELLE2_DEFINE_EXCEPTION(FoundNoTrueHit, "Found no related TrueHit for one (or more) Clusters of the Track Candidate. Cannot check if this is a curling track!"); /**< Exception thrown, when no relation to a TrueHit can be found for a Cluster. Information from the TrueHit is needed for deciding if a track is curling or not */

    BELLE2_DEFINE_EXCEPTION(FoundNoCluster, "No related Cluster to a SpacePoint was found."); /**< Exception thrown, when no relation to a Cluster can be found for a SpacePoint. This should never happen, since only SpacePoints related from a Cluster get into the SpacePointTrackCand in the first place. */

  protected:

    std::string m_PXDClusterName; /**< PXDCluster collection name */

    std::string m_SVDClusterName; /**< SVDCluster collection name */

    std::string m_SingleClusterSVDSPName; /**< Single Cluster SVD SpacePoints collection name */

    std::string m_NoSingleClusterSVDSPName; /**< Non SingleCluster SVD SpacePoints collection name */

    std::string m_PXDClusterSPName; /**< PXDCluster SpacePoints collection name */

    std::string m_genfitTCName; /**< Name of collection of genfit::TrackCand StoreArray */

    std::string m_SPTCName; /**< Name of collection under which SpacePointTrackCands will be stored in the StoreArray */

    std::vector<std::string> m_PARAMCurlingTCNames; /**< Names of containers under which Track Stubs of curling track candidates get stored in the StoreArray (regard the order: 1. first (outgoing) part of a curling Track, 2. all ingoing parts of a curling Track, 3. all but the first outgoing part of a curling track, 4. all parts of a curling track (for testing)). */

    bool m_PARAMsplitCurlers; /**< Split curling tracks into tracklets.*/

    std::vector<double> m_PARAMsetOrigin; /**< Reset origin, usefull for e.g. testbeam et by user (or to default (0,0,0) if no other user input). WARNING: this does not have to be the actual interaction point!*/

    int m_NTracklets; /**< maximum number of tracklets to be saved, if curling tracks are split up into tracklets*/

    TVector3 m_origin; /**< Assumed interaction point. Defining this a separate parameter, in case it is reset during run-time to not change the user set value */

    bool m_saveCompleteCurler; /**< Indicator whether all parts of a curling TrackCandidate shall be stored in an extra store array, or if only parts of it get stored together */

    // some counters for testing
    unsigned int m_SpacePointTCCtr; /**< Counter for SpacePointTrackCands which were converted (if a curling track is split up, this counter will still be only increased by 1!) */

    unsigned int m_genfitTCCtr; /**< Counter for genfit::TrackCands which were presented to the module */

    unsigned int m_curlingTracksCtr; /**< Counter for tracks that show curling behaviour */

    unsigned int m_TrackletCtr; /**< Counter for all tracklets that were created by splitting up a curling track */

// #ifndef __CINT__ // was once needed, when it was defined in SpacePointTrackCand.h
    template<typename HitType> using HitInfo = std::pair<double, const HitType*>; /**< container used for storing information, that is then put into the SpacePointTrackCand */
// #endif

    template<typename T> using fourTuple = boost::tuple<T, T, T, T>; /**< typdef, for some less writing effort in the code */

    template<typename T> using flaggedPair = boost::tuple<bool, T, T>; /**< typdef, for avoiding haveing a vector<bool> and a vector<pair<T,T>> */

    void initializeCounters(); /**< reset counters to 0 to avoid indeterministic behaviour */

    void markHitAsUsed(std::vector<flaggedPair<int> >& flaggedHitIDs, int hitToMark); /**< mark a hit as used, i.e. change its boolean value to true. Code readability reasons mainly, Output hardcoded to debug level 150 */

    const Belle2::SpacePointTrackCand createSpacePointTC(const genfit::TrackCand* genfitTC); /**< create a SpacePointTrackCand from the genfit::TrackCand */

    const Belle2::SpacePoint* getPXDSpacePoint(const PXDCluster* pxdCluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit); /**< get the SpacePoint related to the passed PXDCluster */

    const Belle2::SpacePoint* getSVDSpacePoint(const SVDCluster* svdCluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit); /**< get the appropriate SpacePoint of the passed SVD Cluster */

    const Belle2::SpacePoint* getSingleClusterSVDSpacePoint(const SVDCluster* svdCluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit); /**< get the single cluster SVD SpacePoint */

    const std::vector<int> checkTrackCandForCurling(const Belle2::SpacePointTrackCand&); /**< Check if the track candidate is curling. Returns the indices of SpacePoint where the Track Candidate changes direction (i.e. changes its direction of flight from outwards to inwards or vice versa) */

    const std::vector<Belle2::SpacePointTrackCand> splitCurlingTrackCand(const Belle2::SpacePointTrackCand& SPTrackCand, int NTracklets, const std::vector<int>& splitIndices); /**< Split a culring track candidate into (up to NTracklets) tracklets */

    /**
     * Get the global position and momentum for a given TrueHit (PXD or SVD at the moment). .first is position, .second is momentum
     */
    template<class TrueHit>
    std::pair<const TVector3, const TVector3> getGlobalPositionAndMomentum(TrueHit* aTrueHit);

    bool getDirectionOfFlight(const std::pair<const TVector3, const TVector3>& hitPosAndMom, const TVector3 origin); /**< determine the direction of flight of a particle for a given hit and the origin (assumed interaction point). True is outwards, false is inwards */

    /**
     * Exception thrown, when not all hits of a genfit::TrackCand have been used for conversion.
     */
    BELLE2_DEFINE_EXCEPTION(UnusedHits, "Not all hits of the genfit::TrackCand have been marked as used. This indicates that not all hits have been used to create a SpacePointTrackCand.");

    /**
     * Exception thrown, when no SpacePoint that is related to a Cluster in the genfit::TrackCand can be found
     */
    BELLE2_DEFINE_EXCEPTION(FoundNoSpacePoint, "Found no relation between Cluster and SpacePoint. This hit would not be in SpacePointTrackCand, therefore skipping TrackCand from conversion!");

    BELLE2_DEFINE_EXCEPTION(TrueHitsNotMatching, "The TrueHits related to the two SVDClusters of a SpacePoint are not the same!") /**< Exception thrown during checking if Track is curling, when a SpacePoint is related to two different TrueHits (via its Clusters) */
  };

}