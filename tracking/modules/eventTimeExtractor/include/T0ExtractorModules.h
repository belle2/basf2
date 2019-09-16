/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractorModule.dcl.h>

#include <tracking/eventTimeExtraction/findlets/IterativeDriftLengthBasedEventTimeExtractor.h>
#include <tracking/eventTimeExtraction/findlets/IterativeChi2BasedEventTimeExtractor.h>
#include <tracking/eventTimeExtraction/findlets/HitBasedT0Extractor.h>
#include <tracking/eventTimeExtraction/findlets/FullGridChi2TrackTimeExtractor.h>
#include <tracking/eventTimeExtraction/findlets/FullGridDriftLengthTrackTimeExtractor.h>

#include <tracking/eventTimeExtraction/utilities/ClassMnemomics.h>

namespace Belle2 {
  /**
   * Module implementation using only the drift lengths.
   */
  class DriftLengthBasedT0ExtractorModule :
    public EventTimeExtractorModule<IterativeDriftLengthBasedEventTimeExtractor> {
  };

  /**
   * Module implementation using only the chi2.
   */
  class Chi2BasedT0ExtractorModule :
    public EventTimeExtractorModule<IterativeChi2BasedEventTimeExtractor> {
  };

  /**
   * Module implementation using only the chi2.
   */
  class FullGridChi2TrackTimeExtractorModule :
    public EventTimeExtractorModule<FullGridChi2TrackTimeExtractor> {
  };

  /**
   * Module implementation using the drift length and the chi2.
   */
  class FullGridDriftLengthTrackTimeExtractorModule :
    public EventTimeExtractorModule<FullGridDriftLengthTrackTimeExtractor> {
  };

  /**
   * Module implementation using the CDCHitBasedT0Extractor
   */
  class CDCHitBasedT0ExtractionModule :
    public TrackFindingCDC::FindletModule<HitBasedT0Extractor> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<HitBasedT0Extractor>;

  public:
    /// Constructor setting the default store vector names
    CDCHitBasedT0ExtractionModule();
  };


}
