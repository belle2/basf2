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

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;

  /**
   * This findlet does also handle the storing of the results.
   */
  template <class AResult>
  class ResultStorer : public TrackFindingCDC::Findlet<AResult> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<AResult>;

  public:
    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the reco tracks and the hits
    void apply(std::vector<AResult>& results) override;

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

    // Store Arrays
    /// Output Reco Tracks Store Array
    StoreArray<RecoTrack> m_outputRecoTracks;
  };
}
