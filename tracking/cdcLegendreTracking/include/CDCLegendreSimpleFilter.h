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

#include <fstream>
#include <vector>
#include <list>
#include <map>

namespace Belle2 {
  namespace TrackFinderCDCLegendre {

    class TrackCandidate;
    class TrackHit;
//  class CDCLegendreFilterCandidate;

    class SimpleFilter {

    public:


      /**
       * Constructor
       */
      SimpleFilter() {};

      ~SimpleFilter() {};

      /**
       * Returns probability of hit assignment to track. TODO: more robust criteria should be implemented
       */
      double getAssigmentProbability(TrackHit* hit, TrackCandidate* track);

      /**
       * Hits reassignment;
       */
      void processTracks(std::list<TrackCandidate*>& trackList);

      /**
       * Append unused hits to tracks
       */
      void appenUnusedHits(std::list<TrackCandidate*>& trackList, std::vector<TrackHit*> AxialHitList);

//    void trackCore();


    private:

//    std::list<CDCLegendreFilterCandidate*> m_cdcLegendreFilterCandidateList;
//    std::list<CDCLegendreTrackCandidate*>& m_trackList;
//    std::list<CDCLegendreTrackCandidate*> m_trackCoreList;
      const double m_distFactor = 0.5; /**< Factor used in assignment probability estimation */
//    const double __attribute__((unused)) m_minProb = 0.98; /**< Minimal probability of hit assignment; currently no used*/

    };
  }
}
