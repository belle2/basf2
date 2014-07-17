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
  namespace TrackFinderCDCLegendre {

    class TrackHit;
    class TrackCandidate;

    class TrackFitter {
    public:

      TrackFitter(int nbinsTheta, double rMax, double rMin, bool fitTracks) :
        m_nbinsTheta(nbinsTheta), m_rMin(rMin), m_rMax(rMax), m_fitTracks(fitTracks)
      {};


      ~TrackFitter();

      /**
       * @brief Perform fast circular fitting
       * @param hits Hits pattern which we are going to fit
       * @param track_par Set of track parameters - polar angle and curvature of track
       * @param ref_point Reference point; In current case - point of closest approach to IP
       * @param with_drift_time Parameter which allows to perform fitting with taking into account drift time of hits; Not stable!
       */
      double fitTrackCandidateFast(
        std::vector<TrackHit*>& hits,
        std::pair<double, double>& track_par,
        std::pair<double, double>& ref_point,
        bool with_drift_time = false);

      /**
       * @brief Perform fast circular fitting
       * @param track Track candidate representation; include set of hits, and parameters of the track (theta and curvature)
       * @param ref_point Reference point; In current case - point of closest approach to IP
       * @param chi2 Chi2 of fit
       * @param with_drift_time Parameter which allows to perform fitting with taking into account drift time of hits; Not stable!
       */
      void fitTrackCandidateFast(
        std::pair < std::vector<TrackHit*>,
        std::pair<double, double> > * track,
        std::pair<double, double>& ref_point,
        double& chi2,
        bool with_drift_time = false);

      /**
       * @brief Perform fast circular fitting
       * @param track Track candidate which are going to be fitted
       * @param ref_point Reference point; In current case - point of closest approach to IP
       * @param with_drift_time Parameter which allows to perform fitting with taking into account drift time of hits; Not stable!
       */
      void fitTrackCandidateFast(
        TrackCandidate* track,
        std::pair<double, double>& ref_point,
        bool with_drift_time = false);

      /**
       * @brief Perform fast circular fitting
       * @param track Track candidate which are going to be fitted
       * @param with_drift_time Parameter which allows to perform fitting with taking into account drift time of hits; Not stable!
       */
      void fitTrackCandidateFast(
        TrackCandidate* track,
        bool with_drift_time = false);

      /** Simple fitter which perform stepped fitting in normal space, not the best choice in sense of performance */
      void fitTrackCandidateNormalSpace(
        std::pair < std::vector<TrackHit*>,
        std::pair<double, double> > * track,
        std::pair<double, double>* ref_point);

      /** Simple fitter which perform stepped fitting in conformal space, not the best choice in sense of performance */
      void fitTrackCandidateStepped(
        std::pair < std::vector<TrackHit*>,
        std::pair<double, double> > * track);

      /** Correct estimation of chi2 */
      double estimateChi2(
        std::vector<TrackHit*>& hits,
        std::pair<double, double>& track_par,
        std::pair<double, double>& ref_point);

    private:


      static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
      static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/

      int m_nbinsTheta; /**< Number of bins in theta, derived from m_maxLevel*/
      double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
      double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/
      bool m_fitTracks; /**< Decides whether fitting will be applied*/

    };
  }
}
