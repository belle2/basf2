/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>

#include <map>
#include <vector>

namespace Belle2 {
  class CKFToSVDResult;
  class RecoTrack;

  class UnusedVXDTracksAdder : public TrackFindingCDC::Findlet<CKFToSVDResult> {
    /// The parent class
    using Super = TrackFindingCDC::Findlet<CKFToSVDResult>;

  public:
    /// Expose our single argument
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /// Require the store array
    void initialize() final;

    /// Clear our flagged list
    void beginEvent() final;

    /// Add additonal results based on the VXDTF2 result
    void apply(std::vector<CKFToSVDResult>& results) final;

  private:
    /// A flagged vector with the tracks found by the VXDTF2
    std::map<const RecoTrack*, bool> m_usedVXDRecoTracks;
    /// The store array created by VXDTF2
    StoreArray <RecoTrack> m_vxdRecoTracks;
    /// The name of the store array produced by VXDTF2.
    std::string m_param_temporaryVXDTracksStoreArrayName = "VXDRecoTracks";
  };
}
