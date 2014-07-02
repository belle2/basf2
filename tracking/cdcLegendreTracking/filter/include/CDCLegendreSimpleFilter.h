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
  class CDCLegendreFilterCandidate;

  class CDCLegendreSimpleFilter {

  public:
    CDCLegendreSimpleFilter();

    ~CDCLegendreSimpleFilter();

    double getAssigmentProbability(CDCLegendreTrackHit* hit, CDCLegendreTrackCandidate* track);

    void processTracks();

    void createFilterCandidateList(std::list<CDCLegendreTrackCandidate*> trackList);

    void appenUnusedHits(std::vector<CDCLegendreTrackHit*> AxialHitList);

    void trackCore();


  private:

    std::list<CDCLegendreFilterCandidate*> m_cdcLegendreFilterCandidateList;
    std::list<CDCLegendreTrackCandidate*> m_trackList;
//    std::list<CDCLegendreTrackCandidate*> m_trackCoreList;
    const double m_distFactor = 1.;
    const double m_minProb = 0.98;

  };

}
