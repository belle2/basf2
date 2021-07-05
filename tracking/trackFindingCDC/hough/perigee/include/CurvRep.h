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
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Strategy to construct discrete curv points from discrete overlap specifications.
    class CurvBinsSpec {
    public:
      /**
       *  Constructs a specification for equally spaced discrete curvature values
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
      CurvBinsSpec(double lowerBound, double upperBound, long nBins, int nOverlap, int nWidth);

      /// Constuct the array of discrete curv positions
      DiscreteCurv::Array constructArray() const
      { return constructLinearArray(); }

      /// Constuct the array of discrete curv positions such that the inverse curvatures are distributed equally
      DiscreteCurv::Array constructInvLinearArray() const;

      /// Constuct the array of discrete curv positions such that the curvatures are distributed equally
      DiscreteCurv::Array constructLinearArray() const;

      /// Constuct the array of discrete curv positions including cache for the two dimensional arc length
      DiscreteCurvWithArcLength2DCache::Array constructCacheArray() const;

      /// Getter for the number of bounds
      long getNPositions() const;

      /**
       *  Getter for the bin width in real curv to investigate the value
       *  that results from the discrete overlap specification.
       */
      double getBinWidth() const;

      /**
       *  Getter for the overlap in real curv to investigate the value
       *  that results from the discrete overlap specification
       */
      double getOverlap() const;

      /// Getter for the overlap in discrete number of positions
      int getNOverlap() const
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

      /// Overlap of the leaves in curv counted in number of discrete values.
      int m_nOverlap = 1;

      /// Width of the leaves at the maximal level in curv counted in number of discrete values.
      int m_nWidth = 3;
    };

    /// Functor to get the lower curvature bound of a hough box.
    struct GetLowerCurv {
      /// Getter function for the lower curvature bound of a hough box - discrete version
      template <class AHoughBox, class SFINAE = std::enable_if_t<AHoughBox::template HasType<DiscreteCurv>::value > >
      float operator()(const AHoughBox& houghBox) const
      {
        return static_cast<float>(houghBox.template getLowerBound<DiscreteCurv>());
      }

      /// Getter function for the lower curvature bound of a hough box - continuous version
      template <class AHoughBox, class SFINAE = std::enable_if_t<AHoughBox::template HasType<ContinuousCurv>::value > >
      double operator()(const AHoughBox& houghBox) const
      {
        return static_cast<float>(houghBox.template getLowerBound<ContinuousCurv>());
      }
    };

    /// Functor to get the upper curvature bound of a hough box.
    struct GetUpperCurv {
      /// Getter function for the upper curvature bound of a hough box - discrete version
      template <class AHoughBox, class SFINAE = std::enable_if_t<AHoughBox::template HasType<DiscreteCurv>::value> >
      float operator()(const AHoughBox& houghBox) const
      {
        return static_cast<float>(houghBox.template getUpperBound<DiscreteCurv>());
      }

      /// Getter function for the upper curvature bound of a hough box - continuous version
      template <class AHoughBox, class SFINAE = std::enable_if_t<AHoughBox::template HasType<ContinuousCurv>::value> >
      double operator()(const AHoughBox& houghBox) const
      {
        return static_cast<float>(houghBox.template getUpperBound<ContinuousCurv>());
      }
    };

    /**
     *  Function to get the lower curvature bound of box.
     *  Returns 0 fo boxes that do not have a curvature coordinate.
     */
    template <class AHoughBox>
    float getLowerCurv(const AHoughBox& houghBox)
    {
      return getIfApplicable<float>(GetLowerCurv(), houghBox, 0.0);
    }

    /**
     *  Function to get the upper curvature bound of box.
     *  Returns 0 fo boxes that do not have a curvature coordinate
     */
    template <class AHoughBox>
    float getUpperCurv(const AHoughBox& houghBox)
    {
      return getIfApplicable<float>(GetUpperCurv(), houghBox, 0.0);
    }
  }
}
