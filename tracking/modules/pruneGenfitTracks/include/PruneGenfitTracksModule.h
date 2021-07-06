/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

/**
 * This module calls genfit::Track::prune to remove hits before
 * the track gets stored to disk to save storage space.
 * By default, this module will remove all but the first and last hits of
 * the track.
 */

namespace Belle2 {

  /// Module to prune genfit::Tracks
  class PruneGenfitTracksModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    PruneGenfitTracksModule();

    /// Event processing, prunes the genfit tracks contained in each event
    void event() override;

  private:

    /// flags used for pruning, See genfit::Track class for possible settings
    std::string m_pruneFlags;

    /// name of the StoreArray to prune
    std::string m_storeArrayName;

  }; // end class
} // end namespace Belle2

