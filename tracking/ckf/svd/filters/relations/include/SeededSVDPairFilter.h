/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/svd/filters/relations/BaseSVDPairFilter.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

namespace Belle2 {
  /// Filter for CKF SVD state relations knowing about the VXD tracks
  class SeededSVDPairFilter : public BaseSVDPairFilter {
  public:
    TrackFindingCDC::Weight operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation) override;

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    std::string m_param_vxdTracksStoreArrayName = "VXDRecoTracks";
    unsigned int m_param_maximalNumberOfDifferentRecoTracks = 0;
  };
}
