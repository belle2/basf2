/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/base/StoreArrayLoader.h>

#include <tracking/modules/trackTimeExtraction/TrackTimeExtraction.h>


#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/dataobjects/EventT0.h>
#include <framework/core/Module.h>

#include <tracking/dataobjects/RecoTrack.h>


namespace Belle2 {

  /**
   * Comparable module to the FullGridTimeExtractionModule, but less well performing.
   *
   * Does also extract the event time from an array of reco tracks, but uses a simple iterative approach:
   * * Try to extract the event time from the reco tracks (by building an average over the whole array) and apply
   *   this shift to the tracks.
   * * Refit and extract again until the maximum number of tries is reached or the
   *   extraction has "converged", meaning the change is below 2 ns (the needed precision).
   * * If the extraction fails, because e.g. the fit fails, randomize the next time step to try out different
   *   starting parameters.
   */
  class TrackTimeExtractionFromRecoTracks : public TrackFindingCDC::Findlet<> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Create a new instance of the module.
    TrackTimeExtractionFromRecoTracks();

    /// Expose the parameters to a module
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

    /// Short description of the findlet
    std::string getDescription() override final;

    /// Register the store arrays and store obj pointers.
    //void initialize() override final;

    /// timing extraction for this findlet
    void apply() override final;

  private:

    TrackFindingCDC::StoreArrayLoader<const RecoTrack> m_recoTracksLoader;
    TrackTimeExtraction m_trackTimeExtraction;
  };
}
