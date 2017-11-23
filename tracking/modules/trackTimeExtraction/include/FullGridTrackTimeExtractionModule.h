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

#include <tracking/modules/trackTimeExtraction/FullGridTrackTimeExtraction.h>

namespace Belle2 {
  /**
   * Module implementation using the CDCHitBasedT0Extraction
   */
  class FullGridTrackTimeExtractionModule : public TrackFindingCDC::FindletModule<FullGridTrackTimeExtraction> {
  public:
    /// Make destructor of interface virtual
    virtual ~FullGridTrackTimeExtractionModule() = default;
  };

}
