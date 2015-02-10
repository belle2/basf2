/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>
#include <utility>
#include <vector>

#include <cdc/dataobjects/CDCHit.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <genfit/TrackCand.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>


// Forward declarations
namespace genfit {
  class TrackCand;
}



namespace Belle2 {

  class NotAssignedHitsCombinerModule : public Module {

  public:

    /**
     * Constructor to set the module parameters.
     */
    NotAssignedHitsCombinerModule();

    /**
     * Empty destructor.
     */
    virtual ~NotAssignedHitsCombinerModule() {};

    virtual void initialize();

    /**
     * Empty begin run.
     */
    virtual void beginRun() {};

    /**
     * Combine the two track finder outputs.
     */
    virtual void event();

    /**
     * Empty end run.
     */
    virtual void endRun() {};

    /**
     * Empty terminate.
     */
    virtual void terminate() {};

  protected:

  private:

    std::string m_param_tracksFromLegendreFinder;                 /**< TrackCandidates store array name from the legendre track finder. */
    std::string m_param_notAssignedTracksFromLocalFinder;         /**< TrackCandidates store array name from the local track finder. */
    std::string m_param_resultTrackCands;                         /**< TrackCandidates collection name from the combined results of the two recognition algorithm. The CDCHits are assumed to come from m_param_cdcHitsColName1. */

    std::string m_param_cdcHits;                                  /**< Name of the store array containing the hits. */
    std::string m_param_notAssignedCDCHits;                       /**< Name of the store array containing the hits not used by the track finding algorithm. This store array is created by this module. */

    int m_param_minimumDistanceBetweenHits;                       /**< Minimum distance between two CDCHits to assume a break. */
    unsigned int m_param_minimumHitSizeForTrack;                  /**< Minimum track hits a track candidate from the local track finder needs to have to be treated independent. */

    bool m_param_reassignLocalTracks;                             /**< Reassign the hits of he local track candidates. */
    double m_param_minimal_chi2;

    /**
     * Tries to combine the newly found track candidates from the local track finder and the legendre track finder.
     * @param resultTrackCands StoreArray.
     * @param localTrackCands StoreArray.
     * @param legendreTrackCands StoreArray.
     * @param cdcHits StoreArray.
     */
    void matchNewlyFoundLocalTracksToLegendreTracks(StoreArray<genfit::TrackCand>& resultTrackCands, const StoreArray<genfit::TrackCand>& localTrackCands, const StoreArray<genfit::TrackCand>& legendreTrackCands, const StoreArray<CDCHit>& cdcHits);

    /**
     * Helper to calculate the angle distance between track and hit to not combine all tracks with all hit segments (probably too slow).
     * @param legendreTrackCand
     * @param firstCDCHitOfSegment
     * @return flag if it should by tried to combine these two.
     */
    bool isTooFarAway(const genfit::TrackCand& legendreTrackCand, CDCHit* firstCDCHitOfSegment);
  };
}
