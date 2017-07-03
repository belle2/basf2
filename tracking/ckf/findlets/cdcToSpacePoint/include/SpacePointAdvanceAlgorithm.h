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

#include <genfit/MeasuredStateOnPlane.h>


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
    template <class AStateObject>
    TrackFindingCDC::Weight operator()(AStateObject& currentState) const
    {
      B2ASSERT("Encountered invalid state", not currentState.isFitted() and not currentState.isAdvanced());

      const SpacePoint* spacePoint = currentState.getHit();

      if (not spacePoint) {
        // If we do not have a space point, we do not need to do anything here.
        currentState.setAdvanced();
        return 1;
      }

      // This is the mSoP we will edit.
      genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState.getMeasuredStateOnPlane();

      if (not extrapolate(measuredStateOnPlane, spacePoint)) {
        return NAN;
      }

      currentState.setMeasuredStateOnPlane(measuredStateOnPlane);
      currentState.setAdvanced();
      return 1;
    }

  private:
    /// Parameter: use material effects during extrapolation.
    bool m_param_useMaterialEffects = true;
  };
}
