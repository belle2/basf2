/*
 * Conversion.cc
 *
 *  Created on: Jul 2, 2015
 *      Author: tkeck
 */

#include <framework/utilities/Conversion.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Converts a string to float
   * @param str string which is converted to float
   * @return converted float
   */
  template<>
  float convert_string(const std::string& str)
  {
    std::string::size_type n;
    float number = std::stof(str, &n);
    if (n != str.size()) {
      B2WARNING("Could only parse a part of the given string " << str << " to a float value " << number);
    }
    return number;
  }

  /**
   * Converts a string to double
   * @param str string which is converted to double
   * @return converted double
   */
  template<>
  double convert_string(const std::string& str)
  {
    std::string::size_type n;
    double number = std::stod(str, &n);
    if (n != str.size()) {
      B2WARNING("Could only parse a part of the given string " << str << " to a double value " << number);
    }
    return number;
  }

  /**
   * Converts a string to long double
   * @param str string which is converted to long double
   * @return converted long double
   */
  template<>
  long double convert_string(const std::string& str)
  {
    std::string::size_type n;
    long double number = std::stold(str, &n);
    if (n != str.size()) {
      B2WARNING("Could only parse a part of the given string " << str << " to a long double value " << number);
    }
    return number;
  }

  /**
   * Converts a string to int
   * @param str string which is converted to int
   * @return converted int
   */
  template<>
  int convert_string(const std::string& str)
  {
    std::string::size_type n;
    int number = std::stoi(str, &n);
    if (n != str.size()) {
      B2WARNING("Could only parse a part of the given string " << str << " to a integer value " << number);
    }
    return number;
  }

  /**
   * Converts a string to long int
   * @param str string which is converted to long int
   * @return converted long int
   */
  template<>
  long int convert_string(const std::string& str)
  {
    std::string::size_type n;
    long int number = std::stol(str, &n);
    if (n != str.size()) {
      B2WARNING("Could only parse a part of the given string " << str << " to a long integer value " << number);
    }
    return number;
  }

  /**
   * Converts a string to unsigned long int
   * @param str string which is converted to unsigned long int
   * @return converted unsigned long int
   */
  template<>
  unsigned long int convert_string(const std::string& str)
  {
    std::string::size_type n;
    unsigned long int number = std::stoul(str, &n);
    if (n != str.size()) {
      B2WARNING("Could only parse a part of the given string " << str << " to a long integer value " << number);
    }
    return number;
  }

}



