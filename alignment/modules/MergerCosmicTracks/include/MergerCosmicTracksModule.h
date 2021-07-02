/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLShower.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>

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
    void MergingTracks(RecoTrack*, RecoTrack*, StoreArray<RecoTrack>&);

  private:
    /// StoreArray name from which to read the reco tracks.
    std::string m_param_recoTracksStoreArrayName = "";
    /// StoreArray name where the merged reco track is written.
    std::string m_param_mergedRecoTracksStoreArrayName = "CosmicRecoTracks";

    /// Tracks.
    StoreArray<Track> m_Tracks;

    /// RecoTracks.
    StoreArray<RecoTrack> m_RecoTracks;

    /// Merged RecoTracks.
    StoreArray<RecoTrack> m_MergedRecoTracks;

    /// ECL connected regions.
    StoreArray<ECLConnectedRegion> m_ECLConnectedRegions;

    /// KLM clusters.
    StoreArray<KLMCluster> m_KLMClusters;


    /// Flag to using magnetic field during reconstruction.
    bool m_usingMagneticField = true;
    /// Number of CDC hit per track required for cosmic track
    unsigned int m_minimumNumHitCut = 40;
    /// Minimal PXD cluster size for used PXD hits in cosmic track
    unsigned int m_minimumClusterSize = 0;
    /// Magnitude of cosmic tracks if magnetic field is not used.
    double m_magnitudeOfMomentumWithoutMagneticField = 10.;
  };
}
