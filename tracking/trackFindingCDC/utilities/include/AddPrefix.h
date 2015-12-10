/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>
#include <memory>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Adds a prefix and captialises the value if necessary to get a camelCase concated name.
    inline std::string addPrefix(const std::string& prefix, const std::string& name)
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
  }
}
