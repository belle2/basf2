/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <algorithm>
#include <sstream>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Adds a prefix and captialises the name if necessary to get a camelCase concatenated name.
    inline std::string prefixed(const std::string& prefix, const std::string& name)
    {
      if (prefix == "") {
        return name;
      } else if (name == "") {
        return prefix;
      } else {
        std::string captialisedName = name;
        captialisedName[0] = ::toupper(name.at(0));
        return prefix + captialisedName;
      }
    }

    /// Add quoting characters to a string.
    inline std::string quoted(const std::string& text)
    {
      return "\"" + text + "\"";
    }

    /// bAdd quoting characters to a string.
    inline std::string bracketed(const std::string& text)
    {
      return "(" + text + ")";
    }

    /// Join together a series of texts with the given separator.
    template <class Ts>
    std::string join(const std::string& sep, const Ts& texts)
    {
      auto it = std::begin(texts);
      auto itEnd = std::end(texts);
      if (it == itEnd) return std::string{};
      std::ostringstream result;
      result << *it++;
      for (; it != itEnd; ++it) {
        result << sep << *it;
      }
      return result.str();
    }

  }
}
