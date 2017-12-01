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
#include <tracking/ckf/general/entities/CKFResult.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  /// Specialized CKF Result for extrapolating into the PXD
  class CKFToPXDResult : public CKFResult<RecoTrack, SpacePoint> {
    using Super = CKFResult<RecoTrack, SpacePoint>;
  public:
    using Super::Super;

    CKFToPXDResult(const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>& path);

    void addToRecoTrack(RecoTrack& recoTrack) const;
  };
}