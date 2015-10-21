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
   * Copy all TrackCands with a given condition (see the virtual function) to the StoreArray.
   *
   * Used the input StoreArray and copies all TrackCands from this array to the output StoreArray that have a related TrackFitResult.
   * This module is just a base module - you have to provide the copyRecoTrack condition.
   * This is typically used to copy all fitted or all not fitted tracks - therefore the name.
   */
  class BaseFilterFittedTracksModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    BaseFilterFittedTracksModule();

    /** Initialize the output StoreArray. */
    void initialize() override;

    /** Copy the trackCands. */
    void event() override;

    /** You have to provide this condition. It is called for every RecoTrack in the store array. If it returns true,
     * the track gets copied into the new store array. */
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

  /** Implementation of the BaseFilterFittedTracksModule to copy all fitted tracks */
  class CopyFittedTracksModule : public BaseFilterFittedTracksModule {
    /** Implementation */
    bool copyRecoTrack(const RecoTrack& recoTrack) const override
    {
      TrackFitResult* relatedTrackFitResult = recoTrack.getRelated<TrackFitResult>("TrackFitResults");
      return relatedTrackFitResult != nullptr;
    }
  };

  /** Implementation of the BaseFilterFittedTracksModule to copy all not fitted tracks */
  class CopyNotFittedTracksModule : public BaseFilterFittedTracksModule {
    /** Implementation */
    bool copyRecoTrack(const RecoTrack& recoTrack) const override
    {
      TrackFitResult* relatedTrackFitResult = recoTrack.getRelated<TrackFitResult>("TrackFitResults");
      return relatedTrackFitResult == nullptr;
    }
  };
}
