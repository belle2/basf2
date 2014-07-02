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

    CDCLegendreTrackFitter(int nbinsTheta, double rMax, double rMin, bool fitTracks) :
      m_nbinsTheta(nbinsTheta), m_rMin(rMin), m_rMax(rMax), m_fitTracks(fitTracks)
    {};


    ~CDCLegendreTrackFitter();

    /*
     * Perform fast circular fitting
     */
    double fitTrackCandidateFast(std::vector<CDCLegendreTrackHit*>& hits, std::pair<double, double>& track_par, std::pair<double, double>& ref_point, bool with_drift_time = false);

    /*
     * Perform fast circular fitting
     */
    void fitTrackCandidateFast(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track, std::pair<double, double>& ref_point, double& chi2, bool with_drift_time = false);

    /*
     * Perform fast circular fitting
     */
    void fitTrackCandidateFast(CDCLegendreTrackCandidate* track, std::pair<double, double>& ref_point, bool with_drift_time = false);

    /*
     * Perform fast circular fitting
     */
    void fitTrackCandidateFast(CDCLegendreTrackCandidate* track, bool with_drift_time = false);

    /**
     * Simple fitter which perform stepped fitting in normal space, not the best choice in sense of performance
     */
    void fitTrackCandidateNormalSpace(
      std::pair < std::vector<CDCLegendreTrackHit*>,
      std::pair<double, double> > * track,
      std::pair<double, double>* ref_point);

    /**
     * Simple fitter which perform stepped fitting in conformal space, not the best choice in sense of performance
     */
    void fitTrackCandidateStepped(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track);


  private:


    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
    static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/

    int m_nbinsTheta; /**< Number of bins in theta, derived from m_maxLevel*/
    double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
    double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/
    bool m_fitTracks; /**< Decides whether fitting will be applied*/

  };
}
