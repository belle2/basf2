/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <bitset>

namespace Belle2 {

  namespace PXD {
    namespace PXDError {
      /** Enums for maximum error bits, we fix to 128 as not to break data objects
       */
      enum { ONSEN_MAX_TYPE_ERR = 128};

      /** Enums for used error bits (out of maximum).
       * used for e.g. plotting ranges
       */
      enum { ONSEN_USED_TYPE_ERR = 64};

      /** Typedef the flag variable type (uint64_t)
       */
      typedef std::bitset <ONSEN_MAX_TYPE_ERR> PXDErrorFlags;

      /// Static function to return PXDError::PXDErrorFlags with no bits set (default)
      const PXDError::PXDErrorFlags getVerboseMask(void);
      /// Static function to return PXDError::PXDErrorFlags with all bits set
      const PXDError::PXDErrorFlags getSilenceMask(void);

      // TODO: reorder and remove unused, might not be backward compatible!
      /** Enum the bits we use for error flags
       * If you change this list, change the description below accordingly
       * For consistency with description, keep the order!
       */
      enum EPXDErrNr : int {
        //
        c_nrEVENT_STRUCT = 0,
        c_nrFRAME_TNR_MM = 1,
        c_nrMETA_MM = 2,
        c_nrONSEN_TRG_FIRST = 3,
        //
        c_nrDHC_END_MISS = 4,
        c_nrNR_FRAMES_TO_SMALL = 5,
        c_nrROI_PACKET_INV_SIZE = 6,
        c_nrDATA_OUTSIDE = 7,
        //
        c_nrDHC_START_SECOND = 8,
        c_nrDHE_WRONG_ID_SEQ = 9,
        c_nrFIX_SIZE = 10,
        c_nrDHE_CRC = 11,
        //
        c_nrDHC_UNKNOWN = 12,
        c_nrHEADERTYPE_INV = 13,
        c_nrPACKET_SIZE = 14,
        c_nrMAGIC = 15,
        //
        c_nrFRAME_NR = 16,
        c_nrFRAME_SIZE = 17,
        c_nrHLTROI_MAGIC = 18,
        c_nrMERGER_TRIGNR = 19,
        //
        c_nrDHP_SIZE = 20,
        c_nrDHE_DHP_DHEID = 21,
        c_nrDHE_DHP_PORT = 22,
        c_nrDHP_PIX_WO_ROW = 23,
        //
        c_nrDHE_START_END_ID = 24,
        c_nrDHE_START_ID = 25,
        c_nrDHE_START_WO_END = 26,
        c_nrNO_PXD = 27,
        //
        c_nrNO_DATCON = 28,
        c_nrFAKE_NO_DATA_TRIG = 29,
        c_nrDHE_ACTIVE = 30,
        c_nrDHP_ACTIVE = 31,
        //
        c_nrSENDALL_TYPE = 32,
        c_nrNOTSENDALL_TYPE = 33,
        c_nrDHP_DBL_HEADER = 34,
        c_nrHEADER_ERR = 35,
        //
        c_nrHEADER_ERR_GHOST = 36,
        c_nrSUSP_PADDING = 37,
        c_nrDHC_WIE = 38,
        c_nrDHE_WIE = 39,
        //
        c_nrROW_OVERFLOW = 40,
        c_nrDHP_NOT_CONT = 41,
        c_nrDHP_DHP_FRAME_DIFFER = 42,
        c_nrDHP_DHE_FRAME_DIFFER = 43,
        //
        c_nrDHE_ID_INVALID = 44,
        c_nrDHC_DHCID_START_END_MM = 45,
        c_nrDHE_END_WO_START = 46,
        c_nrDHC_END_DBL = 47,
        //
        c_nrMETA_MM_DHC = 48,
        c_nrMETA_MM_DHE = 49,
        c_nrCOL_OVERFLOW = 50,
        c_nrUNEXPECTED_FRAME_TYPE = 51,
        //
        c_nrMETA_MM_DHC_ERS = 52,
        c_nrMETA_MM_DHC_TT = 53,
        c_nrMETA_MM_ONS_HLT = 54,
        c_nrMETA_MM_ONS_DC = 55,
        //
        c_nrEVT_TRG_GATE_DIFFER = 56,
        c_nrEVT_TRG_FRM_NR_DIFFER = 57,
        c_nrDHP_ROW_WO_PIX = 58,
        c_nrDHE_START_THIRD = 59,
        //
        c_nrFAKE_NO_FAKE_DATA = 60,
        c_nrUNUSABLE_DATA = 61,
        c_nrDHH_END_ERRORBITS  = 62,
        c_nrDHH_MISC_ERROR  = 63,
        //
      };

      /** Get the description for the given error bit set above
       * @param bit The bit number, NOT the bit mask!
       * @return description as test
       */
      const std::string& getPXDBitErrorName(int bit);

    } // end namespace PXDError

  } // end namespace PXD
} // end namespace Belle2
