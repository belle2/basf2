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

/**
 * Combine related tracks from CDC (determined by SVD->CDC CKF) and generic  tracks into a single track by copying the hit information
 * and combining the seed information. The sign of the weight defines, if the hits go before (-1) or after (+1)
 * the SVD track.
 */
namespace Belle2 {
  /// Module to combine RecoTracks.
  class CDCCKFTracksCombinerModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    CDCCKFTracksCombinerModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing, combine store array
    void event() override;

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

