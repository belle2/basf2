/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/entities/CDCCKFState.h>

namespace Belle2 {
  std::ostream& operator<<(std::ostream& output, const CDCCKFState& state)
  {
    if (state.isSeed()) {
      output << "seed";
    } else {
      const auto* wireHit = state.getWireHit();
      const auto& wire = wireHit->getWire();
      output << wire.getICLayer() << " " << wire.getIWire();
    }
    return output;
  }
}
