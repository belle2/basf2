/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*
 * Conversion.cc
 *
 *  Created on: Jul 2, 2015
 *      Author: tkeck
 */

#include <framework/utilities/Conversion.h>
#include <stdexcept>

namespace Belle2 {

  /**
   * Converts a string to float
   * @param str string which is converted to float
   * @return converted float
   */
  template<>
  float convertString(const std::string& str)
  {
    std::string::size_type n;
    float number = std::stof(str, &n);
    if (n != str.size()) {
      throw std::invalid_argument("Could only parse a part of the given string " + str);
    }
    return number;
  }

  /**
   * Converts a string to double
   * @param str string which is converted to double
   * @return converted double
   */
  template<>
  double convertString(const std::string& str)
  {
    std::string::size_type n;
    double number = std::stod(str, &n);
    if (n != str.size()) {
      throw std::invalid_argument("Could only parse a part of the given string " + str);
    }
    return number;
  }

  /**
   * Converts a string to long double
   * @param str string which is converted to long double
   * @return converted long double
   */
  template<>
  long double convertString(const std::string& str)
  {
    std::string::size_type n;
    long double number = std::stold(str, &n);
    if (n != str.size()) {
      throw std::invalid_argument("Could only parse a part of the given string " + str);
    }
    return number;
  }

  /**
   * Converts a string to int
   * @param str string which is converted to int
   * @return converted int
   */
  template<>
  int convertString(const std::string& str)
  {
    std::string::size_type n;
    int number = std::stoi(str, &n);
    if (n != str.size()) {
      throw std::invalid_argument("Could only parse a part of the given string " + str);
    }
    return number;
  }

  /**
   * Converts a string to long int
   * @param str string which is converted to long int
   * @return converted long int
   */
  template<>
  long int convertString(const std::string& str)
  {
    std::string::size_type n;
    long int number = std::stol(str, &n);
    if (n != str.size()) {
      throw std::invalid_argument("Could only parse a part of the given string " + str);
    }
    return number;
  }

  /**
   * Converts a string to unsigned long int
   * @param str string which is converted to unsigned long int
   * @return converted unsigned long int
   */
  template<>
  unsigned long int convertString(const std::string& str)
  {
    std::string::size_type n;
    unsigned long int number = std::stoul(str, &n);
    if (n != str.size()) {
      throw std::invalid_argument("Could only parse a part of the given string " + str);
    }
    return number;
  }

}



