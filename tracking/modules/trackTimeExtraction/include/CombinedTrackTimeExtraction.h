/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreObjPtr.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/base/StoreArrayLoader.h>

#include <framework/dataobjects/EventT0.h>
#include <framework/core/Module.h>

#include <tracking/modules/trackTimeExtraction/FullGridTrackTimeExtraction.h>
#include <tracking/modules/trackTimeExtraction/TrackTimeExtraction.h>

namespace Belle2 {
  class RecoTrack;

  /**
   * Findlet to extract the global event time using the CDC drift circles information and combining
   * the faster TrackTimeExtraction and slower FullGridTrackTimeExtraction to achieve the best possible
   * result.
   * If a EvnetT0 for CDC is already available (for example from the CDC hit based method), the TrackTimeExtraction
   * is used to refine this estimate.
   * If no EventT0 is available or the TrackTimeExtraction findlet fails, the FullGridTrackTimeExtraction is used.
   * This findlet checks for a wide range of possible t0 and is therefore slow but has some chance to determine the
   * CDC t0 if all other methods failed.
   */
  class CombinedTrackTimeExtraction final : public TrackFindingCDC::Findlet<RecoTrack*> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<RecoTrack*>;

  public:
    /// Create a new instance of this module.
    CombinedTrackTimeExtraction();

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

    /// Register the store arrays and store obj pointers.
    void initialize() final override;

    /// apply the combined method of fast fitting and (if needed) full grid extraction
    void apply(std::vector<RecoTrack*>&) final override;

  private:

    StoreObjPtr<EventT0> m_eventT0;
    FullGridTrackTimeExtraction m_fullGridExtraction;
    TrackTimeExtraction m_trackTimeExtraction;

    bool m_param_useFullGridExtraction = true;
  };
}
