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
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include <cmath>

using namespace Belle2;

CDCLegendreQuadTree::CDCLegendreQuadTree(double rMin, double rMax, int thetaMin, int thetaMax, int level, CDCLegendreQuadTree* parent) :
  m_rMin(rMin), m_rMax(rMax), m_thetaMin(thetaMin), m_thetaMax(thetaMax), m_level(level)
{
  if (level > 0) {
    m_parent = parent;
  }
  /*  m_northWest = 0;
    m_northEast = 0;
    m_southWest = 0;
    m_southEast = 0;
  */
  Rcell = 2. * m_PI * 60. / 384.;

  m_deltaR = 4. * Rcell / (4. / ((m_rMax + m_rMin) * (m_rMax + m_rMin)) - 4.*Rcell * Rcell);

  m_lastLevel = checkLimitsR();

//  B2INFO("CREATED node:" << m_thetaMin<< "x" << m_thetaMax << "x" << m_rMin << "x" << m_rMax << " dr=" << delta_r << " level:" << m_level);

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

  /*
    double r_temp;
    if (hit->checkRValue(m_thetaMin)) r_temp = hit->getRValue(m_thetaMin);
    else {
      r_temp = hit->getConformalX() * m_cos_theta[m_thetaMin] +
               hit->getConformalY() * m_sin_theta[m_thetaMin];
      hit->setRValue(m_thetaMin, r_temp);
    }


    r_1 = r_temp + hit->getConformalDriftTime();
    r_2 = r_temp - hit->getConformalDriftTime();

    //calculate distances of lines to horizontal bin border
    for (int r_index = 0; r_index < 3; ++r_index) {
      dist_1[t_index][r_index] = r[r_index] - r_1;
      dist_2[t_index][r_index] = r[r_index] - r_2;
    }
  }

  //actual voting, based on the distances (test, if line is passing though the bin)
  for (int t_index = 0; t_index < 2; ++t_index)
  {
    for (int r_index = 0; r_index < 2; ++r_index) {
      //curves are assumed to be straight lines, might be a reasonable assumption locally
      if ((!sameSign(dist_1[t_index][r_index], dist_1[t_index][r_index + 1], dist_1[t_index + 1][r_index], dist_1[t_index + 1][r_index + 1])) &&
          (hit->isUsed() == CDCLegendreTrackHit::not_used))
        voted_hits[t_index][r_index].push_back(hit);
      else if ((!sameSign(dist_2[t_index][r_index], dist_2[t_index][r_index + 1], dist_2[t_index + 1][r_index], dist_2[t_index + 1][r_index + 1])) &&
               (hit->isUsed() == CDCLegendreTrackHit::not_used))
        voted_hits[t_index][r_index].push_back(hit);
    }
  }


  if (!m_lastLevel)
  {
    for (int ii = 0; ii < 2; ii++) {
      for (int jj = 0; jj < 2; jj++) {
        if (m_children[ii] != 0) {
          m_children[ii][jj]->insert(hit);
        } else {
          if (m_lastLevel) {
            m_children[ii][jj] = 0;
          } else {
            double dr = fabs(m_rMax - m_rMin) / 2.;
            int dtheta = (m_thetaMax - m_thetaMin) / 2 ;
            m_children[ii][jj] = new CDCLegendreQuadTree(m_rMin + dr * ii, m_rMin + dr * (ii + 1), m_thetaMin + dtheta * jj, m_thetaMin + dtheta * (jj + 1), m_level + 1, this);
          }
        }
      }
    }
  } else
    m_hits.push_back(hit);

  */
}

bool CDCLegendreQuadTree::checkLimitsR()
{

  if (m_level == 11)
    return true;

  if (fabs(m_rMax - m_rMin) < m_deltaR) {
    return true;
  }

  return false;
}
