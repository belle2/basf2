/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <functional>
#include <vector>
#include <cmath>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class T>
    constexpr const T& clamp(const T& value, const T& lowerBound, const T& upperBound)
    {
      assert(not(upperBound < lowerBound));
      return (value < lowerBound) ? lowerBound : (upperBound < value) ? upperBound : value;
    }

    /** Returns n evenly spaced samples, calculated over the closed interval [start, stop ].*/
    template <class AResultType = double>
    std::vector<AResultType> linspace(double start,
                                      double final,
                                      std::size_t n,
                                      const std::function<AResultType(double)>& map)
    {
      std::vector<AResultType> result;
      result.reserve(n);
      result.push_back(map(start));
      for (std::size_t i = 1; i < n - 1; ++i) {
        result.push_back(map((start * (n - 1 - i) + final * i) / (n - 1)));
      }
      result.push_back(map(final));
      assert(result.size() == n);
      return result;
    }

    /** Returns n evenly spaced samples, calculated over the closed interval [start, stop ].*/
    template <class AResultType = double>
    std::vector<AResultType> linspace(double start, double final, std::size_t n)
    {
      auto map = [](const double in) -> AResultType {return AResultType(in);};
      return linspace<AResultType>(start, final, n, map);
    }

    /// Class which holds precomputed values of a function
    template<class T = double>
    class LookupTable {

    public:
      /**
       *  Constructs a look up table for the given function
       *  The function is sampled at n + 1 equally spaced points
       *  between the given bound, such that the distance
       *  between consecutive sampling points is (upper - lower) / nBins.
       */
      LookupTable(const std::function<T(double)>& map,
                  std::size_t nBins,
                  double lowerBound,
                  double upperBound)
        : m_lowerBound(lowerBound)
        , m_upperBound(upperBound)
        , m_binWidth((m_upperBound - m_lowerBound) / nBins)
        , m_values(linspace(lowerBound, upperBound, nBins + 1, map))
      {
        // Add a sentinal at the back.
        m_values.push_back(map(NAN));
      }

      /// Evaluate as piecewise linear interpolation
      T operator()(double x) const
      {
        if (not std::isfinite(x)) return m_values.back(); // Return sentinal
        const int iMax =  m_values.size() - 2; // Subtracting sentinal index
        double iBin = 0;
        double delta = std::modf((x - m_lowerBound) / m_binWidth, &iBin);
        int i = clamp(int(iBin), 0, iMax);
        return m_values[i] * (1 - delta) + m_values[i + 1] * delta;
      }

      /// Return the precomputed value at the position closest to the given value.
      const T& nearest(double x) const
      {
        if (not std::isfinite(x)) return m_values.back(); // Return sentinal value
        const int iMax =  m_values.size() - 2; // Subtracting sentinal index
        int iBin = std::round((x - m_lowerBound) / m_binWidth);
        int i = clamp(iBin, 0, iMax);
        return m_values[i];
      }

      /// Return the value at the given index
      const T& at(int i) const
      {
        const int iMax =  m_values.size() - 2; // Subtracting sentinal index
        i = clamp(i, 0, iMax);
        return m_values[i];
      }

      /// Return the number of bins in this lookup table
      int getNBins() const
      {
        return m_values.size() - 2;
      }

      /// Return the number of finite sampling points in this lookup table
      int getNPoints() const
      {
        return m_values.size() - 1;
      }

    private:
      /// Lower bound of the precomputed range
      double m_lowerBound;

      /// Upper bound of the precomputed range
      double m_upperBound;

      /// Distance between two precomputed positions
      double m_binWidth;

      /// Precomputed value
      std::vector<T> m_values;
    };
  }
}
