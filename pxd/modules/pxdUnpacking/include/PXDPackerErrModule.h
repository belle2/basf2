/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <pxd/dataobjects/PXDErrorFlags.h>

namespace Belle2 {

  namespace PXD {

    /** The PXDPackerErr module.
     *
     * This module is used to challenge the unpacker by creating broken events.
     * Thus allowing for ctest checking of features.
     * No actual simulated data is used as input, we create random data.
     *
     */
    class PXDPackerErrModule : public Module {
      enum {PACKER_NUM_ROWS = 768};
      enum {PACKER_NUM_COLS = 250};

      enum myerrormasks : __uint128_t {
        c_NO_ERROR = __uint128_t(0),
        c_ALL_ERROR = ~ c_NO_ERROR,
        //
        c_EVENT_STRUCT = __uint128_t(1) << Belle2::PXD::PXDError::c_nrEVENT_STRUCT,
        c_FRAME_TNR_MM = __uint128_t(1) << Belle2::PXD::PXDError::c_nrFRAME_TNR_MM,
        c_META_MM = __uint128_t(1) << Belle2::PXD::PXDError::c_nrMETA_MM,
        c_ONSEN_TRG_FIRST = __uint128_t(1) << Belle2::PXD::PXDError::c_nrONSEN_TRG_FIRST,
        //
        c_DHC_END_MISS = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHC_END_MISS,
        c_NR_FRAMES_TO_SMALL = __uint128_t(1) << Belle2::PXD::PXDError::c_nrNR_FRAMES_TO_SMALL,
        c_ROI_PACKET_INV_SIZE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrROI_PACKET_INV_SIZE,
        c_DATA_OUTSIDE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDATA_OUTSIDE,
        //
        c_DHC_START_SECOND = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHC_START_SECOND,
        c_DHE_WRONG_ID_SEQ = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_WRONG_ID_SEQ,
        c_FIX_SIZE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrFIX_SIZE,
        c_DHE_CRC = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_CRC,
        //
        c_DHC_UNKNOWN = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHC_UNKNOWN,
        c_HEADERTYPE_INV = __uint128_t(1) << Belle2::PXD::PXDError::c_nrHEADERTYPE_INV,
        c_PACKET_SIZE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrPACKET_SIZE,
        c_MAGIC = __uint128_t(1) << Belle2::PXD::PXDError::c_nrMAGIC,
        //
        c_FRAME_NR = __uint128_t(1) << Belle2::PXD::PXDError::c_nrFRAME_NR,
        c_FRAME_SIZE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrFRAME_SIZE,
        c_HLTROI_MAGIC = __uint128_t(1) << Belle2::PXD::PXDError::c_nrHLTROI_MAGIC,
        c_MERGER_TRIGNR = __uint128_t(1) << Belle2::PXD::PXDError::c_nrMERGER_TRIGNR,
        //
        c_DHP_SIZE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHP_SIZE,
        c_DHE_DHP_DHEID = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_DHP_DHEID,
        c_DHE_DHP_PORT = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_DHP_PORT,
        c_DHP_PIX_WO_ROW = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHP_PIX_WO_ROW,
        //
        c_DHE_START_END_ID = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_START_END_ID,
        c_DHE_START_ID = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_START_ID,
        c_DHE_START_WO_END = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_START_WO_END,
        c_NO_PXD = __uint128_t(1) << Belle2::PXD::PXDError::c_nrNO_PXD,
        //
        c_NO_DATCON = __uint128_t(1) << Belle2::PXD::PXDError::c_nrNO_DATCON,
        c_FAKE_NO_DATA_TRIG = __uint128_t(1) << Belle2::PXD::PXDError::c_nrFAKE_NO_DATA_TRIG,
        c_DHE_ACTIVE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_ACTIVE,
        c_DHP_ACTIVE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHP_ACTIVE,
        //
        c_SENDALL_TYPE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrSENDALL_TYPE,
        c_NOTSENDALL_TYPE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrNOTSENDALL_TYPE,
        c_DHP_DBL_HEADER = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHP_DBL_HEADER,
        c_HEADER_ERR = __uint128_t(1) << Belle2::PXD::PXDError::c_nrHEADER_ERR,
        //
        c_HEADER_ERR_GHOST = __uint128_t(1) << Belle2::PXD::PXDError::c_nrHEADER_ERR_GHOST,
        c_SUSP_PADDING = __uint128_t(1) << Belle2::PXD::PXDError::c_nrSUSP_PADDING,
        c_DHC_WIE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHC_WIE,
        c_DHE_WIE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_WIE,
        //
        c_ROW_OVERFLOW = __uint128_t(1) << Belle2::PXD::PXDError::c_nrROW_OVERFLOW,
        c_DHP_NOT_CONT = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHP_NOT_CONT,
        c_DHP_DHP_FRAME_DIFFER = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHP_DHP_FRAME_DIFFER,
        c_DHP_DHE_FRAME_DIFFER = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHP_DHE_FRAME_DIFFER,
        //
        c_DHE_ID_INVALID = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_ID_INVALID,
        c_DHC_DHCID_START_END_MM = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHC_DHCID_START_END_MM,
        c_DHE_END_WO_START = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_END_WO_START,
        c_DHC_END_DBL = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHC_END_DBL,
        //
        c_META_MM_DHC = __uint128_t(1) << Belle2::PXD::PXDError::c_nrMETA_MM_DHC,
        c_META_MM_DHE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrMETA_MM_DHE,
        c_COL_OVERFLOW = __uint128_t(1) << Belle2::PXD::PXDError::c_nrCOL_OVERFLOW,
        c_UNEXPECTED_FRAME_TYPE = __uint128_t(1) << Belle2::PXD::PXDError::c_nrUNEXPECTED_FRAME_TYPE,
        //
        c_META_MM_DHC_ERS = __uint128_t(1) << Belle2::PXD::PXDError::c_nrMETA_MM_DHC_ERS,
        c_META_MM_DHC_TT = __uint128_t(1) << Belle2::PXD::PXDError::c_nrMETA_MM_DHC_TT,
        c_META_MM_ONS_HLT = __uint128_t(1) << Belle2::PXD::PXDError::c_nrMETA_MM_ONS_HLT,
        c_META_MM_ONS_DC = __uint128_t(1) << Belle2::PXD::PXDError::c_nrMETA_MM_ONS_DC,
        //
        c_EVT_TRG_GATE_DIFFER = __uint128_t(1) << Belle2::PXD::PXDError::c_nrEVT_TRG_GATE_DIFFER,
        c_EVT_TRG_FRM_NR_DIFFER = __uint128_t(1) << Belle2::PXD::PXDError::c_nrEVT_TRG_FRM_NR_DIFFER,
        c_DHP_ROW_WO_PIX = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHP_ROW_WO_PIX,
        c_DHE_START_THIRD = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHE_START_THIRD,
        //
        c_FAKE_NO_FAKE_DATA = __uint128_t(1) << Belle2::PXD::PXDError::c_nrFAKE_NO_FAKE_DATA,
        c_UNUSABLE_DATA = __uint128_t(1) << Belle2::PXD::PXDError::c_nrUNUSABLE_DATA,
        c_DHH_END_ERRORBITS  = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHH_END_ERRORBITS ,
        c_DHH_MISC_ERROR  = __uint128_t(1) << Belle2::PXD::PXDError::c_nrDHH_MISC_ERROR ,
        //
      };

