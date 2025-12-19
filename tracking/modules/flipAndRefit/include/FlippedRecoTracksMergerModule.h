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

/**
 * Module doing the "merging" of the RecoTracks, Tracks and TrackFitResults
 * of the original and flipped ones.
 */
namespace Belle2 {
  class RecoTrack;
  class TrackFlippingCuts;
  class TrackFitResult;

  /// Module to merge the original and flipped RecoTracks.
  class FlippedRecoTracksMergerModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    FlippedRecoTracksMergerModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Begin run
    void beginRun() override;

    /// Event processing
    void event() override;

  private:
    /// Name of the input StoreArray
    std::string m_inputStoreArrayName;
    /// store array for the input RecoTracks
    StoreArray<RecoTrack> m_inputRecoTracks;
    /// Name of the input StoreArray for flipped tracks
    std::string m_inputStoreArrayNameFlipped;
    /// store array for the input flipped RecoTracks
    StoreArray<RecoTrack> m_inputRecoTracksFlipped;
    /// flipping cuts could be read from the DB
    DBObjPtr<TrackFlippingCuts> m_flipCutsFromDB{"TRKTrackFlipAndRefit_MVA_cuts"};
    /// StoreArray of TrackFitResult, only default name should be considered here
    StoreArray<TrackFitResult> m_trackFitResults;
  };
}

