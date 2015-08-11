/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <string>

namespace genfit {
  class AbsTrackRep;
}

namespace Belle2 {

  class RecoTrack;
  class Track;

  class TrackBuilderFromRecoTracksModule : public Module {

  public:
    /** Constructor.
     */
    TrackBuilderFromRecoTracksModule();

    /**
     * Initialize the needed StoreArrays.
     */
    void initialize() override;


    /**
     * Convert the RecoTracks to Belle2::Tracks.
     */
    void event() override;

  private:
    std::string m_param_recoTracksStoreArrayName; /**< StoreArray name of the input reco tracks */
    std::string m_param_tracksStoreArrayName; /**< StoreArray name of the output belle tracks */
    std::string m_param_trackCandidatesStoreArrayName; /**< StoreArray name of the input track candidates related to the reco tracks. */

    bool createTrackFitResult(const RecoTrack& recoTrack, Track& newBelleTrack, const genfit::AbsTrackRep* trackRep);
  };
}

