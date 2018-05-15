/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <regex>

namespace Belle2 {
  /** Utility functions related to environment variables */
  class EnvironmentVariables {
  public:
    /** Check if a value is set in the database */
    static bool isSet(const std::string& name);

    /** Get the value of an environment variable or the given fallback value if the variable is not set */
    static std::string get(const std::string& name, const std::string fallback = "");

    /** Get a list of values from an environment variable or the given fallback
     * list if the variable is not set.
     *
     * By default the value of the environment variable is split by whitespace
     * (" \t\n\r") but a different list of characters can be supplied, for
     * example ":" for path lists or ", " for comma or space separated values
     * */
    static std::vector<std::string> getList(const std::string& name, const std::vector<std::string>& fallback = {},
                                            const std::string& separators = " \t\n\r");

    /** Modify the given string and replace every occurence of $NAME or ${NAME}
     * with the value of the environment variable NAME. Variables which are not
     * found are returned unchanged
     */
    static std::string expand(const std::string& text);
  };
};
