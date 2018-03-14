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
#include <svd/reconstruction/SVDRecoHit.h>
#include <genfit/SharedPlanePtr.h>
#include <memory>

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
    CKFToSVDState(const RecoTrack* seed, bool reversed = false);

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

  private:
    /// Precache the PXDRecoHits for runtime performance reasons.
    std::vector<SVDRecoHit> m_recoHits;

    /// The related SVD track, if we go along one of them (or a nullptr)
    const RecoTrack* m_relatedSVDTrack = nullptr;
  };
}