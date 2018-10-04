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

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>


namespace Belle2 {
  class CDCCKFStateCreator
    : public TrackFindingCDC::Findlet<CDCCKFState, const CDCCKFState, const TrackFindingCDC::WeightedRelation<TrackFindingCDC::CDCWireHit>> {
  public:
    void apply(std::vector<CDCCKFState>& nextStates, const CDCCKFPath& path,
               const std::vector<TrackFindingCDC::WeightedRelation<TrackFindingCDC::CDCWireHit>>& relations) override
    {
      // TODO
    }
  };
}
