/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

namespace Belle2 {
  class CDCCKFStateFilter : public TrackFindingCDC::Findlet<const CDCCKFState, CDCCKFState> {
  public:
    void apply(const CDCCKFPath& path, std::vector<CDCCKFState>& nextStates) override
    {
    }
  };
}
