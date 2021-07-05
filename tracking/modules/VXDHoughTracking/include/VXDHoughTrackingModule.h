/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/vxdHoughTracking/findlets/VXDHoughTracking.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  /**
   * Optimized DATCON that is not bound to limitations of an FPGA and thus can work as a
   * full tracking module, use genfit fitting and produce RecoTracks for proper anlysis.
   */
  class VXDHoughTrackingModule : public TrackFindingCDC::FindletModule<vxdHoughTracking::VXDHoughTracking> {

  public:
    /// Set description
    VXDHoughTrackingModule()
    {
      setDescription("Optimized DATCON that is not bound to limitations of an FPGA and thus "
                     "can work as a full tracking module, use genfit fitting and produce "
                     "RecoTracks for proper anlysis.");
    }
  };

}
