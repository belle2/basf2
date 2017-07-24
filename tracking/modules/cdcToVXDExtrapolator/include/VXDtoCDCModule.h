/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/modules/cdcToVXDExtrapolator/VXDtoCDCFindlet.h>

namespace Belle2 {
  /**
   * Combinatorical Kalman Filter used for extracting CDC tracks into VXD (SVD) and create merged tracks.
   *
   * All implementation is done in the CDCToSpacePointCKFFindlet.
   */
  class VXDtoCDCModule : public TrackFindingCDC::FindletModule<VXDtoCDCFindlet> {

  public:
    /// Set description
    VXDtoCDCModule()
    {
      setDescription("Combinatorical Kalman Filter used for extracting CDC tracks into "
                     "VXD (SVD) and create merged tracks.");
    }
  };
}
