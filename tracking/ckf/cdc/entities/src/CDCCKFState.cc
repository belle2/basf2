/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
