/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

#include <tracking/modules/trackTimeExtraction/FullGridTrackTimeExtraction.h>
#include <tracking/trackFindingCDC/findlets/wrapper/FindletStoreArrayInput.h>
#include <tracking/ckf/general/utilities/ClassMnemomics.h>

namespace Belle2 {
  /**
   * Module implementation using only the FullGridTrackTimeExtraction Findlet. Look a the
   * documentation of the FullGridTrackTimeExtraction class for further details.
   */
  class FullGridTrackTimeExtractionModule final : public
    TrackFindingCDC::FindletModule<TrackFindingCDC::FindletStoreArrayInput<FullGridTrackTimeExtraction>> {
  };

}
