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

#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.dcl.h>
#include <tracking/eventTimeExtraction/findlets/GridEventTimeExtractor.dcl.h>
#include <tracking/eventTimeExtraction/findlets/DriftLengthBasedEventTimeExtractor.h>
#include <tracking/eventTimeExtraction/findlets/IterativeChi2BasedEventTimeExtractor.h>

namespace Belle2 {
  class RecoTrack;

  class FullGridDriftLengthTrackTimeExtractor final : public BaseEventTimeExtractor<RecoTrack*> {
  private:
    /// Type of the base class
    using Super = BaseEventTimeExtractor<RecoTrack*>;

  public:
    /// Constructor
    FullGridDriftLengthTrackTimeExtractor();

    /// Copy the parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

    /// Timing extraction for this findlet
    void apply(std::vector<RecoTrack*>&) override final;

  private:
    /// Slow grid based extractor
    GridEventTimeExtractor<DriftLengthBasedEventTimeExtractor> m_gridExtractor;
    /// Refining extractor in the end
    IterativeChi2BasedEventTimeExtractor m_finalExtractor;
  };
}
