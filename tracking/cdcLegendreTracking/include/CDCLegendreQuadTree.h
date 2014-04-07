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

#include <vector>

namespace Belle2 {

  class CDCLegendreTrackHit;

  class CDCLegendreQuadTree {

  public:
    CDCLegendreQuadTree(double rMin, double rMax, int thetaMin, int thetaMax, int level);

    void insert(CDCLegendreTrackHit* hit);

  private:

    double m_rMin;
    double m_rMax;
    int m_thetaMin;
    int m_thetaMax;
    int m_level;

    std::vector<CDCLegendreTrackHit*> m_hits;

    CDCLegendreQuadTree* m_northWest;
    CDCLegendreQuadTree* m_northEast;
    CDCLegendreQuadTree* m_southWest;
    CDCLegendreQuadTree* m_southEast;
  };
}
