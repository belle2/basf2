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
#include <list>
#include <set>
#include <cstdlib>
#include <vector>

namespace Belle2 {

  class CDCLegendreTrackHit;
  class CDCLegendreTrackCandidate;
  class CDCLegendreTrackFitter;
  class CDCLegendreFastHough;
  class CDCLegendreTrackCreator;


  /* TODO: Check whether track overlaps with other tracks; may be try to sort tracks according to number of inresections/overlappings, some weights might be applied
   * if track sharing more than, for example, 50% of hits, that track should be definitely splitted into few*/
  class CDCLegendreTrackMerger {
  public:

    CDCLegendreTrackMerger(std::list<CDCLegendreTrackCandidate*>& trackList, std::list<CDCLegendreTrackCandidate*>& trackletList, std::list<CDCLegendreTrackCandidate*>& stereoTrackletList, CDCLegendreTrackFitter* cdcLegendreTrackFitter, CDCLegendreFastHough* cdcLegendreFastHough, CDCLegendreTrackCreator* cdcLegendreTrackCreator);


    /**
     * The track finding often finds two curling tracks, originating from the same particle. This function merges them.
     */
    void MergeCurler();

    /**
     * Trying to merge tracks
     */
    double tryToMergeAndFit(CDCLegendreTrackCandidate* cand1, CDCLegendreTrackCandidate* cand2, bool remove_hits = true);

    /**
     * Trying select core of the tracks and fit them together, then add good hits
     */
    double selectCoreMergeFit(CDCLegendreTrackCandidate* cand1, CDCLegendreTrackCandidate* cand2);

    /**
     * Calculate distance between track and hit
     */
    double checkDist(CDCLegendreTrackHit* hit, CDCLegendreTrackCandidate* cand);

    /**
     * Split tracks into positive and negative parts
     */
    void splitTracks();

    /**
     * check whether tracks are overlapping;
     * for overlapping tracks try to merge them or make clear separation
     */
    void checkOverlapping();

    /**
     * @brief Function to merge two track candidates
     * All hits of track 2 are assigned to track 1 and the mean of the r and theta values of the two tracks are assigned to track 1
     * Track 2 is deleted.
     */
    bool earlyCandidateMerge(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track, std::set<CDCLegendreTrackHit*>& hits_set, bool fitTracksEarly);

    /**
     * @brief Function to merge two track candidates
     * All hits of track 2 are assigned to track 1 and the mean of the r and theta values of the two tracks are assigned to track 1
     * Track 2 is deleted.
     */
    void mergeTracks(CDCLegendreTrackCandidate* cand1, const std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track, std::set<CDCLegendreTrackHit*>& hits_set);

    /**
     * @brief Function to merge two track candidates
     * All hits of track 2 are assigned to track 1 and the mean of the r and theta values of the two tracks are assigned to track 1
     * Track 2 is deleted.
     */
    void mergeTracks(CDCLegendreTrackCandidate* cand1, CDCLegendreTrackCandidate* cand2, bool remove_hits = false);

    /**
     * Function which adds stereo tracklets to the track
     * Uses simple combinations of tracks
     * also theta angle determination implemented; for each tracklet which possibly can belong to the track theta angle are calculated, and then voting for better theta angle done
     */
    void addStereoTracklesToTrack();

    /**
     * Set stereowire z position
     * first, estimate z-position of wires, fine ajustment of z position using info about drift time
     *
     */
    double fitStereoTrackletsToTrack(CDCLegendreTrackCandidate*, CDCLegendreTrackCandidate*);

    /**
     * Extend tracklet using conformal transformation with respect to given point
     */
    void extendTracklet(CDCLegendreTrackCandidate* tracklet, std::vector<CDCLegendreTrackHit*>& m_AxialHitList);


  private:

    std::list<CDCLegendreTrackCandidate*>& m_trackList; /**< List of track candidates. Mainly used for memory management! */
    std::list<CDCLegendreTrackCandidate*>& __attribute__((unused)) m_trackletList; /**< List of tracklets. */
    std::list<CDCLegendreTrackCandidate*>& m_stereoTrackletList; /**< List of tracklets. */
    CDCLegendreTrackFitter* m_cdcLegendreTrackFitter; /**< Track fitter */
    CDCLegendreFastHough* m_cdcLegendreFastHough;  /**< Fast Hough finder */
    CDCLegendreTrackCreator* m_cdcLegendreTrackCreator; /**< Track creator */


  };

}
