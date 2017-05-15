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
    TrackFindingCDC::Weight operator()(CKFCDCToVXDStateObject& currentState) const;
  };
}
