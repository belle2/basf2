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
#include <framework/datastore/StoreArray.h>

#include <TVector3.h>
#include <string>
#include <list>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackCandidate;
    class TrackHit;

    class TrackFitter {
    public:
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
       * @param track Track candidate which are going to be fitted
       * @param with_drift_time Parameter which allows to perform fitting with taking into account drift time of hits; Not stable!
       */
      void fitTrackCandidateFast(
        TrackCandidate* track,
        bool with_drift_time = false);
    };
  }
}
