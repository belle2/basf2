/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCRecoSegment2D;

    /// A segment consisting of three dimensional reconstructed hits.
    class CDCRecoSegment3D : public CDCSegment<CDCRecoHit3D> {

    public:
      /// Reconstructs a two dimensional stereo segment by shifting each hit onto the given two dimensional trajectory.
      static CDCRecoSegment3D reconstruct(const CDCRecoSegment2D& segment2D,
                                          const CDCTrajectory2D& trajectory2D);

      /** Constructs a two dimensional segment by carrying out the stereo ! projection to the wire reference postion.
       * Note : no fitting information is transported to the resulting segment.
       */
      CDCRecoSegment2D stereoProjectToRef() const;

      /// Returns true of there is one hit in the range which has a reconstructed position in the CDC (scaled by the factor).
      bool atLeastOneHitIsNotInCDCZBounds(const double factor = 1) const;
    };
  }
}


