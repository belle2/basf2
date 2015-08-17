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

#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/DiscreteAngles.h>
#include <tracking/trackFindingCDC/hough/DiscreteValues.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Type for discrete float values
    class DiscreteCurv : public DiscreteValue<float> {

    public:
      /// Exposing the base class constructors
      using DiscreteValue<float>::DiscreteValue;
    };

    /// Type for the container of the discrete values
    class DiscreteCurvArray : public DiscreteValueArray<float> {

    private:
      /// Type of the base class.
      typedef DiscreteValueArray<float> Super;

    public:
      /// Inheriting the constuctor
      using Super::Super;

    public:
      /** Constructs equally spaced discrete curvature values with discrete overlap specification
       *
       *  @param minCurv  Lower bound of the curvature range
       *  @param maxCurv  Upper bound of the curvature range
       *  @param nBins    Total number of final bins to be constructed
       *  @param nWidth   Number of discrete values in each bin
       *                  (counted semi open [start, stop)).
       *  @param nOverlap Number of discrete values that overlapping bins have in common
       *                  (counted semi open [start, stop)).
       */
      static
      DiscreteCurvArray
      forCurvBinsWithOverlap(float minCurv,
                             float maxCurv,
                             size_t nBins,
                             size_t nWidth,
                             size_t nOverlap);

      /** Constructs equally spaced discrete curvature values
       *  for searches in the positive curvature range.
       *
       *  The bounds are constructed such that the first bin
       *  corresponds to near zero curvature values.
       *  To avoid cut of effects for hits that spill over to the
       *  subzero curvature values due to their uncertainty, the
       *  lowest bin is slightly expanded such that its is symmetric around zero.
       *  To maintain an equal spacing all other bound are expanded accordingly.
       *
       *  @param maxCurv  Upper bound of the positive curvature range
       *  @param nBins    Total number of final bins to be constructed
       *  @param nWidth   Number of discrete values in each bin
       *                  (counted semi open [start, stop)).
       *  @param nOverlap Number of discrete values that overlapping bins have in common
       *                  (counted semi open [start, stop)).
       */
      static
      DiscreteCurvArray
      forPositiveCurvBinsWithOverlap(float maxCurv,
                                     size_t nBins,
                                     size_t nWidth = 1,
                                     size_t nOverlap = 0);


    public:
      /// Getter for the first of the discrete curv values
      DiscreteCurv front() const
      { return DiscreteCurv(Super::front()); }

      /// Getter for the last of the discrete curv values
      DiscreteCurv back() const
      { return DiscreteCurv(Super::back()); }

      /// Getter for the complete curv range
      std::pair<DiscreteCurv, DiscreteCurv> getRange()
      { return std::pair<DiscreteCurv, DiscreteCurv>(front(), back()); }

    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
