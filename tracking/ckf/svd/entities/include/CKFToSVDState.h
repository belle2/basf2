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

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  /// Specialized CKF State for extrapolating into the SVD
  class CKFToSVDState : public CKFState<RecoTrack, SpacePoint> {
  public:
    /// Copy the constructors from the base class
    using CKFState::CKFState;

    /// Constructor setting the state to the position of the first CDC track seed hit
    CKFToSVDState(const RecoTrack* seed);

    unsigned int getGeometricalLayer() const;

    bool operator<(unsigned int layer)
    {
      return getGeometricalLayer() < layer;
    }

    friend bool operator<(unsigned int layer, const CKFToSVDState& state)
    {
      return layer < state.getGeometricalLayer();
    }
  };
}