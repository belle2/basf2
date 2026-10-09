/* Copyright 2026, DESY
   Authors: Christian Wessel

   This file is part of GENFIT.

   GENFIT is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GENFIT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GENFIT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

namespace genfit {

  /**
   * Calculate the square of the input
   */
  template<typename T>
  inline constexpr T square(const T& x)
  {
    return x * x;
  }
  /**
   * Calculate the cube of the input
   */
  template<typename T>
  inline constexpr T cube(const T& x)
  {
    return x * x * x;
  }
  /**
   * Calculate the fourth power of the input
   */
  template<typename T>
  inline constexpr T pow4(const T& x)
  {
    const T x2 = square(x);
    return x2 * x2;
  }
  /**
   * Calculate the fifth power of the input
   */
  template<typename T>
  inline constexpr T pow5(const T& x)
  {
    const T x4 = pow4(x);
    return x4 * x;
  }
  /**
   * Calculate the sixth power of the input
   */
  template<typename T>
  inline constexpr T pow6(const T& x)
  {
    const T x4 = pow4(x);
    const T x2 = square(x);
    return x4 * x2;
  }

  /**
   * Generalised method
   */
  template<typename T>
  inline constexpr T powN(const T x, const int N)
  {
    if (N == 3) {
      return cube(x);
    } else if (N == 4) {
      return pow4(x);
    } else if (N == 5) {
      return pow5(x);
    } else if (N == 6) {
      return pow6(x);
    } else {
      return std::pow(x, static_cast<T>(N));
    }
  }

}
