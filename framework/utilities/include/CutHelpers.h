/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>

namespace Belle2 {
  /**
   * Returns position of the matched closing parenthesis if the first character in the given
   * string contains an opening parenthesis. Otherwise return 0.
   */
  unsigned long int findMatchedParenthesis(std::string str, char open = '[', char close = ']');

  /**
   * Split into std::vector on delimiter ignoring delimiters between parenthesis
   */
  std::vector<std::string> splitOnDelimiterAndConserveParenthesis(std::string str, char delimiter, char open, char close);

  /**
   * Returns the position of a pattern in a string ignoring everything that is in parenthesis.
   */
  unsigned long int findIgnoringParenthesis(std::string str, std::string pattern, unsigned int begin = 0);

  /**
   * Helper function to test if two floats are almost equal.
   */
  bool almostEqualFloat(const float& a, const float& b);

  /**
   * Helper function to test if two doubles are almost equal.
   */
  bool almostEqualDouble(const double& a, const double& b);

}
