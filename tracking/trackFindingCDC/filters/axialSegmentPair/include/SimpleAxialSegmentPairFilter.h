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

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class SimpleAxialSegmentPairFilter : public Filter<CDCAxialSegmentPair> {

    public:
      /// Constructor
      SimpleAxialSegmentPairFilter();

      /// Checks if a pair of axial segments is a good combination
      virtual Weight operator()(const CDCAxialSegmentPair& axialSegmentPair) override final;

    public:
      /// Returns the trajectory of the axial segment. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCAxialRecoSegment2D& segment) const;

      /// Returns the trajectory of the axial to axial segment pair. Also fits it if necessary.
      const CDCTrajectory2D& getFittedTrajectory2D(const CDCAxialSegmentPair& axialSegmentPair) const;

      /// Returns the xy fitter instance that is used by this filter
      const CDCRiemannFitter& getRiemannFitter() const
      { return m_riemannFitter; }

    private:
      /// Memory of the Riemann fitter for the circle fits.
      CDCRiemannFitter m_riemannFitter;

    }; // end class SimpleAxialSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
