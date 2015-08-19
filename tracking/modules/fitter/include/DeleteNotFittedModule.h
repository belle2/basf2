/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {

  class TrackFitResult;

  /**
   * Copy all TrackCands with a TrackFitResult to the StoreArray
   *
   * Used the input StoreArray and copies all TrackCands from this array to the output StoreArray that have a related TrackFitResult.
   *
   */
  class BaseFilterFittedTracksModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    BaseFilterFittedTracksModule();

    /** Initialize the output StoreArray */
    void initialize() override;

    /** Copy the trackCands. */
    void event() override;

    virtual bool copyRecoTrack(const RecoTrack& recoTrack) const = 0;

  private:

    /** Input StoreArray name for the TrackCands. */
    std::string m_param_inputStoreArrayNameForTrackCands;
    /** Input StoreArray name for the RecoTracks. */
    std::string m_param_inputStoreArrayNameForRecoTracks;
    /** Output StoreArray name for the TrackCands. */
    std::string m_param_outputStoreArrayNameForTrackCands;
    /** Output StoreArray name for the RecoTracks. */
    std::string m_param_outputStoreArrayNameForRecoTracks;
  };

  class CopyFittedTracksModule : public BaseFilterFittedTracksModule {
    bool copyRecoTrack(const RecoTrack& recoTrack) const override
    {
      TrackFitResult* relatedTrackFitResult = recoTrack.getRelated<TrackFitResult>("TrackFitResults");
      return relatedTrackFitResult != nullptr;
    }
  };

  class CopyNotFittedTracksModule : public BaseFilterFittedTracksModule {
    bool copyRecoTrack(const RecoTrack& recoTrack) const override
    {
      TrackFitResult* relatedTrackFitResult = recoTrack.getRelated<TrackFitResult>("TrackFitResults");
      return relatedTrackFitResult == nullptr;
    }
  };
}
