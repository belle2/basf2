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

#include <tracking/trackFindingCDC/numerics/numerics.h>

#include <vector>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class T>
    class DiscreteValue : public std::vector<T>::const_iterator {

    private:
      typedef typename std::vector<T>::const_iterator Super;
      typedef typename std::iterator_traits<Super>::difference_type DifferenceType;

    public:
      using Super::Super;

      /// Get the value
      const T& getValue() const
      { return **this; }

      /// Allow explicit conversion to the container type.
      explicit operator const T& () const
      { return getValue(); }

      // Comparision operator to the discrete values.
      bool operator<(const T& rhs) const
      { return getValue() < rhs; }

      /// Advance to next discrete values
      DiscreteValue<T> operator+(const DifferenceType& advanceBy) const
      {
        DiscreteValue<T> result(*this);
        std::advance(result, advanceBy);
        return result;
      }

      /// Advance to previous discrete values
      DiscreteValue<T> operator-(const DifferenceType& advanceBy) const
      {
        DiscreteValue<T> result(*this);
        std::advance(result, -advanceBy);
        return result;
      }
    };

    template<class T>
    class DiscreteValueArray {

    public:
      /** Create an evenly spaced range of floating point values.
       *  Note to get n bins you have to give  nPositions = nBins + 1 in this constructor
       *  Hence for a typical use case in a hough grid nPositions should be 2^(levels) + 1.
       */
      DiscreteValueArray(T lowerBound, T upperBound, size_t nPositions) :
        m_values(linspace(lowerBound, upperBound, nPositions))
      {;}

      /// Get the first angle
      DiscreteValue<T> front()
      { return DiscreteValue<T>(m_values.begin()); }

      /// Get the last angle
      DiscreteValue<T> back()
      { return DiscreteValue<T>(--(m_values.end())); }

      /// Get the  angle with the given index
      DiscreteValue<T> operator[](size_t i)
      { return DiscreteValue<T>(m_values.begin() + i); }

    private:
      /// Memory for the values representing the discrete positions.
      std::vector<T> m_values;
    };
  }
}
