/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/ckf/general/entities/CKFResult.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  /// Specialized CKF Result for extrapolating into the PXD
  class CKFToPXDResult : public CKFResult<RecoTrack, SpacePoint> {
    /// The parent class
    using Super = CKFResult<RecoTrack, SpacePoint>;
  public:
    /// Constructor using a path.
    explicit CKFToPXDResult(const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>& path);

    /// Called in the exporter findlet for adding this to a already created reco track.
    void addToRecoTrack(RecoTrack& recoTrack) const;
  };
}
