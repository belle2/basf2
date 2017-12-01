/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <set>

namespace Belle2 {

  /**
   * Generate a sequence of numbers defined by a string.
   * The syntax looks like this:
   * sequence = interval,interval,interval,...
   * interval = begin:end
   *            number
   *
   * For instance.
   *     3,23:27,101
   * Would generate the following sequence of numbers
   *     3,23,24,25,26,27,101
   *
   *  If intervals overlap the number is only returned once, so the following string
   *    27,24:28,26:29
   *  yields
   *    24,25,26,27,28,29
   *
   *  Negative numbers and intervals are supported as well
   *    -3:3
   *  yields
   *    -3,-2,-1,0,1,2,3
   *
   *  But reversed intervals are considered empty
   *    5:3
   *  yields an empty sequence
   *
   *  Returns a std::set, which contains the number sequence,
   *  each number (as stated above) occurs only once (obviously, since it's a set)
   *  and the numbers are ordered using the default Compare operator less<int> of std::set.
   */
  std::set<int64_t> generate_number_sequence(const std::string& str);

}
