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

#include <array>

#include <TVector3.h>
#include <TFile.h>
#include <TTree.h>

#include <array>

namespace Belle2 {
  /**
   * Module for checking SpacePointTrackCandidates for curling behaviour and splitting them into Track Candidate Stubs (each of them being a SpacePointTrackCand again) which do not show curling behaviour.
   *
   * NOTE: If talking of a 'TrackCand' (in this documentation or in the code) it is assumed to be a SpacePointTrackCand if not stated explicitly otherwise
   *
   * Some statements on how the module works on checking if a SpacePointTrackCand is curling:
   * 1) convert genfit::TrackCand to SpacePointTrackCand
   * 2) for every SpacePoint in SpacePointTrackCand get Cluster(s) and from them get the according TrueHit(s).
   * 4) Check if the TrueHits are compatible (explanation below)
   * 3) From TrueHit get position and momentum of hit (in global coordinates) and decide with this information if the particles direction of flight is inwards our outwards (i.e. towards or away from set origin)
   * 4) If Direction changes from one SpacePoint to another -> split SpacePointTrackCand
   *
   * Explanation of compatible TrueHits:
   * For all Clusters related from a SpacePoint all related TrueHits are collected. If there is one (or more) shared TrueHit this is considered to be compatible (regardless of other non-matching TrueHits)
   *
   */
  class CurlingTrackCandSplitterModule : public Module {

  public:

    CurlingTrackCandSplitterModule(); /**< Constructor */

    virtual void initialize(); /**< initialize: initialize counters, register stuff in DataStore, check if all necessary StoreArrays are present, etc. */

    virtual void event(); /**< event: check SpacePointTrackCand for curling behaviour, split if needed (and wanted by user) */

    virtual void terminate(); /**< terminate: print some summary on the modules work */

    /** Some constants for initialization */
    enum {
      c_nSVDPlanes = 4,
      c_nPXDPlanes = 2,

      c_nPlanes = c_nSVDPlanes + c_nPXDPlanes,
    };

    /** Internal DataStore for ROOT output variables */
    struct RootVariables {
      std::array<std::vector<double>, c_nPlanes> SpacePointXGlobal; /**< global x-positions of SpacePoints (layerwise) */
      std::array<std::vector<double>, c_nPlanes> SpacePointYGlobal; /**< global y-positions of SpacePoints (layerwise) */
      std::array<std::vector<double>, c_nPlanes> SpacePointZGlobal; /**< global z-positions of SpacePoints (layerwise) */

      std::array<std::vector<double>, c_nPlanes> SpacePointULocal; /**< local u-positions of SpacePoints (layerwise) */
      std::array<std::vector<double>, c_nPlanes> SpacePointVLocal; /**< local v-positions of SpacePoints (layerwise) */
//       std::array<std::vector<double>, c_nPlanes> SpacePointZLocal; /**< local z-positions of SpacePoints (layerwise)*/

      std::array<std::vector<double>, c_nPlanes> TrueHitXGlobal; /**< global x-positions of TrueHits (layerwise) */
      std::array<std::vector<double>, c_nPlanes> TrueHitYGlobal; /**< global y-positions of TrueHits (layerwise) */
      std::array<std::vector<double>, c_nPlanes> TrueHitZGlobal; /**< global z-positions of TrueHits (layerwise) */

      std::array<std::vector<double>, c_nPlanes> PosResidueXGlobal; /**< X-position (global) difference between TrueHit and SpacePoint (layerwise) */
      std::array<std::vector<double>, c_nPlanes> PosResidueYGlobal; /**< Y-position (global) difference between TrueHit and SpacePoint (layerwise) */
      std::array<std::vector<double>, c_nPlanes> PosResidueZGlobal; /**< Z-position (global) difference between TrueHit and SpacePoint (layerwise) */

      std::array<std::vector<double>, c_nPlanes> PosResidueULocal; /**< U-position (local) differnece between TrueHit and SpacePoint (layerwise) */
      std::array<std::vector<double>, c_nPlanes> PosResidueVLocal; /**< V-position (local) difference between TrueHit and SpacePoint (layerwise) */

      std::array<std::vector<double>, c_nPlanes> TrueHitULocal; /**< local u-positions of TrueHits (layerwise) */
      std::array<std::vector<double>, c_nPlanes> TrueHitVLocal; /**< local v-positions of TrueHits (layerwise) */
//       std::array<std::vector<double>, c_nPlanes> TrueHitZLocal; /**< local z-positions of TrueHits (layerwise) */

      std::array<std::vector<double>, c_nPlanes> PosResiduesLocal; /**< position differences in local coordinates (layerwise) */
      std::array<std::vector<double>, c_nPlanes> PosResiduesGlobal; /**< position differences in global coordinates (layerwise) */

      std::array<std::vector<double>, c_nPlanes> MisMatchPosResiduals; /**< Distance between TrueHits that do not match but are related from one SpacePoint (layerwise) */
      std::array<std::vector<double>, c_nPlanes> MisMatchPosX; /**< Difference of X-positions (global) for mismatched TrueHits (layerwise) */
      std::array<std::vector<double>, c_nPlanes> MisMatchPosY; /**< Difference of Y-positions (global) for mismatched TrueHits (layerwise) */
      std::array<std::vector<double>, c_nPlanes> MisMatchPosZ; /**< Difference of Z-positions (global) for mismatched TrueHits (layerwise) */

      std::array<std::vector<double>, c_nPlanes> MisMatchPosU; /**< Difference of U-positions (local) for mismatched TrueHits (layerwise) */
      std::array<std::vector<double>, c_nPlanes> MisMatchPosV; /**< Difference of V-positions (local) for mismatched TrueHits (layerwise) */

