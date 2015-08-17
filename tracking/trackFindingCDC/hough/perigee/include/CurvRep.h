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

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Phantom type tag for the discrete curvature representation
    class CurvTag;

    /// Type for discrete curv values
    using DiscreteCurv = DiscreteValue<float, CurvTag>;

    /// Strategy to construct discrete curv points from discrete overlap specifications.
    class CurvBinsSpec {
    public:
      /** Constructs a specification for equally spaced discrete curvature values
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
      CurvBinsSpec(double lowerBound,
                   double upperBound,
                   size_t nBins,
                   size_t nOverlap,
                   size_t nWidth);

      /// Constuct the array of discrete curv positions
      DiscreteCurv::Array constructArray() const;

      /// Getter for the number of bounds
      size_t getNPositions() const;

      /** Getter for the bin width in real curv to investigate the value
       *  that results from the discrete overlap specification*/
      double getBinWidth() const;

      /** Getter for the overlap in real curv to investigate the value
       *  that results from the discrete overlap specification*/
      double getOverlap() const;

      /// Getter for the overlap in discrete number of positions
      size_t getNOverlap() const
      { return m_nOverlap; }

    private:
      /// Lower bound of the binning range
      double m_lowerBound;

      /// Upper bound of the binning range
      double m_upperBound;

      /// Number of accessable bins
      size_t m_nBins;

      /// Overlap of the leaves in curv counted in number of discrete values.
      size_t m_nOverlap = 1;

      /// Width of the leaves at the maximal level in curv counted in number of discrete values.
      size_t m_nWidth = 3;
    };

    /** Function to get the lower curvature bound of box.
     *  Default implementation returning 0*/
    template<class HoughBox>
    float getLowerCurvImpl(const HoughBox& houghBoxWithOutCurv, long)
    { return 0; }

    /** Function to get the lower curvature bound of box.
     *  Implementation for boxes with a curvature coordinate returns
     *  the lower curvature bound.
     */
    template<class HoughBox>
    auto getLowerCurvImpl(const HoughBox& houghBoxWithCurv, int) ->
    decltype(houghBoxWithCurv.template getLowerBound<DiscreteCurv>())
    { return houghBoxWithCurv.template getLowerBound<DiscreteCurv>(); }

    /** Function to get the lower curvature bound of box.
     *  Returns 0 fo boxes that do not have a curvature coordinate*/
    template<class HoughBox>
    float getLowerCurv(const HoughBox& houghBox)
    { return float(getLowerCurvImpl(houghBox, 0)); }


    /** Function to get the upper curvature bound of box.
     *  Default implementation returning 0*/
    template<class HoughBox>
    float getUpperCurvImpl(const HoughBox& houghBoxWithOutCurv, long)
    { return 0; }

    /** Function to get the upper curvature bound of box.
     *  Implementation for boxes with a curvature coordinate returns
     *  the upper curvature bound.
     */
    template<class HoughBox>
    auto getUpperCurvImpl(const HoughBox& houghBoxWithCurv, int) ->
    decltype(houghBoxWithCurv.template getUpperBound<DiscreteCurv>())
    { return houghBoxWithCurv.template getUpperBound<DiscreteCurv>(); }

    /** Function to get the upper curvature bound of box.
     *  Returns 0 fo boxes that do not have a curvature coordinate*/
    template<class HoughBox>
    float getUpperCurv(const HoughBox& houghBox)
    { return float(getUpperCurvImpl(houghBox, 0)); }

  } // end namespace TrackFindingCDC
} // end namespace Belle2
