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

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLTaggedWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

#include <tracking/trackFindingCDC/hough/perigee/StereoHitIn.h>
#include <tracking/trackFindingCDC/hough/perigee/Phi0CurvSweepedXLine.h>

// #include <tracking/trackFindingCDC/hough/phi0_curv/Phi0CurvBox.h>
// #include <tracking/trackFindingCDC/hough/phi0_curv/RLTagged.h>
// #include <tracking/trackFindingCDC/hough/SameSignChecker.h>

#include <tracking/trackFindingCDC/numerics/numerics.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of hits in a phi0 curv hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class HitInPhi0CurvBox : public StereoHitIn< Phi0CurvSweepedXLine > {

    public:
      /** Constructor taking a curvature below which the trajectory is specially treated as a non curler.
       *  Non curling trajectories generally excipit two different arms which should not be mixed.
       *  On the other hand curlers are allowed to have hits on both arms joined together.
       *  Default is that arms are allowed to be joined together.
       */
      explicit HitInPhi0CurvBox(float curlCurv = NAN, float rlWeightGain = 0) :
        StereoHitIn< Phi0CurvSweepedXLine >(fabs(curlCurv))
      { setRLWeightGain(rlWeightGain); }
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
