/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <functional>
#include <utility>
#include <cmath>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Computes the modulo operation always yielding a positive result.
     *  The implementation asserts the proper behaviour even if negative numbers are put in.
     */
    inline int modulo(int numerator, int denominator)
    {
      return ((numerator % denominator) + denominator) % denominator;
    }

    /**
     *  Computes the modulo operation always yielding a positive result.
     *  The implementation asserts the proper behaviour even if negative numbers are put in.
     *
     *  The fast implementation avoids the division operation in favour of additions.
     *  Only faster if the numerator is close to the desired value.
     */
    inline int moduloFast(int numerator, int denominator)
    {
      while (numerator < 0) numerator += denominator;
      while (numerator >= denominator) numerator -= denominator;
      return numerator;
    }

    /**
     *  Computes the modulo operation with result between (denominator + 1) / 2 and -denominator / 2
     *  Therefore the result is always the one closest to zero.
     *  Its behaviour is equivalent to std::remainder but for integer numbers.
     */
    inline int symmetricModulo(int numerator, int denominator)
    {
      return modulo(numerator + denominator / 2, denominator) - denominator / 2;
    }

    /**
     *  Computes the modulo operation with result between (denominator + 1) / 2 and -denominator / 2
     *  Therefore the result is always the one closest to zero.
     *  Its behaviour is equivalent to std::remainder but for integer numbers.
     *
     *  The fast implementation avoids the division operation in favour of additions.
     *  Only faster if the numerator is close to the desired value.
     */
    inline int symmetricModuloFast(int numerator, int denominator)
    {
      return moduloFast(numerator + denominator / 2, denominator) - denominator / 2;
    }

    /// Checks if an integer is even
    template<class AInt>
    inline bool isEven(const AInt& x)
    {
      return (x % 2) == 0;
    }

    /// Checks if an integer is odd
    template<class AInt>
    inline bool isOdd(const AInt& x)
    {
      return (x % 2) != 0;
    }
  }
}
