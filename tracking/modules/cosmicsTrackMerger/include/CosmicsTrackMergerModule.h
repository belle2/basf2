/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/modules/cosmicsTrackMerger/CosmicsTrackMergerFindlet.h>

namespace Belle2 {
  /**
   * Module linking tracks based on their Phi parameter.
   */
  class CosmicsTrackMergerModule : public TrackFindingCDC::FindletModule<CosmicsTrackMergerFindlet> {

  public:
    /// Set description
    CosmicsTrackMergerModule()
    {
      setDescription("Module linking tracks based on their Phi parameter.");
    }
  };
}