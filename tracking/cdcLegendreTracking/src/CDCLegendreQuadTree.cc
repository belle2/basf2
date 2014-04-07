/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>

using namespace Belle2;

CDCLegendreQuadTree::CDCLegendreQuadTree(double rMin, double rMax, int thetaMin, int thetaMax, int level) :
  m_rMin(rMin), m_rMax(rMax), m_thetaMin(thetaMin), m_thetaMax(thetaMax), m_level(level)
{
  m_northWest = 0;
  m_northEast = 0;
  m_southWest = 0;
  m_southEast = 0;

}

void CDCLegendreQuadTree::insert(CDCLegendreTrackHit* hit)
{
  /*  // Ignore objects which do not belong in this quad tree
     if (!boundary.containsPoint(p))
       return false; // object cannot be added

     // If there is space in this quad tree, add the object here
     if (points.size < QT_NODE_CAPACITY)
     {
       points.append(p);
       return true;
     }

     // Otherwise, we need to subdivide then add the point to whichever node will accept it
     if (northWest == null)
       subdivide();

     if (northWest->insert(p)) return true;
     if (northEast->insert(p)) return true;
     if (southWest->insert(p)) return true;
     if (southEast->insert(p)) return true;

     // Otherwise, the point cannot be inserted for some unknown reason (which should never happen)
     return false;
  */
  m_hits.push_back(hit);
}
