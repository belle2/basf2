/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/ckf/general/entities/CKFState.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <genfit/SharedPlanePtr.h>

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
    explicit CKFToPXDState(const SpacePoint* hit);

    /// Constructor setting the state to the position of the first CDC track seed hit
    explicit CKFToPXDState(const RecoTrack* seed, bool reversed = false);

    /// Extract the real layer this state sits on
    unsigned int getGeometricalLayer() const;

    /// Return the plane of the first PXD cluster
    genfit::SharedPlanePtr getPlane(const genfit::MeasuredStateOnPlane& state) const;

    /// Helper function for getting the already created reco hit (runtime reasons)
    const PXDRecoHit& getRecoHit() const;

    /// Helper function for getting the already created reco hits (runtime reasons)
    const std::vector<PXDRecoHit>& getRecoHits() const;

    /// Get the cached data of this state
    const struct stateCache& getStateCache() const {
      return m_stateCache;
    }

  private:
    /// Precache the PXDRecoHits for runtime performance reasons.
    std::vector<PXDRecoHit> m_recoHits;
  };
}
