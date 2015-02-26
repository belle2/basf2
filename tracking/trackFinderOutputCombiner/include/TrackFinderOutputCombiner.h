/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <genfit/TrackCand.h>
#include <string>
#include <vector>

namespace Belle2 {
  class CDCHit;

  namespace TrackFindingCDC {
    class TrackHit;
  }

  class TrackFinderOutputCombiner {

  public:
    typedef std::pair<int, double> BestFitInformation;
    typedef std::pair<std::vector<int>, BestFitInformation> HitSegment;

    /**
     * The local track finder outputs tracks with hits from another CDCHitsStoreArray than the legendre track finder. This function uses the relations between the two CDCHitsStoreArrays
     * to reset the local track cands to the same CDCHitsStoreArray indices.
     * @param trackCands the StoreArray to reset
     * @param notAssignedCDCHits the StoreArray with the hits the tracks use in the moment
     * @param m_param_cdcHits the name of the new CDCHitsStroreArray
     */
    static void resetLocalTrackCandsToCorrectCDCHits(StoreArray<genfit::TrackCand>& trackCands, const StoreArray<CDCHit>& notAssignedCDCHits, std::string m_param_cdcHits);

    /**
     * Calculates the Chi2 for a given set of hits. These hits are assumed to be axial only. In the moment the methods from CDCRiemannFitter are used.
     * @param hits must be axial only.
     * @return not the chi2 but the probability for a good fit using TMath::Prob
     */
    static double calculateChi2(std::vector<Belle2::TrackFindingCDC::TrackHit*>& hits);

    /**
     * Used the hits already stored in the TrackCand to get the parameters from a cirlce fit. These parameters and the already stored z-information is than stored in the TrackCand.
     * In the moment the methods from TrackFitter are used.
     * @param newResultTrack must already have hits and z information.
     * @param cdcHits StoreArray with the hits.
     * @return the probability for a goodness of the fit.
     */
    static double constructTrackCandidate(genfit::TrackCand* newResultTrack, const StoreArray<CDCHit>& cdcHits);

    /**
     * Splits the tracks coming from the local track finder according to their distance in the x-y-plane.
     * This method does not use any z information (because this information is maybe incorrect or not present at all). So the distance calculation may be not correct for stereo-axial pairs.
     * @param localTrackCands StoreArray with the local track candidates.
     * @param cdcHits their CDC hits
     * @param m_param_minimumDistanceBetweenHits the distance after which a distance is assumed to be too large and the track is splitted.
     * @return a vector with hit segments as pairs. Each hit segment (=pair) consists of a vector with the CDCHitIDs of this segment and the fit information.
     * The fit information is a pair also consisting of the ID in the legendre StoreArray for the best "partner" and the probability to fit to this partner.
     * This method only sets empty values for the fit information.
     */
    static std::vector<HitSegment> collectHitSegmentsOfLocalTracks(const StoreArray<genfit::TrackCand>& localTrackCands, const StoreArray<CDCHit>& cdcHits, int m_param_minimumDistanceBetweenHits);


    /**
     * Helper to calculate the wire position for a given cdcHit.
     * @param cdcHit
     * @return a TVector2 with the wire position.
     */
    static TVector2 calculateWirePosition(const CDCHit* cdcHit);
  };
}
