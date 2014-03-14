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

  class CDCLegendreTrackCreator {
  public:
    CDCLegendreTrackCreator(std::list<CDCLegendreTrackCandidate*>& trackList, CDCLegendreTrackFitter* cdcLegendreTrackFitter);

    /**
     * @brief Function to create a track candidate
     * @param track construction of std::pairs, describing the track candidate by the axial hits, belonging to it and the parameter r and theta
     * @param trackHitList list of all track hits, which are used for track finding. Hits belonging to the track candidate will be deleted from it.
     */
    void createLegendreTrackCandidate(const std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track, std::set<CDCLegendreTrackHit*>* trackHitList, std::pair<double, double>& ref_point);


    /**
     * @brief Perform the necessary operations after the track candidate has been constructed
     * @param track The constructed track candidate
     * @param trackHitList list of all hits, which are used for track finding. Hits belonging to the track candidate will be deleted from it.
     * This function leaves room for other operations like further quality checks or even the actual fitting of the track candidate.
     */
    void processTrack(CDCLegendreTrackCandidate* track, std::set<CDCLegendreTrackHit*>* trackHitList);

    /**
     * @brief Implementation of check for quality criteria after the track candidate was produced.
     */
    bool fullfillsQualityCriteria(CDCLegendreTrackCandidate* cand);

  private:
    CDCLegendreTrackFitter* m_cdcLegendreTrackFitter;
    std::list<CDCLegendreTrackCandidate*>& m_trackList; /**< List of track candidates. Mainly used for memory management! */


  };
}

