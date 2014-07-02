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
using namespace std;

namespace Belle2 {

  class CDCLegendreTrackCandidate;
  class CDCLegendreTrackHit;

  class CDCLegendreFilterCandidate {

  public:
    CDCLegendreFilterCandidate(CDCLegendreTrackCandidate* trackCandidate);

    ~CDCLegendreFilterCandidate();

    void addHit(CDCLegendreTrackHit* hit, double prob);

    void removeHit(CDCLegendreTrackHit* hit);

    inline double getTrackProbability() const {return m_trackProbability;};

    inline double getHitProbability(CDCLegendreTrackHit* hit) const {return m_trackHitsProbability.find(hit)->second;};

    void setHitProbability(CDCLegendreTrackHit* hit, double prob);

    CDCLegendreTrackCandidate* getLegendreCandidate() {return m_trackCandidate;};

    std::vector<CDCLegendreTrackHit*>& getHitsVector() {return m_trackHits;};

    std::map<CDCLegendreTrackHit*, double>& getHitsMap() {return m_trackHitsProbability;};

  private:

    CDCLegendreTrackCandidate* m_trackCandidate;
    std::map<CDCLegendreTrackHit*, double> m_trackHitsProbability;
    std::vector<CDCLegendreTrackHit*> m_trackHits;
    double m_trackProbability;

  };

}
