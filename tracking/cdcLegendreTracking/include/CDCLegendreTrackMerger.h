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


#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <tracking/cdcLegendreTracking/CDCLegendreFastHough.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCreator.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>


#include <cdc/geometry/CDCGeometryPar.h>

#include <list>
#include <set>
#include <cstdlib>
#include <vector>
#include <cmath>

namespace Belle2 {
  namespace TrackFinderCDCLegendre {

    class FastHough;
    class TrackCreator;

    /* TODO: Check whether track overlaps with other tracks; may be try to sort tracks according to number of inresections/overlappings, some weights might be applied
     * if track sharing more than, for example, 50% of hits, that track should be definitely splitted into few*/
    class TrackMerger {
    public:

      TrackMerger(std::list<TrackCandidate*>& trackList, std::list<TrackCandidate*>& trackletList,
                  std::list<TrackCandidate*>& stereoTrackletList, TrackFitter* cdcLegendreTrackFitter,
                  FastHough* cdcLegendreFastHough, TrackCreator* cdcLegendreTrackCreator);


      /** The track finding often finds two curling tracks, originating from the same particle. This function merges them. */
      void doTracksMerging();

      /** Trying to merge tracks */
      double tryToMergeAndFit(TrackCandidate* cand1, TrackCandidate* cand2, bool remove_hits = true);

      /** Trying select core of the tracks and fit them together, then add good hits */
      double selectCoreMergeFit(TrackCandidate* cand1, TrackCandidate* cand2);

      /** Calculate distance between track and hit */
      double checkDist(TrackHit* hit, TrackCandidate* cand);

      /** Split tracks into positive and negative parts */
      void splitTracks();

      /** Check whether tracks are overlapping;
       * for overlapping tracks try to merge them or make clear separation
       */
      void checkOverlapping();

      /**
       * @brief Function to merge two track candidates
       * All hits of track 2 are assigned to track 1 and the mean of the r and theta values of the two tracks are assigned to track 1
       * Track 2 is deleted.
       */
      bool earlyCandidateMerge(std::pair<std::vector<TrackHit*>, std::pair<double, double> >& track, std::set<TrackHit*>& hits_set, bool fitTracksEarly);

      /**
       * @brief Function to merge two track candidates
       * All hits of track 2 are assigned to track 1 and the mean of the r and theta values of the two tracks are assigned to track 1
       * Track 2 is deleted.
       */
      void mergeTracks(TrackCandidate* cand1, const std::pair<std::vector<TrackHit*>, std::pair<double, double> >& track, std::set<TrackHit*>& hits_set);

      /**
       * @brief Function to merge two track candidates
       * All hits of track 2 are assigned to track 1 and the mean of the r and theta values of the two tracks are assigned to track 1
       * Track 2 is deleted.
       */
      void mergeTracks(TrackCandidate* cand1, TrackCandidate* cand2, bool remove_hits = false);

      /**
       * Function which adds stereo tracklets to the track
       * Uses simple combinations of tracks
       * also theta angle determination implemented; for each tracklet which possibly can belong to the track theta angle are
       * calculated, and then voting for better theta angle done
       */
      void addStereoTracklesToTrack();

      /** Set stereowire z position
       * first, estimate z-position of wires, fine ajustment of z position using info about drift time
       */
      double fitStereoTrackletsToTrack(TrackCandidate*, TrackCandidate*);

      /** Extend tracklet using conformal transformation with respect to given point */
      void extendTracklet(TrackCandidate* tracklet, std::vector<TrackHit*>& m_AxialHitList);


    private:

      std::list<TrackCandidate*>& m_trackList; /**< List of track candidates. Mainly used for memory management! */
      std::list<TrackCandidate*>& __attribute__((unused)) m_trackletList; /**< List of tracklets. */
      std::list<TrackCandidate*>& m_stereoTrackletList; /**< List of tracklets. */
      TrackFitter* m_cdcLegendreTrackFitter; /**< Track fitter */
      FastHough* m_cdcLegendreFastHough;  /**< Fast Hough finder */
      TrackCreator* m_cdcLegendreTrackCreator; /**< Track creator */


    };
  }
}
