/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>
#include <framework/core/ModuleParamList.dcl.h>
#include <genfit/SharedPlanePtr.h>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class Advancer : public TrackFindingCDC::ProcessingSignalListener {
  public:
    double extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                              const genfit::SharedPlanePtr& plane);

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

    void setMaterialEffectsToParameterValue() const;
    void resetMaterialEffects() const;

  private:
    bool m_param_useMaterialEffects = false;
    double m_param_direction = 1;
  };
}