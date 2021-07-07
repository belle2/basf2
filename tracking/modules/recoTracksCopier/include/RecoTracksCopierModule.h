/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

/**
 * Very simple module to copy an array of RecoTracks into another one. Copies all hits but not the fitted information.
 */
namespace Belle2 {
  /// Module to copy RecoTracks.
  class RecoTracksCopierModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    RecoTracksCopierModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing, copies store array
    void event() override;

  private:
    /// Name of the input StoreArray
    std::string m_inputStoreArrayName;
    /// Name of the output StoreArray
    std::string m_outputStoreArrayName;

    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_inputRecoTracks;
    /// Store Array of the output tracks
    StoreArray<RecoTrack> m_outputRecoTracks;
    /// Store Array of the input tracks (for relations)
    StoreArray<Track> m_tracks;

    /// Parameter: Copy only fitted tracks
    bool m_param_onlyFittedTracks = false;
  };
}