    public:
      /** Constructor defining the parameters */
      PXDPackerErrModule();

    private:
      /** Initialize the module */
      void initialize() override final;
      /** do the packing */
      void event() override final;
      /** Terminate the module */
      void terminate() override final;

      std::string m_RawPXDsName;  /**< The name of the StoreArray of generated RawPXDs */

      bool m_InvertMapping; /**< Flag if we invert mapping to DHP row/col or use premapped coordinates */
      bool m_Clusterize; /**< Use clusterizer (FCE format) */
      bool m_Check; /**< false=Pack Raw Data, true=Check unpacked result */
      std::string m_PXDDAQEvtStatsName;  /**< The name of the StoreObjPtr of PXDDAQStatus to be read */

      /** Parameter dhc<->dhe list, mapping from steering file */
      std::vector< std::vector<int >> m_dhe_to_dhc;

      /** mapping calculated from m_dhe_to_dhc for easier handling */
      std::map <int, std::vector <int>> m_dhc_mapto_dhe;

      /** Event counter */
      unsigned int m_packed_events{0};
      /** Real Trigger Nr */
      unsigned int m_real_trigger_nr{0};
      /** Trigger Nr */
      unsigned int m_trigger_nr{0};
      /** Run+Subrun Nr */
      unsigned short m_run_nr_word1{0};
      /** Exp+Run Nr */
      unsigned short m_run_nr_word2{0};
      /** Time(Tag) from MetaInfo */
      unsigned long long int m_meta_time{0};

