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

#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>

/**
 * Combine related tracks from CDC and VXD into a single track by copying the hit information
 * and combining the seed information. The sign of the weight defines, if the hits go before (-1) or after (+1)
 * the CDC track.
 */
namespace Belle2 {
  /// Module to combine RecoTracks.
  class RelatedTracksCombinerModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    RelatedTracksCombinerModule();

    /// Declare required StoreArray
    virtual void initialize() override;

    /// Event processing, combine store array
    virtual void event() override;

  private:
    /// Name of the input CDC StoreArray
    std::string m_cdcRecoTracksStoreArrayName;
    /// Name of the input VXD StoreArray
    std::string m_vxdRecoTracksStoreArrayName;
    /// Name of the output StoreArray
    std::string m_recoTracksStoreArrayName;

    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_cdcRecoTracks;
    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_vxdRecoTracks;
    /// Store Array of the output tracks
    StoreArray<RecoTrack> m_recoTracks;
  };
}

