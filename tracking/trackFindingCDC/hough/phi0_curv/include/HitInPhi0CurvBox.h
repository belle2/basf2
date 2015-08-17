/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/perigee/StereoHitContained.h>
#include <tracking/trackFindingCDC/hough/perigee/InPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/perigee/OffOrigin.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of hits in a phi0 curv hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class HitInPhi0CurvBox : public StereoHitContained< OffOrigin<InPhi0CurvBox > > {

    public:
      /** Constructor taking a curvature below which the trajectory is specially treated as a non curler.
       *  Non curling trajectories generally excipit two different arms which should not be mixed.
       *  On the other hand curlers are allowed to have hits on both arms joined together.
       *  Default is that arms are allowed to be joined together.
       */
      explicit HitInPhi0CurvBox(float curlCurv = NAN, float rlWeightGain = 0) :
        StereoHitContained< OffOrigin<InPhi0CurvBox > >(fabs(curlCurv))
      { setRLWeightGain(rlWeightGain); }
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