      /** DHP Readout Frame Nr for DHP and DHE headers */
      unsigned int m_trigger_dhp_framenr{0};
      /** DHE Trigger Gate for DHE headers */
      unsigned int m_trigger_dhe_gate{0};

      /** flag if we found one test failing */
      bool m_found_fatal {false};

      /** For one DHC event, we utilize one header (writing out, beware of endianess!) */
      std::vector <unsigned int> m_onsen_header;

      /** For one DHC event, we utilize one payload for all DHE/DHP frames */
      std::vector <std::vector <unsigned char>> m_onsen_payload;

      /** For current processed frames */
      std::vector <unsigned char> m_current_frame;

      /** Output array for RawPxds */
      StoreArray<RawPXD> m_storeRaws;

      /** Output array for RawPxds */
      StoreObjPtr<PXDDAQStatus> m_daqStatus;

      /** Pack one event (several DHC) stored in seperate RawPXD object.
       */
      void pack_event(void);

      /** Pack one DHC (several DHE) stored in one RawPXD object.
       */
      void pack_dhc(int dhc_id, int dhe_mask, int* dhe_ids);

      /** Pack one DHE (several DHP) to buffer.
       */
      void pack_dhe(int dhe_id, int dhp_mask);

      /** Pack one DHP to buffer.
       */
      void pack_dhp(int dhp_id, int dhe_id, int dhe_reformat);

      /** Pack one DHP RAW to buffer.
       */
      /* cppcheck-suppress unusedPrivateFunction */
      void pack_dhp_raw(int dhp_id, int dhe_id);

      void start_frame(void); ///< Start with a new Frame
      void append_int8(unsigned char w); ///< cat 8bit value to frame
      void append_int16(unsigned short w); ///< cat 16bit value to frame
      void append_int32(unsigned int w); ///< cat 32value value to frame
      void add_frame_to_payload(void); ///< Add Frame to Event payload

      /** function still to be implemented */
      void do_the_reverse_mapping(unsigned int& row, unsigned int& col, unsigned short layer, unsigned short sensor);

      /** Check if we want this type of error in this event */
      bool isErrorIn(uint32_t enr);

      /** Store start of Vxd Detector related digits */
      std::map <VxdID , int> startOfVxdID;

      /** temporary hitmap buffer for pixel to raw data conversion */
      unsigned char halfladder_pixmap[PACKER_NUM_ROWS][PACKER_NUM_COLS] = {0};

      unsigned int dhe_byte_count{0}; /**< Byte count in current DHE package */
      unsigned int dhc_byte_count{0}; /**< Byte count in current DHC package */

      bool CheckErrorMaskInEvent(unsigned int eventnr, PXDError::PXDErrorFlags mask);

      /// The pxd error flags
      static std::vector <PXDErrorFlags> m_errors;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
