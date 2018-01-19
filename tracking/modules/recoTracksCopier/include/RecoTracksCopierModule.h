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
  };
}

