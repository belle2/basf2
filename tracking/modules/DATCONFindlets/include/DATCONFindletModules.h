/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/datcon/fpgaDATCON/findlets/DATCONFPGAFindlet.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  /**
   * Combinatorical Kalman Filter used for extrapolating CDC tracks into SVD and create merged tracks.
   * All implementation is done in the corresponding findlet.
   */
  class DATCONFPGAModule : public TrackFindingCDC::FindletModule<DATCONFPGAFindlet> {

  public:
    /// Set description
    DATCONFPGAModule()
    {
      setDescription("Combinatorical Kalman Filter used for extrapolating CDC tracks into "
                     "SVD and create merged tracks.");
    }
  };

}
