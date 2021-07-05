/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
