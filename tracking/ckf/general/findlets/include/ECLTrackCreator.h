/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Simon Kurz, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

//#include <analysis/ClusterUtility/ClusterUtils.h>
#include <ecl/dataobjects/ECLShower.h>
//#include <ecl/dataobjects/ECLSimHit.h>
#include <framework/datastore/StoreArray.h>
//#include <mdst/dataobjects/ECLCluster.h>

#include <string>
#include <vector>

namespace Belle2 {
  class RecoTrack;

  class ModuleParamList;

  /**
   * Findlet for
   */
  class ECLTrackCreator : public TrackFindingCDC::Findlet<RecoTrack*> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<RecoTrack*>;

  public:
    /// Add the subfindlets
    ECLTrackCreator();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the reco tracks and the hits
    void apply(std::vector<RecoTrack*>& seeds) override;

  private:
    // Findlets
    // Parameters
    /// StoreArray name of the input Ecl Shower Store Array
    std::string m_param_inputEclShowerStoreArrayName = "ECLShowers";
    /// StoreArray name of the output Track Store Array.
    std::string m_param_eclSeedRecoTrackStoreArrayName = "EclSeedRecoTracks";
    /// Minimal pt requirement
    double m_param_minimalEnRequirement = 0.0;


    // Store Arrays
    /// Input ECL Showers Store Array
    StoreArray<ECLShower> m_inputECLshowers;
    /// Output Reco Tracks Store Array
    StoreArray<RecoTrack> m_eclSeedRecoTracks;
  };
}
