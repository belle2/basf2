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

    /// Parameter: Copy only tracks with good QI
    bool m_param_onlyGoodQITracks = false;

    /** Allow relations from one CDC track to m VXD tracks */
    bool m_allow_more_mulitple_relations;

    /// Parameter: QI cut value
    float m_param_qiCutValue = 0.0;
  };
}

