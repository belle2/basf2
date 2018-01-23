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

#include <tracking/modules/cdcHitBasedT0Extraction/cdcHitBasedT0Extraction.h>

namespace Belle2 {
  /**
   * Module implementation using the CDCHitBasedT0Extraction
   */
  class CDCHitBasedT0ExtractionModule : public TrackFindingCDC::FindletModule<CDCHitBasedT0Extraction> {

    /// Type of the base class
    using Super = TrackFindingCDC::FindletModule<CDCHitBasedT0Extraction>;

  public:
    /// Constructor setting the default store vector names
    CDCHitBasedT0ExtractionModule();
  };

}
