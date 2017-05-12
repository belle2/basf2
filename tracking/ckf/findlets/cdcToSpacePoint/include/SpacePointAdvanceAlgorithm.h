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

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class SpacePoint;

  class SpacePointAdvanceAlgorithm : public TrackFindingCDC::CompositeProcessingSignalListener {
  public:
    bool extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const SpacePoint* spacePoint) const;

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      moduleParamList->addParameter("useMaterialEffects", m_param_useMaterialEffects,
                                    "Use material effects during extrapolation.", m_param_useMaterialEffects);
    }

    TrackFindingCDC::Weight operator()(CKFCDCToVXDStateObject& currentState) const;

  private:
    /// Parameter: use material effects
    bool m_param_useMaterialEffects = true;
  };
}
