/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    /** Enums for DHE DHP StateMachine Error States.
     * 4 bits value;
     * Currently the same definitions as the ones encoded in ghost frame (new! but 3 bit only)
     * See Data format definitions [BELLE2-NOTE-TE-2016-009] on https://docs.belle2.org/
     */
    enum EDHEStateMachineError {
      c_DHESM_NO_ERROR = 0x0, // no errors
      c_DHESM_MISS_DHP_FRM = 0x1, // missing DHPT frame
      c_DHESM_TIMEOUT = 0x2, // timeout
      c_DHESM_DHP_LINKDOWN = 0x3, // DHP link down
      c_DHESM_DHP_MASKED = 0x4, // DHP masked
      c_DHESM_EVTNR_MM = 0x5, // event number mismatch (between FSM_START and FSM_END; DHE error.)
      c_DHESM_DHP_SIZE_OVERFLOW = 0x6, // DHPT frame bigger than maximum allowed, data truncated
      /// others currently not defined
    };

  }
}

