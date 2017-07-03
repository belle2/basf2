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

#include <genfit/MeasuredStateOnPlane.h>


namespace Belle2 {
  class SpacePoint;

  /**
   * Algorithm class for updating the track parameters and the covariance matrix based on the Kalman algorithm.
   *
   * For this, the state has to be already extrapolated to the plane of the space point.
   */
  class SpacePointKalmanUpdateFitter : public TrackFindingCDC::CompositeProcessingSignalListener {
  public:
    /// Reusable function to do the kalman update of a mSoP with the information in the clusters of the SP.
    double kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const SpacePoint* spacePoint) const;

    /// Currently, no parameters are exported.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {}

    /// Main function: update the parameters stored in the mSoP of the state using the SP related to this state.
    template <class AStateObject>
    TrackFindingCDC::Weight operator()(AStateObject& currentState) const
    {
      B2ASSERT("Encountered invalid state", not currentState.isFitted() and currentState.isAdvanced());

      const SpacePoint* spacePoint = currentState.getHit();

      if (not spacePoint) {
        // If we do not have a space point, we do not need to do anything here.
        currentState.setFitted();
        return 1;
      }

      genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState.getMeasuredStateOnPlane();

      const double chi2 = kalmanStep(measuredStateOnPlane, spacePoint);

      currentState.setMeasuredStateOnPlane(measuredStateOnPlane);
      currentState.setChi2(chi2);
      currentState.setFitted();
      return 1;
    }
  };
}
