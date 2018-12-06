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

#include <tracking/ckf/general/findlets/TrackFitterAndDeleter.h>
#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class RecoTrack;

  class ModuleParamList;

  /**
   * Findlet for loading the seeds from the data store.
   * Also, the tracks are fitted and only the fittable tracks are passed on.
   *
   * If a direction != "invalid" is given, the relations of the tracks to the given store array are checked.
   * If there is a relation with the weight equal to the given direction (meaning there is already a
   * partner for this direction), the track is not passed on.
   */
  class TrackLoader : public TrackFindingCDC::Findlet<RecoTrack*> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<RecoTrack*>;

  public:
    /// Add the subfindlets
    TrackLoader();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the reco tracks and the hits
    void apply(std::vector<RecoTrack*>& seeds) override;

  private:
    // Findlets
    /// Findlet for fitting the tracks
    TrackFitterAndDeleter m_trackFitter;

    // Parameters
    /// StoreArray name of the output Track Store Array
    std::string m_param_relationRecoTrackStoreArrayName = "RecoTracks";
    /// StoreArray name of the input Track Store Array
    std::string m_param_inputRecoTrackStoreArrayName = "RecoTracks";
    /// Minimal pt requirement
    double m_param_minimalPtRequirement = 0.0;
    /// Parameter for the distance given to the framework (can not handle EForwardBackward directly)
    std::string m_param_relationCheckForDirectionAsString = "invalid";
    /// Direction parameter converted from the string parameters
    TrackFindingCDC::EForwardBackward m_param_relationCheckForDirection = TrackFindingCDC::EForwardBackward::c_Unknown;


    // Store Arrays
    /// Output Reco Tracks Store Array
    StoreArray<RecoTrack> m_inputRecoTracks;
  };
}
