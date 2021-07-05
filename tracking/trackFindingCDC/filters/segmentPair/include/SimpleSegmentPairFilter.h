/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;
    class CDCSegment2D;
    class CDCTrajectory3D;
    class CDCTrajectory2D;

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class SimpleSegmentPairFilter : public BaseSegmentPairFilter {

    public:
      /// Checks if a pair of segments is a good combination
      Weight operator()(const CDCSegmentPair& segmentPair) final;

      /// Returns the trajectory of the segment. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCSegment2D& segment) const;

      /**
       *  Returns the three dimensional trajectory of the axial stereo segment pair.
       *  Also fits it if necessary.
       */
      const CDCTrajectory3D& getFittedTrajectory3D(const CDCSegmentPair& segmentPair) const;

      /// Returns the xy fitter instance that is used by this filter.
      const CDCRiemannFitter& getRiemannFitter() const
      {
        return m_riemannFitter;
      }

    private:
      /// Memory of the Riemann fitter for the circle fits.
      CDCRiemannFitter m_riemannFitter;
    };
  }
}
