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
    // Define region (phi) around seed so that additional ECLShowers are ignored (likely from Bremsstrahlung)
    double m_param_seedDeadRegionPhi = -2.0; // negative: accept all
    // Define region (theta) around seed so that additional ECLShowers are ignored (likely from Bremsstrahlung)
    double m_param_seedDeadRegionTheta = -0.1; // negative: accept all


    // Store Arrays
    /// Input ECL Showers Store Array
    StoreArray<ECLShower> m_inputECLshowers;
    /// Output Reco Tracks Store Array
    StoreArray<RecoTrack> m_eclSeedRecoTracks;
  };
}
