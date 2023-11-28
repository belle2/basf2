/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dbobjects/TrackFlippingCuts.h>

/**
 * module to revert the recotracks.
 */
namespace Belle2 {
  /// Module to revert RecoTracks.
  class RecoTracksReverterModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    RecoTracksReverterModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing
    void event() override;

  private:
    /// Name of the input StoreArray
    std::string m_inputStoreArrayName;
    /// Name of the output StoreArray
    std::string m_outputStoreArrayName;
    /// the flipping cuts can be read from the DB
    OptionalDBObjPtr<TrackFlippingCuts> m_flipCutsFromDB{"TRKTrackFlipAndRefit_MVA_cuts"};
    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_inputRecoTracks;
    /// Store Array of the output tracks
    StoreArray<RecoTrack> m_outputRecoTracks;
    /// Store Array of the input tracks (for relations)
    StoreArray<Track> m_tracks;

  };
}

