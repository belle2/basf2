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

#include <tracking/trackFindingCDC/hough/DiscreteValues.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/numerics/numerics.h>

#include <vector>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class to represent a discrete angular position
    using DiscreteAngle = DiscreteValue<Vector2D>;

    /// An array accomodating discrete angular positions.
    class DiscreteAngleArray : public DiscreteValueArray<Vector2D> {

    public:
      /// Default constructor
      DiscreteAngleArray() = default;

      /** Create an evenly spaced range of angular values and precompute sin and cos.
       *  Note to get n bins you have to give  nPositions = nBins + 1 in this constructor
       *  Hence for a typical use case in a hough grid nPositions should be 2^(levels) + 1.
       */
      DiscreteAngleArray(double lowerBound, double upperBound, size_t nPositions) :
        DiscreteValueArray(lowerBound, upperBound, nPositions,
                           [](double phi) -> Vector2D {return Vector2D::Phi(phi);})
      {
      }

      /** Create an evenly spaced range of angular values and precompute sin and cos.
       *  Note to get n bins you have to give  nPositions = nBins + 1 in this constructor
       *  Hence for a typical use case in a hough grid nPositions should be 2^(levels) + 1.
       */
      explicit DiscreteAngleArray(size_t nPositions) : DiscreteAngleArray(-PI, PI, nPositions)
      {;}
    };
  }
}
