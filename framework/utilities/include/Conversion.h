/*
 * Conversion.h
 *
 *  Created on: Jul 2, 2015
 *      Author: tkeck
 */

#pragma once

#include <string>

namespace Belle2 {

  /**
   * Converts a string to type T (one of float, double, long double, int, long int, unsigned long int).
   *
   * Throws std::invalid_argument on conversion errors, out_of_range if T cannot hold the given value.
   * You can catch both possible exception types of the function (invalid_argument and out_of_range) using the base class logic_error
   */
  template<typename T>
  T convertString(const std::string& str);


}
