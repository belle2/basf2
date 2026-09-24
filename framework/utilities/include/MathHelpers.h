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

}
