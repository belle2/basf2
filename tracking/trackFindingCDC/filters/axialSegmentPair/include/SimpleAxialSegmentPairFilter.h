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
  namespace TrackingUtilities {
    using CDCAxialSegment2D = class CDCSegment2D;
  }
  namespace TrackFindingCDC {

    /// Filter for the construction of axial to axial segment pairs based on simple criteria
    class SimpleAxialSegmentPairFilter : public BaseAxialSegmentPairFilter {

    public:
      /// Constructor
      SimpleAxialSegmentPairFilter();

      /// Checks if a pair of axial segments is a good combination
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCAxialSegmentPair& axialSegmentPair) final;

    public:
      /// Returns the trajectory of the axial segment. Also fits it if necessary.
      const TrackingUtilities::CDCTrajectory2D& getFittedTrajectory2D(const TrackingUtilities::CDCAxialSegment2D& segment) const;

      /// Returns the trajectory of the axial to axial segment pair. Also fits it if necessary.
      const TrackingUtilities::CDCTrajectory2D& getFittedTrajectory2D(const TrackingUtilities::CDCAxialSegmentPair& axialSegmentPair)
      const;

      /// Returns the xy fitter instance that is used by this filter
      const CDCRiemannFitter& getRiemannFitter() const
      { return m_riemannFitter; }

    private:
      /// Memory of the Riemann fitter for the circle fits.
      CDCRiemannFitter m_riemannFitter;
    };
  }
}
