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
#include <tracking/datcon/optimizedDATCON/findlets/DATCONFindlet.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  /**
   * DATCON module which implementation is close to the one actually used on the FPGA.
   */
  class DATCONFPGAModule : public TrackFindingCDC::FindletModule<DATCONFPGAFindlet> {

  public:
    /// Set description
    DATCONFPGAModule()
    {
      setDescription("DATCON module which implementation is close to the one actually used "
                     "on the FPGA. This requires some simplifications, as well as tweaks as "
                     "the FPGA only can deal with integers, so as far as possible or reasonable "
                     "numbers in this module are represented as int / long instead of double.");
    }
  };

  /**
   * Optimized DATCON that is not bound to limitations of an FPGA and thus can work as a
   * full tracking module, use genfit fitting and produce RecoTracks for proper anlysis.
   */
  class DATCONModule : public TrackFindingCDC::FindletModule<DATCONFindlet> {

  public:
    /// Set description
    DATCONModule()
    {
      setDescription("Optimized DATCON that is not bound to limitations of an FPGA and thus "
                     "can work as a full tracking module, use genfit fitting and produce "
                     "RecoTracks for proper anlysis.");
    }
  };

}
