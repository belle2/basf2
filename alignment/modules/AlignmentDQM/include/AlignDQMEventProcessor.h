/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMEventProcessorBase.h>
#include <alignment/modules/AlignmentDQM/AlignDQMModule.h>

namespace Belle2 {
  /**
   * The purpose of this class is to process one event() in AlignDQMModule.
   *
   * After instance of this class is created via constructor its only public function Run() should be called to process the event.
   *
   * This class doesn't actually fill the histograms but it calls Fill- functions on given AlignDQMModule instead.
   *
   * All functions of this class are supposed to be virtual so they can be overridden in derived classes. */
  class AlignDQMEventProcessor : public DQMEventProcessorBase {

  public:
    /** Constructor.
     * @param histoModule DQMHistoModuleBase or derived module on which the Fill- functions are called.
     * @param recoTracksStoreArrayName StoreArray name where the merged RecoTracks are written.
     * @param tracksStoreArrayName StoreArray name where the merged Tracks are written. */
    AlignDQMEventProcessor(AlignDQMModule* histoModule, const std::string& tracksStoreArrayName,
                           const std::string& recoTracksStoreArrayName) :
      DQMEventProcessorBase(histoModule, recoTracksStoreArrayName, tracksStoreArrayName) { }

  protected:
    /** Fill histograms which are common for PXD and SVD hit. */
    virtual void FillCommonHistograms() override;
  };
}
