/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/datastore/StoreArray.h>

#include <memory>

/**
 * Create relations between MC-matched RecoTracks in CDC and SVD.
 */
namespace Belle2 {
  /// Module to combine RecoTracks.
  class MCRelationCreatorModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    MCRelationCreatorModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing, combine store array
    void event() override;

  private:
    /// Name of the input CDC StoreArray
    std::string m_cdcRecoTracksStoreArrayName;
    /// Name of the input VXD StoreArray
    std::string m_vxdRecoTracksStoreArrayName;

    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_cdcRecoTracks;
    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_vxdRecoTracks;

    /// TrackMatchLookUp for the matching (CDC)
    std::unique_ptr<TrackMatchLookUp> m_cdcTrackMatchLookUp;
    /// TrackMatchLookUp for the matching (VXD)
    std::unique_ptr<TrackMatchLookUp> m_vxdTrackMatchLookUp;
  };
}

