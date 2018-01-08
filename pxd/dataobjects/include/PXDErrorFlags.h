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
      enum { ONSEN_MAX_TYPE_ERR = 64}; // 64 bit mask

      typedef uint64_t PXDErrorFlags;

      /// If you change this list, change the NAMEs in the terminate function, too
      enum EPXDErrMask : uint64_t {
        c_NO_ERROR = 0ull,
        c_TB_IDS = 1ull << 0,
        c_FRAME_TNR_MM = 1ull << 1,
        c_META_MM = 1ull << 2,
        c_ONSEN_TRG_FIRST = 1ull << 3,
        c_DHC_END_MISS = 1ull << 4,
        c_DHE_START_MISS = 1ull << 5,
        c_DHC_FRAMECOUNT = 1ull << 6,
        c_DATA_OUTSIDE = 1ull << 7,
        c_DHC_START_SECOND = 1ull << 8,
        c_DHE_WRONG_ID_SEQ = 1ull << 9,
        c_FIX_SIZE = 1ull << 10,
        c_DHE_CRC = 1ull << 11,
        c_DHC_UNKNOWN = 1ull << 12,
        c_MERGER_CRC = 1ull << 13,
        c_PACKET_SIZE = 1ull << 14,
        c_MAGIC = 1ull << 15,
        c_FRAME_NR = 1ull << 16,
        c_FRAME_SIZE = 1ull << 17,
        c_HLTROI_MAGIC = 1ull << 18,
        c_MERGER_TRIGNR = 1ull << 19,
        c_DHP_SIZE = 1ull << 20,
        c_DHE_DHP_DHEID = 1ull << 21,
        c_DHE_DHP_PORT = 1ull << 22,
        c_DHP_PIX_WO_ROW = 1ull << 23,
        c_DHE_START_END_ID = 1ull << 24,
        c_DHE_START_ID = 1ull << 25,
        c_DHE_START_WO_END = 1ull << 26,
        c_NO_PXD = 1ull << 27,
        c_NO_DATCON = 1ull << 28,
        c_FAKE_NO_DATA_TRIG = 1ull << 29,
        c_DHE_ACTIVE = 1ull << 30,
        c_DHP_ACTIVE = 1ull << 31,
        c_SENDALL_TYPE = 1ull << 32,
        c_NOTSENDALL_TYPE = 1ull << 33,
        c_DHP_DBL_HEADER = 1ull << 34,
        c_HEADER_ERR = 1ull << 35,
        c_HEADER_ERR_GHOST = 1ull << 36,
        c_SUSP_PADDING = 1ull << 37,
        c_DHC_WIE = 1ull << 38,
        c_DHE_WIE = 1ull << 39,
        c_ROW_OVERFLOW = 1ull << 40,
        c_DHP_NOT_CONT = 1ull << 40,
        c_DHP_DHP_FRAME_DIFFER = 1ull << 42,
        c_DHP_DHE_FRAME_DIFFER = 1ull << 43,
        c_DHE_ID_INVALID = 1ull << 44,
        c_DHC_DHCID_START_END_MM = 1ull << 45,
        c_DHE_END_WO_START = 1ull << 46,
        c_DHC_END_DBL = 1ull << 47
      };

      const std::string& getPXDBitErrorName(int bit)
      {
        static const std::string undefined("undefined");
        static const std::string error_name[ONSEN_MAX_TYPE_ERR] = {
          "TestBeam: Unknown DHE IDs", "Frame TrigNr vs ONSEN Trigger Frame mismatch", "Frame TrigNr vs EvtMeta mismatch", "ONSEN Trigger is not first frame",
          "DHC_END missing/wrong placed", "DHE_START missing", "DHC Framecount mismatch", "DATA outside of DHE",
          "DHC_START is not second frame", "DHE IDs have wrong order in DHC", "Fixed size frame wrong size", "DHE CRC Error:",
          "Unknown DHC type", "Merger CRC Error", "Event Header Full Packet Size Error", "Event Header Magic Error",
          "Event Header Frame Count Error", "Event header Frame Size Error", "HLTROI Magic Error", "Merger HLT/DATCON TrigNr Mismatch",
          "DHP Size too small", "DHP-DHE DHEID mismatch", "DHP-DHE Port mismatch", "DHP Pix w/o row",
          "DHE START/END ID mismatch", "DHE ID mismatch of START and this frame", "DHE_START w/o prev END", "Nr PXD data ==0",
          "Missing Datcon", "NO DHC data for Trigger", "DHE active mismatch", "DHP active mismatch",

          "SendUnfiltered but Filtered Frame Type", "!SendUnfiltered but Unfiltered Frame Type", "DHP has double header", "Error bit in frame header set",
          "Error bit in GHOST frame header not set", "Suspicious Padding/Checksum in DHP ZSP", "DHC Words in Event mismatch", "DHH Words in Event mismatch",
          "Row Overflow/out of bounds >=768", "DHP Frames not continuesly", "DHP Frame numbers of different frames differ>1", "DHP Frame differ from DHE Frame by >1",
          "DHE ID is invalid", "DHC ID Start/End Mismatch", "DHE End w/o Start", "double DHC End",

          "unused", "unused", "unused", "unused",
          "unused", "unused", "unused", "unused",
          "unused", "unused", "unused", "unused",
          "unused", "unused", "unused", "unused"
        };
        if (bit < 0 || bit >= ONSEN_MAX_TYPE_ERR) return undefined;
        return error_name[bit];
      }
    } // end namespace PXDError

  } // end namespace PXD
} // end namespace Belle2
