/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/ckf/general/entities/CKFResult.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  /// Specialized CKF Result for extrapolating into the SVD
  class CKFToSVDResult : public CKFResult<RecoTrack, SpacePoint> {
    /// The parent class
    using Super = CKFResult<RecoTrack, SpacePoint>;

  public:
    /// Constructor using a path.
    explicit CKFToSVDResult(const std::vector<TrackFindingCDC::WithWeight<const CKFToSVDState*>>& path);

    /// Called in the exporter findlet for adding this to a already created reco track.
    void addToRecoTrack(RecoTrack& recoTrack) const;

    /// Return the related SVD track, if we go along one of them.
    const RecoTrack* getRelatedSVDRecoTrack() const;

  private:
    /// The related SVD track, if we go along one of them (or a nullptr)
    const RecoTrack* m_relatedSVDRecoTrack = nullptr;
  };
}
