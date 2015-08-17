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

#include <tracking/trackFindingCDC/hough/DiscreteAngles.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type to represent an array of discrete phi0 values
    using DiscretePhi0Array = DiscreteAngleArray;

    /// Type to represent a discrete phi0 value
    class DiscretePhi0 : public DiscreteAngle {
    public:
      /// Inheriting the constructor of the base class
      using DiscreteAngle::DiscreteAngle;

      /// The type of the array which contains the underlying values.
      using Array = DiscretePhi0Array;

    public:
      /// Extract the range from an array providing the discrete values.
      static std::pair<DiscretePhi0, DiscretePhi0> getRange(const DiscretePhi0Array& valueArray)
      { return {DiscretePhi0(valueArray.front()), DiscretePhi0(valueArray.back())}; }
    };


    /// Strategy to construct discrete phi0 points from discrete overlap specifications.
    class Phi0BinsSpec {
    public:
      /** Constructor from fixed number of accessable bins and overlap specification
       *  in discrete number of positions */
      Phi0BinsSpec(size_t nBins, size_t nOverlap, size_t nWidth);

      /// Constuct the array of discrete phi0 positions
      DiscretePhi0Array constructArray() const;

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
