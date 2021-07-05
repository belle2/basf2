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
#include <tracking/datcon/findlets/DATCONFPGAFindlet.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  /**
   * DATCON module which implementation is close to the one actually used on the FPGA.
   */
  class DATCONModule : public TrackFindingCDC::FindletModule<DATCONFPGAFindlet> {

  public:
    /// Set description
    DATCONModule()
    {
      setDescription("DATCON module which implementation is close to the one actually used "
                     "on the FPGA. This requires some simplifications, as well as tweaks as "
                     "the FPGA only can deal with integers, so as far as possible or reasonable "
                     "numbers in this module are represented as int / long instead of double.");
    }
  };

}
