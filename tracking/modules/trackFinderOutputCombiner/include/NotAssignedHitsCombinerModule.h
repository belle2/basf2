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

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <Eigen/Dense>
#include <tracking/trackFinderOutputCombiner/FittingMatrix.h>

// Forward declarations
namespace genfit {
  class TrackCand;
}



namespace Belle2 {

  class CDCHit;

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
  }

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
    std::string m_param_resultTrackCands;                         /**< TrackCandidates collection name from the combined results of the two recognition algorithm. The CDCHits are assumed to come from m_param_cdcHitsColName1. */
    std::string m_param_cdcHits;                                  /**< Name of the store array containing the hits. */
    std::string m_param_badTrackCands;                            /**< Name of the Store Array for the bad segments for testing. */
    std::string m_param_recoSegments;                             /**< Name of the Store Array for the segments from the local track finder. */

    FittingMatrix m_fittingMatrix;

    double calculateGoodFitIndex(const genfit::TrackCand& trackCandidate, const TrackFindingCDC::CDCRecoSegment2D segment, const StoreArray<CDCHit>& cdcHits);
    double calculateThetaOfTrackCandidate(genfit::TrackCand* trackCandidate, const StoreArray<CDCHit>& cdcHits);
    void findEasyCandidates(std::vector<TrackFindingCDC::CDCRecoSegment2D>& recoSegments, const StoreArray<genfit::TrackCand>& resultTrackCands, const StoreArray<genfit::TrackCand>& legendreTrackCands, const StoreArray<CDCHit>& cdcHits);
  };
}
