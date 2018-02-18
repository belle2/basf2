/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Dong Van Thanh, Jakub Kandra                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/core/Module.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  /**
   * Module use to select two cosmic tracks event and merger these two tracks become one.
   */
  class MergerCosmicTracksModule : public Module {
  public:
    /// Create a new instance of the module.
    MergerCosmicTracksModule();

    /// Register the store arrays and store obj pointers.
    void initialize() override;

    /// Do the selection.
    void event() override;

    /// Merge cosmic tracks.
    void MergingTracks(RecoTrack*, RecoTrack*, StoreArray<RecoTrack>);

  private:
    /// StoreArray name from which to read the reco tracks.
    std::string m_param_recoTracksStoreArrayName = "";
    /// StoreArray name where the merged reco track is written.
    std::string m_param_mergedRecoTracksStoreArrayName = "__MergedRecoTracks";
    /// Flag to using magnetic field during reconstruction.
    bool m_usingMagneticField = true;
    /// Number of CDC hit per track required for cosmic track
    unsigned int m_minimumNumHitCut = 40;
    /// Minimal PXD cluster size for used PXD hits in cosmic track
    unsigned int m_minimumClusterSize = 1;
    /// Magnitude of cosmic tracks if magnetic field is not used.
    double m_magnitudeOfMomentumWithoutMagneticField;
  };
}
