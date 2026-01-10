/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentTriple/BaseSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/SimpleAxialSegmentPairFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the construction of segment triples based on simple criteria
    class SimpleSegmentTripleFilter : public TrackingUtilities::Filter<TrackingUtilities::CDCSegmentTriple> {

    private:
      /// Type of the base class.
      using Super = TrackingUtilities::Filter<TrackingUtilities::CDCSegmentTriple>;

    public:
      /**
       *  Checks if a triple of axial, stereo and axial segments is a good combination to be stored.
       *  Checks the proper alignment and the quality of connection between all three segments.
       *  Returns NAN if the connection shall not be made or
       *  a finit value be used as the cell weight of the cell to constructed.
       */
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCSegmentTriple& segmentTriple) final;

    private:
      /// Returns the xy fitter instance that is used by this filter
      const CDCRiemannFitter& getFitter2D() const
      { return m_riemannFitter; }

      /// Returns the sz fitter instance that is used by this filter.
      const CDCSZFitter& getSZFitter() const
      { return m_szFitter; }

    private:
      /// Memory of the SZ fitter fitting sz lines to the stereo segments
      CDCSZFitter m_szFitter;

      /// Memory of the Riemann fitter for the circle fits.
      CDCRiemannFitter m_riemannFitter;
    };

  }
}
