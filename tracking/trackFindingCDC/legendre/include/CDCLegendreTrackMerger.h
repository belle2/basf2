/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackFitter.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>


#include <cdc/geometry/CDCGeometryPar.h>

#include <list>

namespace Belle2 {
  namespace TrackFindingCDC {

    class FastHough;
    class TrackProcessor;

    /* TODO: Check whether track overlaps with other tracks; may be try to sort tracks according to number of inresections/overlappings, some weights might be applied
     * if track sharing more than, for example, 50% of hits, that track should be definitely splitted into few*/
    class TrackMerger {
    public:

      /**
       * This class is to be used static only.
       */
      TrackMerger() = delete;


      /** The track finding often finds two curling tracks, originating from the same particle. This function merges them. */
      static void doTracksMerging(std::list<TrackCandidate*> trackList, TrackFitter* trackFitter);

      /** Try to merge given track with tracks in tracklist. */
      static void tryToMergeTrackWithOtherTracks(TrackCandidate* cand1, std::list<TrackCandidate*> trackList, TrackFitter* trackFitter);

      /** Trying select core of the tracks and fit them together, then add good hits
       * UNUSED */
      //double selectCoreMergeFit(TrackCandidate* cand1, TrackCandidate* cand2);

      /** Calculate distance between track and hit
       * UNUSED */
      //double checkDist(TrackHit* hit, TrackCandidate* cand);

      /** Split tracks into positive and negative parts
       * UNUSED */
      //void splitTracks();

      /** Check whether tracks are overlapping;
       * for overlapping tracks try to merge them or make clear separation
       * UNUSED
       */
      //void checkOverlapping();

      /**
       * @brief Function to merge two track candidates
       * All hits of track 2 are assigned to track 1 and the mean of the r and theta values of the two tracks are assigned to track 1
       * Track 2 is deleted.
       * UNUSED
       */
      //bool earlyCandidateMerge(std::pair<std::vector<TrackHit*>, std::pair<double, double> >& track, std::set<TrackHit*>& hits_set, bool fitTracksEarly);

      /**
       * @brief Function to merge two track candidates
       * All hits of track 2 are assigned to track 1 and the mean of the r and theta values of the two tracks are assigned to track 1
       * Track 2 is deleted.
       * UNUSED
       */
      //void mergeTracks(TrackCandidate* cand1, const std::pair<std::vector<TrackHit*>, std::pair<double, double> >& track, std::set<TrackHit*>& hits_set);

      /**
       * Function which adds stereo tracklets to the track
       * Uses simple combinations of tracks
       * also theta angle determination implemented; for each tracklet which possibly can belong to the track theta angle are
       * calculated, and then voting for better theta angle done
       * UNUSED
       */
      //void addStereoTracklesToTrack();

      /** Set stereowire z position
       * first, estimate z-position of wires, fine ajustment of z position using info about drift time
       * UNUSED
       */
      //double fitStereoTrackletsToTrack(TrackCandidate*, TrackCandidate*);

      /** Extend tracklet using conformal transformation with respect to given point
       * UNUSED */
      //void extendTracklet(TrackCandidate* tracklet, std::vector<TrackHit*>& m_AxialHitList);


    private:


      typedef unsigned int TrackCandidateIndex;
      typedef std::pair<TrackCandidateIndex, TrackCandidate*> TrackCandidateWithIndex;
      typedef double Probability;
      typedef std::pair<TrackCandidate*, Probability> BestMergePartner;

      /**
       * @brief Function to merge two track candidates
       */
      static void mergeTracks(TrackCandidate* cand1, TrackCandidate* cand2);

      /**
       * Marks hits away from the trajectory as bad. This method is used for calculating the chi2 of the tracks to be merged.
       * @param factor gives a number how far the hit is allowed to be.
       */
      static void removeStrangeHits(double factor, std::vector<TrackHit*>& trackHits, std::pair<double, double>& track_par, std::pair<double , double>& ref_point);

      /** Try to merge the two tracks
       * For this, build a common hit list and do a fast fit.
       * Then, throw away hits with a very high distance and fit again. Repeat this process three times.
       * As a result. the reduced chi2 is given.
       * The bad hits are marked but none of them is deleted!
       * This method does not do the actual merging. */
      static double doTracksFitTogether(TrackCandidate* cand1, TrackCandidate* cand2, TrackFitter* trackFitter);

      /**
       * Searches for the best candidate to merge this track to.
       * @param trackCandidateToBeMerged
       * @param start_iterator the iterator where to start searching (this element included)
       * @return a pointer to the best fit candidate.
       */
      static BestMergePartner calculateBestTrackToMerge(TrackCandidate* trackCandidateToBeMerged, std::list<TrackCandidate*>::iterator start_iterator, std::list<TrackCandidate*>::iterator end_iterator, TrackFitter* trackFitter);

      static void resetHits(TrackCandidate* otherTrackCandidate);

      //std::list<TrackCandidate*>& m_trackList; /**< List of track candidates. Mainly used for memory management! */
      //TrackFitter* m_cdcLegendreTrackFitter; /**< Track fitter */

      static constexpr double m_minimum_probability_to_be_merged = 0.8;

      //std::list<TrackCandidate*>& __attribute__((unused)) m_trackletList; /**< List of tracklets. */
      //std::list<TrackCandidate*>& __attribute__((unused)) m_stereoTrackletList; /**< List of tracklets. */
      //TrackProcessor* __attribute__((unused)) m_cdcLegendreTrackProcessor; /**< Track creator */
      //FastHough* __attribute__((unused)) m_cdcLegendreFastHough;  /**< Fast Hough finder */
    };
  }
}
