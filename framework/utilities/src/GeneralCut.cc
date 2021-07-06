/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/GeneralCut.h>
#include <cmath>

namespace Belle2 {

  bool almostEqualFloat(const float& a, const float& b)
  {
    static_assert(sizeof(float) == sizeof(int32_t));
    // cppcheck-suppress invalidPointerCast
    return std::fabs(*(int32_t*)&a - * (int32_t*)&b) <= 2 or (a == b);
  }

  bool almostEqualDouble(const double& a, const double& b)
  {
    static_assert(sizeof(double) == sizeof(int64_t));
    // cppcheck-suppress invalidPointerCast
    return std::fabs(*(int64_t*)&a - * (int64_t*)&b) <= 2 or (a == b);
  }

  unsigned long int findMatchedParenthesis(std::string str, char open, char close)
  {
    unsigned long int end = 1;
    if (str[0] == open) {
      unsigned int count = 1;
      for (end = 1; end < str.size() and count > 0; ++end) {
        if (str[end] == open) ++count;
        else if (str[end] == close) --count;
      }

      if (count > 0)
        throw std::runtime_error("Variable string has an invalid format: " + str);
    }
    return end - 1;
  }

  unsigned long int findIgnoringParenthesis(std::string str, std::string pattern, unsigned int begin)
  {

    if (str.size() < pattern.size())
      return std::string::npos;

    for (unsigned int i = begin; i < str.size() - pattern.size(); ++i) {
      if (str[i] == '[') {
        i += findMatchedParenthesis(str.substr(i), '[', ']');
        continue;
      }
      if (str[i] == '(') {
        i += findMatchedParenthesis(str.substr(i), '(', ')');
        continue;
      }
      if (str[i] == '{') {
        i += findMatchedParenthesis(str.substr(i), '{', '}');
        continue;
      }

      for (unsigned int j = 0; j < pattern.size(); ++j) {
        if (str[i + j] != pattern[j]) {
          break;
        }
        if (j == pattern.size() - 1) {
          return i;
        }
      }
    }
    return std::string::npos;
  }

  std::vector<std::string> splitOnDelimiterAndConserveParenthesis(std::string str, char delimiter, char open, char close)
  {

    std::vector<std::string> result;
    unsigned int lastdelimiter = 0;
    for (unsigned int i = 0; i < str.size(); ++i) {
      if (str[i] == open) {
        i += findMatchedParenthesis(str.substr(i), open, close);
        continue;
      }
      if (str[i] == delimiter) {
        result.push_back(str.substr(lastdelimiter, i - lastdelimiter));
        lastdelimiter = i + 1;
      }
    }
    std::string last = str.substr(lastdelimiter);
    if (last.size() != 0) {
      result.push_back(last);
    }
    return result;
  }

}
