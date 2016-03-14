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

#include <tracking/trackFindingCDC/hough/DiscreteValue.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Phantom type tag for the discrete phi0 representation
    class Phi0Tag;

    /// Type for discrete phi0 values
    using DiscretePhi0 = DiscreteValue<Vector2D, Phi0Tag>;

    /// Strategy to construct discrete phi0 points from discrete overlap specifications.
    class Phi0BinsSpec {
    public:
      /** Constructor from fixed number of accessable bins and overlap specification
       *  in discrete number of positions */
      Phi0BinsSpec(size_t nBins, size_t nOverlap, size_t nWidth);

      /// Constuct the array of discrete phi0 positions
      DiscretePhi0::Array constructArray() const;

      /// Getter for the number of bounds
      size_t getNPositions() const;

      /** Getter for the bin width in real phi0 to investigate the value
       *  that results from the discrete overlap specification*/
      double getBinWidth() const;

      /** Getter for the overlap in real phi0 to investigate the value
       *  that results from the discrete overlap specification*/
      double getOverlap() const;

      /// Getter for the overlap in discrete number of positions
      size_t getNOverlap() const
      { return m_nOverlap; }

    private:
      ///  Number of accessable bins
      size_t m_nBins;

      /// Overlap of the leaves in phi0 counted in number of discrete values.
      size_t m_nOverlap = 1;

      /// Width of the leaves at the maximal level in phi0 counted in number of discrete values.
      size_t m_nWidth = 3;
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
