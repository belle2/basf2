/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    using CDCAxialSegment2D = class CDCSegment2D;

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class SimpleAxialSegmentPairFilter : public BaseAxialSegmentPairFilter {

    public:
      /// Constructor
      SimpleAxialSegmentPairFilter();

      /// Checks if a pair of axial segments is a good combination
      Weight operator()(const CDCAxialSegmentPair& axialSegmentPair) final;

    public:
      /// Returns the trajectory of the axial segment. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCAxialSegment2D& segment) const;

      /// Returns the trajectory of the axial to axial segment pair. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCAxialSegmentPair& axialSegmentPair) const;

      /// Returns the xy fitter instance that is used by this filter
      const CDCRiemannFitter& getRiemannFitter() const
      { return m_riemannFitter; }

    private:
      /// Memory of the Riemann fitter for the circle fits.
      CDCRiemannFitter m_riemannFitter;
    };
  }
}
