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


  /* TODO: Check whether track overlaps with other tracks; may be try to sort tracks according to number of inresections/overlappings, some weights might be applied
   * if track sharing more than, for example, 50% of hits, that track should be definitely splitted into few*/
  class CDCLegendreTrackMerger {
  public:

    CDCLegendreTrackMerger(std::list<CDCLegendreTrackCandidate*>& trackList, CDCLegendreTrackFitter* cdcLegendreTrackFitter);


    /**
     * The track finding often finds two curling tracks, originating from the same particle. This function merges them.
     */
    void MergeCurler();

    /**
     * Trying to merge tracks
     */
    void MergeTracks();

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
    void mergeTracks(CDCLegendreTrackCandidate* cand1, CDCLegendreTrackCandidate* cand2);

  private:

    std::list<CDCLegendreTrackCandidate*>& m_trackList; /**< List of track candidates. Mainly used for memory management! */
    CDCLegendreTrackFitter* m_cdcLegendreTrackFitter;
  };

}
