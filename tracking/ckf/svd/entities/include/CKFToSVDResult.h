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
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  /// Specialized CKF Result for extrapolating into the SVD
  class CKFToSVDResult : public CKFResult<RecoTrack, SpacePoint> {
    using Super = CKFResult<RecoTrack, SpacePoint>;
  public:
    using Super::Super;

    CKFToSVDResult(const std::vector<TrackFindingCDC::WithWeight<const CKFToSVDState*>>& path);

    void addToRecoTrack(RecoTrack& recoTrack) const;
  };
}