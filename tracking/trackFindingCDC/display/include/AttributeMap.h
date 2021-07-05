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

#include <map>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// A map type for attributes names to values for additional drawing information
    using AttributeMap = std::map<std::string, std::string>;
  }
}
