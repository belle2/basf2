/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/ckf/general/entities/CKFState.h>
#include <vtx/reconstruction/VTXRecoHit.h>
#include <genfit/SharedPlanePtr.h>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;
  class VTXRecoHit;

  /// Specialized CKF State for extrapolating into the VTX
  class CKFToVTXState : public CKFState<RecoTrack, SpacePoint> {
  public:
    /// When constructed by a hit, set the reco hit
    explicit CKFToVTXState(const SpacePoint* hit);

    /// Constructor setting the state to the position of the first CDC track seed hit
    explicit CKFToVTXState(const RecoTrack* seed, bool reversed = false);

    /// Extract the real layer this state sits on
    unsigned int getGeometricalLayer() const;

    /// Return the plane of the first VTX cluster
    genfit::SharedPlanePtr getPlane(const genfit::MeasuredStateOnPlane& state) const;

    /// Helper function for getting the already created reco hit (runtime reasons)
    const VTXRecoHit& getRecoHit() const;

    /// Return the related VTX track, if we go along one of them (or a nullptr)
    const RecoTrack* getRelatedVTXTrack() const;

    /// Helper function for getting the already created reco hits (runtime reasons)
    const std::vector<VTXRecoHit>& getRecoHits() const;

    /// Set the related VTX track, if we go along one of them (or a nullptr)
    void setRelatedVTXTrack(const RecoTrack* relatedVTXTrack);

    /// Get the cached data of this state
    const struct stateCache& getStateCache() const {
      return m_stateCache;
    }

  private:
    /// Precache the VTXRecoHits for runtime performance reasons.
    std::vector<VTXRecoHit> m_recoHits;

    /// The related VTX track, if we go along one of them (or a nullptr)
    const RecoTrack* m_relatedVTXTrack = nullptr;
  };
}
