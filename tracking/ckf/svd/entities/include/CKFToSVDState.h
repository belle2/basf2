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
#include <tracking/ckf/general/entities/CKFState.h>
#include <tracking/ckf/svd/entities/CKFToSVDResult.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  /// Specialized CKF State for extrapolating into the SVD
  class CKFToSVDState : public CKFState<RecoTrack, SpacePoint, CKFToSVDResult> {
    /// Constructor setting the state to the position of the first CDC track seed hit
    CKFToSVDState(RecoTrack* seed, unsigned int number);
  };
}