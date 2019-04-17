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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <framework/datastore/StoreArray.h>

#include <tracking/ckf/cdc/entities/CDCCKFResult.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamInfo;

  /// Store resutling tracks and relations on the dataStore
  class CDCCKFResultStorer : public TrackFindingCDC::Findlet<const CDCCKFResult> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const CDCCKFResult>;

  public:
    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track/hit finding/merging.
    void apply(const std::vector<CDCCKFResult>& results) override;

    /// Register the store arrays
    void initialize() override;

  private:
    // Parameters
    /// Parameter for the distance given to the framework (can not handle EForwardBackward directly)
    std::string m_param_writeOutDirectionAsString = "both";
    /// Direction parameter converted from the string parameters
    TrackFindingCDC::EForwardBackward m_param_writeOutDirection = TrackFindingCDC::EForwardBackward::c_Unknown;
    /// Export the tracks or not
    bool m_param_exportTracks = true;
    /// StoreArray name of the output Track Store Array.
    std::string m_param_outputRecoTrackStoreArrayName = "CKFRecoTracks";
    /// StoreArray name of the tracks, the output reco tracks should be related to.
    std::string m_param_outputRelationRecoTrackStoreArrayName = "RecoTracks";
    /// Parameter for the direction of the CKF
    std::string m_param_trackFindingDirectionAsString = "forward";
    /// Direction parameter converted from the string parameters
    TrackFindingCDC::EForwardBackward m_param_trackFindingDirection = TrackFindingCDC::EForwardBackward::c_Unknown;

    // Store Arrays
    /// Output Reco Tracks Store Array
    StoreArray<RecoTrack> m_outputRecoTracks;
  };
}
