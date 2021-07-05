/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

namespace Belle2 {
  std::ostream& operator<<(std::ostream& output, const CDCCKFPath& path)
  {
    output << "[";
    for (const auto& state : path) {
      output << state << ", ";
    }
    output << "]";
    return output;
  }
}
