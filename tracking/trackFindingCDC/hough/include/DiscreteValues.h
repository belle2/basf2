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

#include <framework/logging/Logger.h>
#include <functional>

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

      /// Default constructor for to obtain a valid object in all contexts.
      DiscreteValue() = default;

      /// Constructor from underlying iterator type
      explicit DiscreteValue(const Super& iter) : Super(iter) {;}

      /// Default copy constructor
      DiscreteValue(const DiscreteValue<T>&) = default;

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
      DiscreteValueArray(double lowerBound,
                         double upperBound,
                         size_t nPositions,
      const std::function<T(double)>& map = [](double d) { return T(d);})
      {
        std::vector<double> rawValues = linspace(lowerBound, upperBound, nPositions);
        m_values.reserve(rawValues.size());
        for (const double d : rawValues) {
          m_values.push_back(map(d));
        }
      }

    public:
      /** Constructs equally spaced discrete values with discrete overlap specification
       *
       *  @param map        Translation function from equally spaced doubles to the value type T
       *  @param lowerBound Lower bound of the value range
       *  @param upperBound Upper bound of the value range
       *  @param nBins      Total number of final bins to be constructed
       *  @param nWidth     Number of discrete values in each bin
       *                    (counted semi open [start, stop)).
       *  @param nOverlap   Number of discrete values that overlapping bins have in common
       *                    (counted semi open [start, stop)).
       */
      static
      DiscreteValueArray<T>
      forBinsWithOverlap(const std::function<T(double)>& map,
                         double lowerBound,
                         double upperBound,
                         size_t nBins,
                         size_t nWidth = 1,
                         size_t nOverlap = 0)
      {
        const size_t nPositions = (nWidth - nOverlap) * nBins + nOverlap + 1;
        return DiscreteValueArray<T>(lowerBound, upperBound, nPositions, map);
      }

      /** Constructs equally spaced discrete values
       *  for searches in the positive range.
       *
       *  The bounds are constructed such that the first bin
       *  corresponds to near zero values.
       *  To avoid cut of effects for hits that spill over to the
       *  subzero values due to their uncertainty, the
       *  lowest bin is slightly expanded such that its is symmetric around zero.
       *  To maintain an equal spacing all other bound are expanded accordingly.
       *
       *  @param map         Translation function from equally spaced doubles to the value type T
       *  @param upperBound  Upper bound of the positive range
       *  @param nBins       Total number of final bins to be constructed
       *  @param nWidth      Number of discrete values in each bin
       *                     (counted semi open [start, stop)).
       *  @param nOverlap    Number of discrete values that overlapping bins have in common
       *                     (counted semi open [start, stop)).
       */
      static
      DiscreteValueArray<T>
      forPositiveBinsWithOverlap(const std::function<T(double)>& map,
                                 double upperBound,
                                 size_t nBins,
                                 size_t nWidth = 1,
                                 size_t nOverlap = 0)
      {
        B2ASSERT((upperBound > 0),
                 "Maximum curvature values must be positive for positive curvature searches.");
        B2ASSERT((nWidth > nOverlap),
                 "Overlap must be smaller than the width.");

        const size_t nPositions = (nWidth - nOverlap) * nBins + nOverlap + 1;

        // Determining the lower bound such that the first bin is symmetric around zero
        // This prevents some cut of effects if the hit happens to lean to
        // the negative curvature spaces.
        const double overlapWidthRatio = static_cast<double>(nOverlap) / nWidth;
        const double width = upperBound / (nBins * (1 - overlapWidthRatio) + overlapWidthRatio - 0.5);
        const double lowerBound = -width / 2;
        return DiscreteValueArray<T>(lowerBound, upperBound, nPositions, map);
      }

      /// Get the first angle
      DiscreteValue<T> front() const
      { return DiscreteValue<T>(m_values.begin()); }

      /// Get the last angle
      DiscreteValue<T> back() const
      { return DiscreteValue<T>(--(m_values.end())); }

      /// Get the  angle with the given index
      DiscreteValue<T> operator[](size_t i) const
      { return DiscreteValue<T>(m_values.begin() + i); }

      /// Get the full range of the discrete values
      std::pair<DiscreteValue<T>, DiscreteValue<T> > getRange() const
      { return std::make_pair(front(), back()); }

    protected:
      /// Memory for the values representing the discrete positions.
      std::vector<T> m_values;
    };
  }
}
