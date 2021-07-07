/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/ckf/general/entities/CKFState.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <genfit/SharedPlanePtr.h>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;
  class SVDRecoHit;

  /// Specialized CKF State for extrapolating into the SVD
  class CKFToSVDState : public CKFState<RecoTrack, SpacePoint> {
  public:
    /// When constructed by a hit, set the reco hit
    explicit CKFToSVDState(const SpacePoint* hit);

    /// Constructor setting the state to the position of the first CDC track seed hit
    explicit CKFToSVDState(const RecoTrack* seed, bool reversed = false);

    /// Extract the real layer this state sits on
    unsigned int getGeometricalLayer() const;

    /// Return the plane of the first SVD cluster
    genfit::SharedPlanePtr getPlane(const genfit::MeasuredStateOnPlane& state) const;

    /// Helper function for getting the already created reco hit (runtime reasons)
    const SVDRecoHit& getRecoHit() const;

    /// Helper function for getting the already created reco hits (runtime reasons)
    const std::vector<SVDRecoHit>& getRecoHits() const;

    /// Return the related SVD track, if we go along one of them (or a nullptr)
    const RecoTrack* getRelatedSVDTrack() const;

    /// Set the related SVD track, if we go along one of them (or a nullptr)
    void setRelatedSVDTrack(const RecoTrack* relatedSVDTrack);

    /// Get the cached data of this state
    const struct stateCache& getStateCache() const {
      return m_stateCache;
    }

  private:
    /// Precache the PXDRecoHits for runtime performance reasons.
    std::vector<SVDRecoHit> m_recoHits;

    /// The related SVD track, if we go along one of them (or a nullptr)
    const RecoTrack* m_relatedSVDTrack = nullptr;
  };
}
