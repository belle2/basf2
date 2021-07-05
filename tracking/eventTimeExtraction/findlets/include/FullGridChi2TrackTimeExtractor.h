/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.dcl.h>
#include <tracking/eventTimeExtraction/findlets/GridEventTimeExtractor.dcl.h>
#include <tracking/eventTimeExtraction/findlets/Chi2BasedEventTimeExtractor.h>
#include <tracking/eventTimeExtraction/findlets/IterativeChi2BasedEventTimeExtractor.h>

namespace Belle2 {
  class RecoTrack;

  // Guard to prevent repeated instantiations
  extern template class GridEventTimeExtractor<Chi2BasedEventTimeExtractor>;

  /// Class to extract the event t0 using the chi-squared approach
  class FullGridChi2TrackTimeExtractor final : public BaseEventTimeExtractor<RecoTrack*> {
  private:
    /// Type of the base class
    using Super = BaseEventTimeExtractor<RecoTrack*>;

  public:
    /// Constructor
    FullGridChi2TrackTimeExtractor();

    /// Copy the parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

    /// Timing extraction for this findlet
    void apply(std::vector<RecoTrack*>&) override final;

  private:
    /// Slow grid based extractor
    GridEventTimeExtractor<Chi2BasedEventTimeExtractor> m_gridExtractor;
    /// Refining extractor in the end
    IterativeChi2BasedEventTimeExtractor m_finalExtractor;
  };
}
