/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <TVector3.h>

namespace Belle2 {
  /**
   * Module for checking SpacePointTrackCandidates for curling behaviour and splitting them into Track Candidate Stubs (each of them being a SpacePointTrackCand again) which do not show curling behaviour.
   *
   * NOTE: If talking of a 'TrackCand' (in this documentation or in the code) it is assumed to be a SpacePointTrackCand if not stated explicitly otherwise
   *
   * Some statements on how the module works on checking if a SpacePointTrackCand is curling:
   * 1) convert genfit::TrackCand to SpacePointTrackCand
   * 2) for every SpacePoint in SpacePointTrackCand get Cluster(s) and from them get the according TrueHit(s). If there is more than one TrueHit for a SpacePoint (e.g. SVD) check if they are the same, if not throw
   * 3) From TrueHit get position and momentum of hit (in global coordinates) and decide with this information if the particles direction of flight is inwards our outwards (i.e. towards or away from set origin)
   * 4) If Direction changes from one SpacePoint to another -> split SpacePointTrackCand
   *
   */
  class CurlingTrackCandSplitterModule : public Module {

  public:

    CurlingTrackCandSplitterModule(); /**< Constructor */

    virtual void initialize(); /**< initialize: initialize counters, register stuff in DataStore, check if all necessary StoreArrays are present, etc. */

    virtual void event(); /**< event: check SpacePointTrackCand for curling behaviour, split if needed (and wanted by user) */

    virtual void terminate(); /**< terminate: print some summary on the modules work */

  protected:

    bool m_PARAMsplitCurlers; /**< indicating if the SpacePointTrackCands should only be analyzed for curling behaviour, or analyzed and split into TrackCand Stubs */

    std::string m_PARAMsptcName; /**< collection name of the SpacePointTrackCands to be analyzed */

    std::string m_PARAMcurlingOutFirstName; /**< collection name of the first outgoing (i.e. coming from the interaction point) parts of a curling TrackCand */

    std::string m_PARAMcurlingAllInName; /**< collection name of all ingoing parts of a curling TrackCand */

    std::string m_PARAMcurlingOutRestName; /**< collection name of all but the first outgoing parts of a curling TrackCand */

    std::string m_PARAMcompleteCurlerName; /**< collection name of all parts of a curling TrackCand */

    std::vector<double> m_PARAMsetOrigin; /**< set the origin to a specific point. Needed for determining the direction of flight of the particle for a given hit */

    int m_PARAMnTrackStubs; /**< maximum number of TrackCand Stubs to be stored for a curling TrackCand */

    bool m_saveCompleteCurler; /**< set to true if all parts of a curling TrackCand should be stored in a separate StoreArray (no parameter!) */

    TVector3 m_origin; /**< origin used internally (set from user set value) */

    int m_spacePointTCCtr; /**< Counter for presented SpacePointTrackCands */

    int m_curlingTCCtr; /**< Counter for TrackCands that show curling behaviour */

    int m_createdTrackStubsCtr; /**< Counter for created TrackCand Stubs by splitting a SpacePointTrackCand */

    int m_noDecisionPossibleCtr; /**< Counter for TrackCands where a decision if curling or not is not possible */

    void initializeCounters(); /**< initialize all counters to 0 for avoiding undeterministic behaviour. */

    /**
     *Check if the track candidate is curling. Returns the indices of SpacePoint where the Track Candidate changes direction (i.e. changes its direction of flight from outwards to inwards or vice versa)
     */
    const std::vector<int> checkTrackCandForCurling(const Belle2::SpacePointTrackCand&);

    /**
     * Get the global position and momentum for a given TrueHit (PXD or SVD at the moment). .first is position, .second is momentum
     */
    template<class TrueHit>
    std::pair<const TVector3, const TVector3> getGlobalPositionAndMomentum(TrueHit* aTrueHit);

    /**
     * Split a culring track candidate into (up to NTracklets) tracklets
     */
    const std::vector<Belle2::SpacePointTrackCand>
    splitCurlingTrackCand(const Belle2::SpacePointTrackCand& SPTrackCand, int NTracklets, const std::vector<int>& splitIndices);

    /**
     * determine the direction of flight of a particle for a given hit and the origin (assumed interaction point). True is outwards, false is inwards */
    bool getDirectionOfFlight(const std::pair<const TVector3, const TVector3>& hitPosAndMom, const TVector3 origin);

    /**
     * Exception for case when no TrueHit can be found for a Cluster
     */
    BELLE2_DEFINE_EXCEPTION(FoundNoTrueHit, "Found no TrueHit for one (or more) Cluster(s). Without a TrueHit a decision if a TrackCand is curling is not possible");

    /**
     * Exception for case when TrueHits for a given SpacePoint do not match (possible for e.g. SVD SpacePoint consisting of two SVDClusters)
     */
    BELLE2_DEFINE_EXCEPTION(TrueHitsNotMatching, "The TrueHits related to the two SVDClusters of a SpacePoint are not the same!")

    /**
     * Exception thrown, when no relation to a Cluster can be found for a SpacePoint. This should never happen, since only SpacePoints related from a Cluster get into the SpacePointTrackCand in the first place.
     */
    BELLE2_DEFINE_EXCEPTION(FoundNoCluster, "No related Cluster to a SpacePoint was found.");
  };
}
