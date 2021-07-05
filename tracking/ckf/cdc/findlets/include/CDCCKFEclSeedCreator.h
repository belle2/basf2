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

#include <tracking/ckf/cdc/entities/CDCCKFPath.h>
#include <ecl/dataobjects/ECLShower.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class RecoTrack;

  class ModuleParamList;

  /**
   * Findlet for
   */
  class CDCCKFEclSeedCreator : public TrackFindingCDC::Findlet<CDCCKFPath> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<CDCCKFPath>;

  public:
    /// Add the subfindlets
    CDCCKFEclSeedCreator();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the reco tracks and the hits
    void apply(std::vector<CDCCKFPath>& seeds) override;

  private:
    // Findlets

    // Parameters
    /// StoreArray name of the input Ecl Shower Store Array
    std::string m_param_inputEclShowerStoreArrayName = "ECLShowers";
    /// StoreArray name of the output Track Store Array.
    std::string m_param_eclSeedRecoTrackStoreArrayName = "EclSeedRecoTracks";
    /// Minimal pt requirement
    double m_param_minimalEnRequirement = 0.3;
    /// Don't do Ecl seeding in central region to save computing time
    bool m_param_restrictToForwardSeeds = true;
    /// Correction if the shower is assumed to start in a certain depth
    double m_param_showerDepth = 12.;
    /// Up to which (neg) tanLambda value should the seeding be performed
    double m_param_tanLambdaForwardNeg = -0.8;
    /// Up to which (pos) tanLambda value should the seeding be performed
    double m_param_tanLambdaForwardPos = 1.8;

    // Store Arrays
    /// Input ECL Showers Store Array
    StoreArray<ECLShower> m_inputECLshowers;
    /// Output Reco Tracks Store Array
    StoreArray<RecoTrack> m_eclSeedRecoTracks;
  };
}
