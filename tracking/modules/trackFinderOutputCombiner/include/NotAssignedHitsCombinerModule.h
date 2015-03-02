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
    std::string m_param_notAssignedTracksFromLocalFinder;         /**< TrackCandidates store array name from the local track finder. */
    std::string m_param_resultTrackCands;                         /**< TrackCandidates collection name from the combined results of the two recognition algorithm. The CDCHits are assumed to come from m_param_cdcHitsColName1. */
    std::string m_param_cdcHits;                                  /**< Name of the store array containing the hits. */

    double m_param_minimal_chi2;
    double m_param_minimal_chi2_stereo;

    double m_param_maximum_momentum_z;
    double m_param_maximum_distance_z;
    double m_param_minimal_theta_difference;
    double m_param_minimal_z_difference;

    Eigen::MatrixXf m_fittingMatrix;
    Eigen::MatrixXf m_zMatrix;
    Eigen::MatrixXf m_zDistMatrix;

    void fillHitsInto(const TrackFindingCDC::CDCRecoSegment2D& recoSegment, genfit::TrackCand* bestTrackCand);

    double calculateGoodFitIndex(const genfit::TrackCand& trackCandidate, const TrackFindingCDC::CDCRecoSegment2D segment, const StoreArray<CDCHit>& cdcHits);
    void resetEntry(unsigned int counterOuter, unsigned int counterInner) {
      m_fittingMatrix(counterOuter, counterInner) = 0;
      m_zMatrix(counterOuter, counterInner) = 0;
      m_zDistMatrix(counterOuter, counterInner) = 0;
    }

    void calculateFittingMatrices(const StoreArray<CDCHit>& cdcHits, const StoreArray<genfit::TrackCand>& resultTrackCands, const std::vector<TrackFindingCDC::CDCRecoSegment2D>& recoSegments);
    bool isGoodEntry(unsigned int counterSegments, int counterTracks);
    double calculateThetaOfTrackCandidate(genfit::TrackCand* trackCandidate, const StoreArray<CDCHit>& cdcHits);
  };
}
