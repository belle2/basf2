/*
 * Conversion.h
 *
 *  Created on: Jul 2, 2015
 *      Author: tkeck
 */

#pragma once

#include <framework/logging/Logger.h>

#include <string>

namespace Belle2 {

  /**
   * Converts a string to type T
   * @param str string which is converted to type T
   * @return converted type T
   */
  template<typename T>
  T convert_string(const std::string& str);


}
