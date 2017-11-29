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

#include <tracking/modules/trackTimeExtraction/CombinedTrackTimeExtraction.h>
#include <tracking/trackFindingCDC/findlets/wrapper/FindletStoreArrayInput.h>
#include <tracking/ckf/general/utilities/ClassMnemomics.h>

namespace Belle2 {
  /**
   * Module implementation using the CDCHitBasedT0Extraction
   */
  class CombinedTrackTimeExtractionModule : public
    TrackFindingCDC::FindletModule<CombinedTrackTimeExtraction> {
  public:
    /// Make destructor of interface virtual
    virtual ~CombinedTrackTimeExtractionModule() = default;
  };

}
