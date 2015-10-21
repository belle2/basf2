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
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <framework/datastore/StoreArray.h>

#include <TVector3.h>
#include <string>
#include <list>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackCandidate;
    class TrackHit;

    class CDCTrajectory2D;
    class CDCWireHit;
    class CDCTrack;

    class TrackFitter {
    public:

      double fitTrackCandidateFast(
        std::vector<const CDCWireHit*>& hits,
        std::pair<double, double>& track_par,
        std::pair<double, double>& ref_point);


      static double fitTrackCandidateFast(
        std::vector<std::pair<std::pair<double, double>, double>>& hits);

      static CDCTrajectory2D fitCDCTrackWhithoutRecoPos(CDCTrack& track);

      static CDCTrajectory2D fitWireHitsWhithoutRecoPos(std::vector<const CDCWireHit*>& wireHits);

      void update(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D)
      {
        CDCRiemannFitter fitter;
        fitter.update(trajectory2D, observations2D);
      }


    };
  }
}
