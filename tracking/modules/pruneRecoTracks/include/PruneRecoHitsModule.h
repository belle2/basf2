/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth, Sasha Glazov                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>
#include <tracking/dataobjects/RecoHitInformation.h>

/**
 * This module prunes all reco hits marked as being to be pruned but PruneRecoTracksmodule
 */

namespace Belle2 {
  /// Module to prune RecoTracks.
  class PruneRecoHitsModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    PruneRecoHitsModule();

    // Declare required StoreArray
    void initialize() override;

    /// Event processing, prunes the RecoTracks contained in each event
    void event() override;

  private:

    /// We use SelectSubset here to delete all pruned RecoHitInformation.
    SelectSubset<RecoHitInformation> m_subsetOfUnprunedRecoHitInformation;

  }; // end class
} // end namespace Belle2

