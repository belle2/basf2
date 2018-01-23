/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRawDataDefinitions_H
#define PXDRawDataDefinitions_H

namespace Belle2 {

  namespace PXD {
    // DHP modes are the same as for DHE envelope
    enum EDHPFrameHeaderDataType { c_RAW = 0x0, c_ZSD = 0x5};

    // DHE like before, but now 4 bits
    enum EDHCFrameHeaderDataType {
      c_DHP_RAW   = 0x0,
      c_DHP_ZSD   = 0x5,
      c_FCE_RAW   = 0x1, //CLUSTER FRAME
      c_COMMODE   = 0x6,
      c_GHOST     = 0x2,
      c_DHE_START = 0x3,
      c_DHE_END   = 0x4,
      // DHC envelope, new
      c_DHC_START = 0xB,
      c_DHC_END   = 0xC,
      // Onsen processed data, new
      c_ONSEN_DHP = 0xD,
      c_ONSEN_FCE = 0x9,
      c_ONSEN_ROI = 0xF,
      c_ONSEN_TRG = 0xE
                    // Free IDs are 0x7 0x8 0xA
    };
  }
}

#endif
