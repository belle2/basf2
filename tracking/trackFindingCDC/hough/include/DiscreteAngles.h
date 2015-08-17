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

    class DiscreteAngle : public std::vector<Vector2D>::const_iterator {

    public:
      typedef std::vector<Vector2D>::const_iterator Super;

      using Super::Super;

      /// Default constructor for to obtain a valid object in all contexts.
      DiscreteAngle() = default;

      /// Constructor from underlying iterator type
      explicit DiscreteAngle(const Super& iter) : Super(iter) {;}

      /// Default copy constructor
      DiscreteAngle(const DiscreteAngle&) = default;

      const Vector2D& getValue() const
      {
        return **this;
      }

      const Vector2D& getAngleVec() const
      {
        return **this;
      }

      double getAngle() const
      {
        return getAngleVec().phi();
      }

      double getCos() const
      {
        return getAngleVec().x();
      }

      double getSin() const
      {
        return getAngleVec().y();
      }

      /// Allow explicit casting to the angle vector
      operator const Vector2D& () const
      { return getAngleVec(); }

      /// Advance to next discrete values
      DiscreteAngle operator+(const std::iterator_traits<Super>::difference_type& advance_by) const
      {
        DiscreteAngle result(*this);
        std::advance(result, advance_by);
        return result;
      }
      /// Advance to previous discrete values
      DiscreteAngle operator-(const std::iterator_traits<Super>::difference_type& advance_by) const
      {
        DiscreteAngle result(*this);
        std::advance(result, -advance_by);
        return result;
      }
    };

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

    public:
      /// Get the first angle
      DiscreteAngle front() const
      { return DiscreteAngle(m_values.begin()); }

      /// Get the last angle
      DiscreteAngle back() const
      { return DiscreteAngle(--(m_values.end())); }

      /// Get the  angle with the given index
      DiscreteAngle operator[](size_t i) const
      { return DiscreteAngle(m_values.begin() + i); }

      /// Get the full range of the discrete values
      std::pair<DiscreteAngle, DiscreteAngle> getRange() const
      { return std::make_pair(front(), back()); }

    };
  }
}
