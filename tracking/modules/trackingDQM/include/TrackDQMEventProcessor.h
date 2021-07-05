/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jachym Bartik                                            *
 *                                                                        *
 * Prepared for Tracking DQM                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMEventProcessorBase.h>
#include <tracking/modules/trackingDQM/TrackDQMModule.h>

namespace Belle2 {
  /**
   * The purpose of this class is to process one event() in TrackDQMModule.
   *
   * After instance of this class is created via constructor its only public function Run() should be called to process the event.
   *
   * This class doesn't actually fill the histograms but it calls Fill- functions on given TrackDQMModule instead.
   *
   * All functions of this class are supposed to be virtual so they can be overridden in derived classes. */
  class TrackDQMEventProcessor : public DQMEventProcessorBase {

  public:
    /** Constructor.
     * @param histoModule - DQMHistoModuleBase or derived module on which the Fill- functions are called.
     * @param recoTracksStoreArrayName - StoreArray name where the merged RecoTracks are written.
     * @param tracksStoreArrayName - StoreArray name where the merged Tracks are written. */
    TrackDQMEventProcessor(TrackDQMModule* histoModule, const std::string& recoTracksStoreArrayName,
                           const std::string& tracksStoreArrayName) :
      DQMEventProcessorBase(histoModule, recoTracksStoreArrayName, tracksStoreArrayName) { }

  protected:

  };
}
