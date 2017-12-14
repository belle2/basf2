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
#include <pxd/reconstruction/PXDRecoHit.h>
#include <genfit/SharedPlanePtr.h>
#include <memory>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;
  class PXDRecoHit;

  /// Specialized CKF State for extrapolating into the PXD
  class CKFToPXDState : public CKFState<RecoTrack, SpacePoint> {
  public:
    /// When constructed by a hit, set the reco hit
    CKFToPXDState(const SpacePoint* hit);

    /// Constructor setting the state to the position of the first CDC track seed hit
    CKFToPXDState(const RecoTrack* seed);

    /// Extract the real layer this state sits on
    unsigned int getGeometricalLayer() const;

    /// Return the plane of the first PXD cluster
    genfit::SharedPlanePtr getPlane(const genfit::MeasuredStateOnPlane& state) const;

    const PXDRecoHit& getRecoHit() const;

    const std::vector<PXDRecoHit>& getRecoHits() const;

  private:
    std::vector<PXDRecoHit> m_recoHits;
  };
}