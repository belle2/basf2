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

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/ckf/states/CKFCDCToVXDStateObject.h>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class SpacePoint;

  /**
   * Algorithm class to extrapolate a state onto the plane of its related space point.
   */
  class SpacePointAdvanceAlgorithm : public TrackFindingCDC::ProcessingSignalListener {
  public:
    /**
     * Reusable extrapolate function to extrapolate a mSoP to the plane of the first SVD cluster
     * related to the given space point.
     */
    bool extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const SpacePoint* spacePoint) const;

    /// Expose the useMaterialEffects parameter.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      moduleParamList->addParameter("useMaterialEffects", m_param_useMaterialEffects,
                                    "Use material effects during extrapolation.", m_param_useMaterialEffects);
    }

    /// Main function: extrapolate the state to its related space point. Returns NAN, if the extrapolation fails.
    TrackFindingCDC::Weight operator()(CKFCDCToVXDStateObject& currentState) const;

  private:
    /// Parameter: use material effects during extrapolation.
    bool m_param_useMaterialEffects = true;
  };
}
