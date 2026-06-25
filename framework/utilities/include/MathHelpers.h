/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {

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
   * Calculate the quadruple of the input
   * Ideally, this would be
   * ```
   * const T a = x * x;
   * return a * a;
   * ```
   * But with inlining this will likely result in x * x * x * x
   */
  template<typename T>
  inline constexpr T quadruple(const T& x)
  {
    return square(x) * square(x);
  }

}
