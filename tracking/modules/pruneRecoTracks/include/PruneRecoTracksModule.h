/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>
#include <tracking/dataobjects/RecoHitInformation.h>

/**
 * This module calls RecoTrack::prune to remove hits before
 * the track gets stored to disk to save storage space.
 * By default, this module will remove all but the first and last hits of
 * the track.
 */

namespace Belle2 {
  /// Module to prune RecoTracks.
  class PruneRecoTracksModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    PruneRecoTracksModule();

    // Declare required StoreArray
    void initialize() override;

    /// Event processing, prunes the RecoTracks contained in each event
    void event() override;

  private:
    /// Name of the StoreArray to prune.
    std::string m_storeArrayName = "RecoTracks";

    /// We use SelectSubset here to delete all pruned RecoHitInformation.
    SelectSubset<RecoHitInformation> m_subsetOfUnprunedRecoHitInformation;

    /// Remove pruned reco hits or leave this for PruneRecoHitsModule
    bool m_do_remove_hits{false};

  }; // end class
} // end namespace Belle2

