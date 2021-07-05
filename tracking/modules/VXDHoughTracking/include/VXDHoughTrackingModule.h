/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
