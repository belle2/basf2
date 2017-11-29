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
#include <tracking/trackFindingCDC/findlets/wrapper/FindletStoreArrayInput.h>

#include <framework/core/Module.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>

namespace Belle2 {
  /**
   * Module implementation using the CDCHitBasedT0Extraction
   */
  class TrackTimeExtractionModule : public //Module {
    TrackFindingCDC::FindletModule<TrackFindingCDC::FindletStoreArrayInput<TrackTimeExtraction>> {
  public:
    /// Type of the base class
    //using Super = TrackFindingCDC::FindletStoreArrayInput<TrackTimeExtraction>;

    /// Make destructor of interface virtual
    virtual ~TrackTimeExtractionModule() = default;
  };

}
