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
   * Module to extract the global event time using the CDC drift circles information.
   *
   * The core functionality is written and documented in the TimeExtractionUtils class.
   * This module just calls the extractTime function in the utilities multiply times and decides, when the
   * time extraction is good enough to finish.
   *
   * As a result of this module, the EventT0 StoreObjPtr is set to the extracted time.
   *
   * All the extraction is done using the RecoTracks stored in the given StoreArray.
   *
   * The module works in the following way:
   * * Use 4 defined event time seeds between T0 min and T0 max. Set the event time to this seed and extract the event
   *   time using the derivatives of chi^2 calculated in the TimeExtractionUtils after arXiv:0810.2241.
   *   This shift in event time is then applied to the reco tracks and the time is extracted again.
   *   One extracted time is called "converged" or "finished", if the second derivitive of chi^2 to the event time is
   *   large (which means that the measurement uncertainty on the event time is small) and the chi^2 is small too.
   *   If the fit which is needed in each step fails, do not use this extracted time.
   *   If the extracted time is not between min T0 and max T0, do also not use this extracted time.
   *
   * * If one or more of the extraction steps from above lead to a converged result, use the one with the lowest chi^2.
   *   If not, start a loop where each extraction point from above is used, the extraction is done twice again and
   *   checked, if it has converged now.
   *
   * * If no converged point is found at all, set the EventT0 to 0.
   */
  class CombinedTrackTimeExtraction : public TrackFindingCDC::Findlet<> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Create a new instance of this module.
    CombinedTrackTimeExtraction();

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

    /// Register the store arrays and store obj pointers.
    void initialize() final override;

    void apply() final override;

    void beginEvent() final override
    {
      m_recoTracks.clear();
    }

  private:

    StoreObjPtr<EventT0> m_eventT0;
    std::vector< RecoTrack*> m_recoTracks;
    TrackFindingCDC::StoreArrayLoader< RecoTrack > m_recoTrackLoader;
    FullGridTrackTimeExtraction m_fullGridExtraction;
    TrackTimeExtraction m_trackTimeExtraction;

    bool m_param_useFullGridExtraction;
  };
}
