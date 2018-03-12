/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 * Created: 23.10.2017                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>

namespace Belle2 {

  namespace PXD {
    namespace PXDError {
      /** Enums for maximum error bits, 64bit mask used (uint64_t)
       */
      enum { ONSEN_MAX_TYPE_ERR = 64};

      /** Enums for used error bits (out of maximum).
       * used for e.g. plotting ranges
       */
      enum { ONSEN_USED_TYPE_ERR = 60};

      /** Typedef the flag variable type (uint64_t)
       */
      typedef uint64_t PXDErrorFlags;

      /** Enum the bits we use for error flags
       * If you change this list, change the description below accordingly
       * For consistency with description, keep the order!
       */
      enum EPXDErrMask : uint64_t {
        c_NO_ERROR = 0ull,
        //
        c_TB_IDS = 1ull << 0,
        c_FRAME_TNR_MM = 1ull << 1,
        c_META_MM = 1ull << 2,
        c_ONSEN_TRG_FIRST = 1ull << 3,
        //
        c_DHC_END_MISS = 1ull << 4,
        c_DHE_START_MISS = 1ull << 5,
        c_ROI_PACKET_INV_SIZE = 1ull << 6,
        c_DATA_OUTSIDE = 1ull << 7,
        //
        c_DHC_START_SECOND = 1ull << 8,
        c_DHE_WRONG_ID_SEQ = 1ull << 9,
        c_FIX_SIZE = 1ull << 10,
        c_DHE_CRC = 1ull << 11,
        //
        c_DHC_UNKNOWN = 1ull << 12,
        c_MERGER_CRC = 1ull << 13,
        c_PACKET_SIZE = 1ull << 14,
        c_MAGIC = 1ull << 15,
        //
        c_FRAME_NR = 1ull << 16,
        c_FRAME_SIZE = 1ull << 17,
        c_HLTROI_MAGIC = 1ull << 18,
        c_MERGER_TRIGNR = 1ull << 19,
        //
        c_DHP_SIZE = 1ull << 20,
        c_DHE_DHP_DHEID = 1ull << 21,
        c_DHE_DHP_PORT = 1ull << 22,
        c_DHP_PIX_WO_ROW = 1ull << 23,
        //
        c_DHE_START_END_ID = 1ull << 24,
        c_DHE_START_ID = 1ull << 25,
        c_DHE_START_WO_END = 1ull << 26,
        c_NO_PXD = 1ull << 27,
        //
        c_NO_DATCON = 1ull << 28,
        c_FAKE_NO_DATA_TRIG = 1ull << 29,
        c_DHE_ACTIVE = 1ull << 30,
        c_DHP_ACTIVE = 1ull << 31,
        //
        c_SENDALL_TYPE = 1ull << 32,
        c_NOTSENDALL_TYPE = 1ull << 33,
        c_DHP_DBL_HEADER = 1ull << 34,
        c_HEADER_ERR = 1ull << 35,
        //
        c_HEADER_ERR_GHOST = 1ull << 36,
        c_SUSP_PADDING = 1ull << 37,
        c_DHC_WIE = 1ull << 38,
        c_DHE_WIE = 1ull << 39,
        //
        c_ROW_OVERFLOW = 1ull << 40,
        c_DHP_NOT_CONT = 1ull << 41,
        c_DHP_DHP_FRAME_DIFFER = 1ull << 42,
        c_DHP_DHE_FRAME_DIFFER = 1ull << 43,
        //
        c_DHE_ID_INVALID = 1ull << 44,
        c_DHC_DHCID_START_END_MM = 1ull << 45,
        c_DHE_END_WO_START = 1ull << 46,
        c_DHC_END_DBL = 1ull << 47,
        //
        c_META_MM_DHC = 1ull << 48,
        c_META_MM_DHE = 1ull << 49,
        c_COL_OVERFLOW = 1ull << 50,
        c_UNEXPECTED_FRAME_TYPE = 1ull << 51,
        //
        c_META_MM_DHC_ERS = 1ull << 52,
        c_META_MM_DHC_TT = 1ull << 53,
        c_META_MM_ONS_HLT = 1ull << 54,
        c_META_MM_ONS_DC = 1ull << 55,
        //
        c_EVT_TRG_GATE_DIFFER = 1ull << 56,
        c_EVT_TRG_FRM_NR_DIFFER = 1ull << 57,
        c_DHP_ROW_WO_PIX = 1ull << 58,
        c_DHE_START_THIRD = 1ull << 59,
        //
        // 60, 61, 62, 63 unused
      };

      /** Get the description for the given error bit set above
       * @param bit The bit number, NOT the bit mask!
       * @return description as test
       */
      const std::string& getPXDBitErrorName(int bit);

      /** Get the description for the given error bit-mask set above
       * @param mask The bit mask, NOT the bit number!
       * @return description as test
       */
      const std::string& getPXDBitMaskErrorName(EPXDErrMask mask);

    } // end namespace PXDError

  } // end namespace PXD
} // end namespace Belle2
