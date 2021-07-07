/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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