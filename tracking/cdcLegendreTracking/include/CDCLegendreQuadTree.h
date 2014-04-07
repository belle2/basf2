/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pointers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 */


#pragma once

#include <vector>

namespace Belle2 {

  class CDCLegendreTrackHit;

  class CDCLegendreQuadTree {

  public:
    CDCLegendreQuadTree(double rMin, double rMax, int thetaMin, int thetaMax, int level, CDCLegendreQuadTree* parent);

    /*
     * Insert pointer on hit into node
     */
    void insert(CDCLegendreTrackHit* hit);

  private:

    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/

    double m_rMin;
    double m_rMax;
    int m_thetaMin;
    int m_thetaMax;
    int m_level;
    double Rcell;
    double m_deltaR;

    std::vector<CDCLegendreTrackHit*> m_hits;

    CDCLegendreQuadTree* m_parent;
    /*    CDCLegendreQuadTree* m_northWest;
        CDCLegendreQuadTree* m_northEast;
        CDCLegendreQuadTree* m_southWest;
        CDCLegendreQuadTree* m_southEast;
    */
    CDCLegendreQuadTree* m_children[2][2];
    bool m_lastLevel;

    /*
     * Check if we reach limitation on dr and dtheta; returns true when reached limit
     */
    bool checkLimitsR();

  };
}
