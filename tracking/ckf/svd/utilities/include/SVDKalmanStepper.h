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

#include <tracking/ckf/general/utilities/KalmanStepper.h>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class SpacePoint;
  class CKFToSVDState;

  class SVDKalmanStepper : public KalmanStepper {
  public:
    double kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, CKFToSVDState& state);
  };
}