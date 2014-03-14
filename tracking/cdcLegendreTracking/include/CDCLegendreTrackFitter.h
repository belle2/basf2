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
#include <cstdlib>
#include <vector>

#include "TVector3.h"


namespace Belle2 {

  class CDCLegendreTrackHit;
  class CDCLegendreTrackCandidate;

  class CDCLegendreTrackFitter {
  public:

    CDCLegendreTrackFitter(int nbinsTheta, double rMax, double rMin) :
      m_nbinsTheta(nbinsTheta), m_rMin(rMin), m_rMax(rMax) {};


    ~CDCLegendreTrackFitter();

    void fitTrackCandidateStepped(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track);

    void fitTrackCandidateFast(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track, std::pair<double, double>& ref_point);

    void fitTrackCandidateFast(CDCLegendreTrackCandidate* track, std::pair<double, double>& ref_point);

    void checkHits(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track, std::pair<double, double>& ref_point);

    void fitTrackCandidateNormalSpace(
      std::pair < std::vector<CDCLegendreTrackHit*>,
      std::pair<double, double> > * track,
      std::pair<double, double>* ref_point);

  private:

    double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
    double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/

    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
    static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/
    int m_nbinsTheta; /**< Number of bins in theta, derived from m_maxLevel*/
    int m_initialAxialHits; /**< Initial number of axial hits in the stepped hough algorithm*/

  };
}
