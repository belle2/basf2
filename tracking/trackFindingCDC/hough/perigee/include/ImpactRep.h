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

#include <tracking/trackFindingCDC/hough/axes/StandardAxes.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Strategy to construct discrete impact points from discrete overlap specifications.
    class ImpactBinsSpec {
    public:
      /**
       *  Constructs a specification for equally spaced discrete impactature values
       *  with discrete overlap specification
       *
       *  @param lowerBound Lower bound of the value range
       *  @param upperBound Upper bound of the value range
       *  @param nBins      Total number of final bins to be constructed
       *  @param nWidth     Number of discrete values in each bin
       *                    (counted semi open [start, stop)).
       *  @param nOverlap   Number of discrete values that overlapping bins have in common
       *                    (counted semi open [start, stop)).
       */
      ImpactBinsSpec(double lowerBound, double upperBound, long nBins, int nOverlap, int nWidth);

      /// Constuct the array of discrete impact positions
      DiscreteImpact::Array constructArray() const;

      /// Getter for the number of bounds
      long getNPositions() const;

      /**
       *  Getter for the bin width in real impact to investigate the value
       *  that results from the discrete overlap specification
       */
      double getBinWidth() const;

      /**
       *  Getter for the overlap in real impact to investigate the value
       *  that results from the discrete overlap specification
       */
      double getOverlap() const;

      /// Getter for the overlap in discrete number of positions
      long getNOverlap() const
      {
        return m_nOverlap;
      }

    private:
      /// Lower bound of the binning range
      double m_lowerBound;

      /// Upper bound of the binning range
      double m_upperBound;

      /// Number of accessable bins
      long m_nBins;

      /// Overlap of the leaves in impact counted in number of discrete values.
      int m_nOverlap = 1;

      /// Width of the leaves at the maximal level in impact counted in number of discrete values.
      int m_nWidth = 3;
    };
  }
}
