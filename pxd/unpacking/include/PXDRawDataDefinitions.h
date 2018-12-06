/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {

  namespace PXD {
    /** Enums for DHP data modes in the DHP header.
     * DHP modes have the same value as for the DHC/DHE frame
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    enum EDHPFrameHeaderDataType { c_RAW = 0x0, c_ZSD = 0x5};

    /** Enums for DHC data frame types.
     * 4 bits value; found in the first header word of each frame.
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    enum EDHCFrameHeaderDataType {
      c_DHP_RAW   = 0x0, // DHP memory dump ("pedestals")
      c_DHP_ZSD   = 0x5, // DHP zero supressed data
      c_FCE_RAW   = 0x1, // Clustered data
      c_COMMODE   = 0x6, // Common mode data
      c_GHOST     = 0x2, // Ghost frame, no data
      // DHE envelope
      c_DHE_START = 0x3, // DHE Start
      c_DHE_END   = 0x4, // DHE End
      // DHC envelope
      c_DHC_START = 0xB, // DHC Start
      c_DHC_END   = 0xC, // DHC End
      // Onsen processed data, new
      c_ONSEN_DHP = 0xD, // Onsen processed zero supressed DHP
      c_ONSEN_FCE = 0x9, // Onsen processed clustered
      c_ONSEN_ROI = 0xF, // Onsen ROIs (HLT+DATCON)
      c_ONSEN_TRG = 0xE,  // Trigger frame (the 1st frame)
      // Free IDs are 0x7 0x8 0xA
      c_UNUSED_7 = 0x7,
      c_UNUSED_8 = 0x8,
      c_UNUSED_A = 0xA,
    };
  }
}

