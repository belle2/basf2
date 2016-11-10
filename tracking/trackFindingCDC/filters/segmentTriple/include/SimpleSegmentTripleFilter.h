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

#include <tracking/trackFindingCDC/filters/segmentTriple/BaseSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/SimpleAxialSegmentPairFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of segment triples based on simple criterions
    class SimpleSegmentTripleFilter : public Filter<CDCSegmentTriple> {

    private:
      /// Type of the base class.
      using Super = Filter<CDCSegmentTriple>;

    public:
      /**
       *  Checks if a triple of axial, stereo and axial segments is a good combination to be stored.
       *  Checks the proper alignement and the quality of connection between all three segments.
       *  Returns NAN if the connection shall not be made or
       *  a finit value be used as the cell weight of the cell to constructed.
       */
      virtual Weight operator()(const CDCSegmentTriple& triple) override final;

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
