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

#include <tracking/modules/trackTimeExtraction/TrackTimeExtractionFromRecoTracks.h>

namespace Belle2 {
  /**
   * Module implementation using the CDCHitBasedT0Extraction
   */
  class TrackTimeExtractionModule : public TrackFindingCDC::FindletModule<TrackTimeExtractionFromRecoTracks> {
  public:
    /// Make destructor of interface virtual
    virtual ~TrackTimeExtractionModule() = default;
  };

}
