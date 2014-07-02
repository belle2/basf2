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

using namespace std;
#include <fstream>
#include <vector>
#include <list>
#include <map>

namespace Belle2 {

  class CDCLegendreTrackCandidate;
  class CDCLegendreTrackHit;
//  class CDCLegendreFilterCandidate;

  class CDCLegendreSimpleFilter {

  public:


    /**
     * Constructor
     */
    CDCLegendreSimpleFilter() {};

    ~CDCLegendreSimpleFilter() {};

    /**
     * Returns probability of hit assignment to track. TODO: more robust criteria should be implemented
     */
    double getAssigmentProbability(CDCLegendreTrackHit* hit, CDCLegendreTrackCandidate* track);

    /**
     * Hits reassignment;
     */
    void processTracks(std::list<CDCLegendreTrackCandidate*>& trackList);

    /**
     * Append unused hits to tracks
     */
    void appenUnusedHits(std::list<CDCLegendreTrackCandidate*>& trackList, std::vector<CDCLegendreTrackHit*> AxialHitList);

//    void trackCore();


  private:

//    std::list<CDCLegendreFilterCandidate*> m_cdcLegendreFilterCandidateList;
//    std::list<CDCLegendreTrackCandidate*>& m_trackList;
//    std::list<CDCLegendreTrackCandidate*> m_trackCoreList;
    const double m_distFactor = 0.5; /**< Factor used in assignment probability estimation */
    const double m_minProb = 0.98; /**< Minimal probability of hit assignment; currently no used*/

  };

}
