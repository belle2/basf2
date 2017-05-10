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

#include <tracking/trackFindingCDC/utilities/CompositeProcessingSignalListener.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/ckf/states/CKFCDCToVXDStateObject.h>

namespace Belle2 {
  class SpacePointKalmanUpdateFitter : public TrackFindingCDC::CompositeProcessingSignalListener {
  public:
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {

    }

    TrackFindingCDC::Weight operator()(CKFCDCToVXDStateObject& currentState);
  };
}