      std::array<std::vector<double>, c_nPlanes> MisMatchMomX; /**< Difference of Momentum in X-Direction for TrueHits that do not match but are related from one SpacePoint (layerwise) */
      std::array<std::vector<double>, c_nPlanes> MisMatchMomY; /**< Difference of Momentum in Y-Direction for TrueHits that do not match but are related from one SpacePoint (layerwise) */
      std::array<std::vector<double>, c_nPlanes> MisMatchMomZ; /**< Difference of Momentum in Z-Direction for TrueHits that do not match but are related from one SpacePoint (layerwise) */
    };

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
     *Check if the track candidate is curling. Returns the indices of SpacePoint where the Track Candidate changes direction (i.e. changes its direction of flight from outwards to inwards or vice versa). If the first entry of this vector is 0, this means, that the direction of flight for the first hit of this particle was towards the (assumed) interaction point!!
     */
    const std::vector<int> checkTrackCandForCurling(const Belle2::SpacePointTrackCand&, RootVariables& rootVariables);

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

    /** determine the direction of flight of a particle for a given hit and the origin (assumed interaction point). True is outwards, false is inwards */
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

    bool m_PARAMpositionAnalysis; /**< Set to true if output to ROOT file is desired with the positions and position differences of SpacePoints and TrueHits */
    std::vector<std::string> m_PARAMrootFileName; /**< two entries accepted. First is filename, second is 'RECREATE' or 'UPDATE' (write mode) */
    TFile* m_rootFilePtr; /**< Pointer to ROOT file */
    TTree* m_treePtr; /**< Pointer to ROOT tree */

    std::array<std::vector<double>, c_nPlanes> m_rootSpacePointXGlobals; /**< Global X-Positions of SpacePoints (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootSpacePointYGlobals; /**< Global Y-Positions of SpacePoints (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootSpacePointZGlobals; /**< Global Z-Positions of SpacePoints (layerwise) */

    std::array<std::vector<double>, c_nPlanes> m_rootSpacePointULocals; /**< Local U-Positions of SpacePoints (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootSpacePointVLocals; /**< Local V-Positions of SpacePoints (layerwise) */
//     std::array<std::vector<double>, c_nPlanes> m_rootSpacePointZLocals; /**< Local Z-Positions of SpacePoints (layerwise) */

    std::array<std::vector<double>, c_nPlanes> m_rootTrueHitXGlobals; /**< Global U-Positions of TrueHits (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootTrueHitYGlobals; /**< Global V-Positions of TrueHits (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootTrueHitZGlobals; /**< Global Z-Positions of TrueHits (layerwise) */

    std::array<std::vector<double>, c_nPlanes> m_rootTrueHitULocals; /**< Local X-Positions of TrueHits (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootTrueHitVLocals; /**< Local Y-Positions of TrueHits (layerwise) */
//     std::vector<double> m_rootTrueHitZLocals; /**< Local Z-Positions of TrueHits (layerwise) */

    std::array<std::vector<double>, c_nPlanes> m_rootPosResidueXGlobal; /**< X-position (global) difference between TrueHit and SpacePoint (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootPosResidueYGlobal; /**< Y-position (global) difference between TrueHit and SpacePoint (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootPosResidueZGlobal; /**< Z-position (global) difference between TrueHit and SpacePoint (layerwise) */

    std::array<std::vector<double>, c_nPlanes> m_rootPosResidueULocal; /**< U-position (local) differnece between TrueHit and SpacePoint (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootPosResidueVLocal; /**< V-position (local) difference between TrueHit and SpacePoint (layerwise) */

    std::array<std::vector<double>, c_nPlanes> m_rootLocalPosResiduals; /**< Local Position Residuals between TrueHits and SpacePoints (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootGlobalPosResiduals; /**< Global Position Residuals between TrueHits and SpacePoints (layerwise) */

    std::array<std::vector<double>, c_nPlanes> m_rootMisMatchPosDistance; /**< Distance of TrueHits that do not match but are related from one SpacePoint (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootMisMatchPosX; /**< Difference of X-positions (global) for mismatched TrueHits (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootMisMatchPosY; /**< Difference of Y-positions (global) for mismatched TrueHits (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootMisMatchPosZ; /**< Difference of Z-positions (global) for mismatched TrueHits (layerwise) */

    std::array<std::vector<double>, c_nPlanes> m_rootMisMatchPosU; /**< Difference of U-positions (local) for mismatched TrueHits (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootMisMatchPosV; /**< Difference of V-positions (local) for mismatched TrueHits (layerwise) */

    std::array<std::vector<double>, c_nPlanes> m_rootMisMatchMomX; /**< Difference of Momentum in X-Direction for TrueHits that do not match but are related from one SpacePoint (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootMisMatchMomY; /**< Difference of Momentum in Y-Direction for TrueHits that do not match but are related from one SpacePoint (layerwise) */
    std::array<std::vector<double>, c_nPlanes> m_rootMisMatchMomZ; /**< Difference of Momentum in Z-Direction for TrueHits that do not match but are related from one SpacePoint (layerwise) */

    std::pair<double, double> getUV(const Belle2::SpacePoint* spacePoint); /**< get U&V for a SpacePoint (via its relation to Clusters) (SpacePoint can only return normalized U&V coordinates) */

    template <class TrueHit>
    void getValuesForRoot(const Belle2::SpacePoint* spacePoint, const TrueHit* trueHit, RootVariables& rootVariables); /**< Get The Values that are later written to a ROOT file */

    void writeToRoot(RootVariables& rootVariables); /**< Write previously collected values to ROOT file */
  };
}
