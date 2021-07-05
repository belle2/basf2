/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
