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


/**
 * Combine two collections of tracks without additional checks.
 */
namespace Belle2 {
  class RecoTrack;
  class SpacePointTrackCand;
  /// Module to combine RecoTracks.
  class RecoTrackStoreArrayCombinerModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    RecoTrackStoreArrayCombinerModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing, combine store array
    void event() override;

  private:
    /// Name of the input CDC StoreArray
    std::string m_temp1RecoTracksStoreArrayName;
    /// Name of the input VXD StoreArray
    std::string m_temp2RecoTracksStoreArrayName;
    /// Name of the input CDC SPTrackCands
    std::string m_temp1SPTrackCandsStoreArrayName = "";
    /// Name of the input VXD SPTrackCands
    std::string m_temp2SPTrackCandsStoreArrayName = "";
    /// Name of the output StoreArray
    std::string m_recoTracksStoreArrayName;

    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_temp1RecoTracks;
    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_temp2RecoTracks;
    /// Store Array of the input SPTrackCands
    StoreArray<SpacePointTrackCand> m_temp1SPTrackCands;
    /// Store Array of the input SPTrackCands
    StoreArray<SpacePointTrackCand> m_temp2SPTrackCands;
    /// Store Array of the output tracks
    StoreArray<RecoTrack> m_recoTracks;
  };
}

