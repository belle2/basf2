/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/unpacking/PXDRawDataStructs.h>
#include <pxd/unpacking/PXDRawDataDefinitions.h>
#include <framework/logging/Logger.h>

#include <boost/crc.hpp>


///*********************************************************************************
///****************** DHC Data Frame structs starts here ***************************
///*********************************************************************************

namespace Belle2 {

  namespace PXD {

    /// define our CRC function
    typedef boost::crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhc_crc_32_type;

    using ubig16_t = boost::endian::big_uint16_t;
    using ubig32_t = boost::endian::big_uint32_t;
    using namespace Belle2::PXD::PXDError;
    using Belle2::PXD::EDHCFrameHeaderDataType;

    void dhc_frame_header_word0::print(void) const
    {
      /* cppcheck-suppress variableScope */
      const char* dhc_type_name[16] = {
        (const char*)"DHP_RAW",
        (const char*)"FCE_RAW",
        (const char*)"GHOST  ",
        (const char*)"H_START",
        (const char*)"H_END  ",
        (const char*)"DHP_ZSD",
        (const char*)"COMMODE",
        (const char*)"undef  ",
        (const char*)"undef  ",
        (const char*)"ONS_FCE",
        (const char*)"undef  ",
        (const char*)"C_START",
        (const char*)"C_END  ",
        (const char*)"ONS_DHP",
        (const char*)"ONS_TRG",
        (const char*)"ONS_ROI"
      };
      B2DEBUG(99, "DHC FRAME TYP $" << std::hex << getFrameType() << " -> " << dhc_type_name[getFrameType()] << " (ERR " << getErrorFlag()
              << ") data " << data);
    };

    bool dhc_start_frame::isFakedData(void) const
    {
      if (word0.data != 0x5800) return false;
      if (trigger_nr_lo != 0) return false;
      if (trigger_nr_hi != 0) return false;
      if (time_tag_lo_and_type != 0) return false;
      if (time_tag_mid != 0) return false;
      if (time_tag_hi != 0) return false;
      if (run_subrun != 0) return false;
      if (exp_run != 0) return false;
      if (crc32 != 0x4829214d) return false;
      return true;
    };

    void dhc_start_frame::print(void) const
    {
      word0.print();
      B2DEBUG(99, "DHC Start Frame TNRLO $" << std::hex << trigger_nr_lo << " TNRHI $" << std::hex << trigger_nr_hi << " TTLO $" <<
              std::hex <<
              time_tag_lo_and_type
              << " TTMID $" << std::hex << time_tag_mid << " TTHI $" << std::hex << time_tag_hi << " Exp/Run/Subrun $" << std::hex << exp_run <<
              " $" <<
              run_subrun
              << " CRC $" << std::hex << crc32);
    };

    void dhc_dhe_start_frame::print(void) const
    {
      word0.print();
      B2DEBUG(99, "DHC Event Frame TNRLO $" << std::hex << trigger_nr_lo  << " DTTLO $" << std::hex << dhe_time_tag_lo << " DTTHI $" <<
              std::hex <<
              dhe_time_tag_hi
              << " DHEID $" << std::hex << getDHEId()
              << " DHPMASK $" << std::hex << getActiveDHPMask()
              << " SFNR $" << std::hex << getStartFrameNr()
              << " OFF $" << std::hex << getTriggerGate()
              << " CRC " << std::hex << crc32);
    };

    void dhc_direct_readout_frame::print(void) const
    {
      word0.print();
      B2DEBUG(99, "DHC Direct Readout (Raw|ZSD|ONS) Frame TNRLO $" << std::hex << trigger_nr_lo << " DHE ID $" << getDHEId() <<
              " DHP port $" << getDHPPort());
    };

    void dhc_onsen_trigger_frame::print(void) const
    {
      word0.print();
      B2DEBUG(99, "ONSEN Trigger Frame TNRLO $" << std::hex << trignr0);
    };

    PXDError::PXDErrorFlags dhc_onsen_trigger_frame::check_error(bool ignore_datcon_flag, bool ignore_hltroi_magic_flag,
        bool ignore_merger_mm_flag) const
    {
      PXDError::PXDErrorFlags m_errorMask = PXDError::EPXDErrMask::c_NO_ERROR;
      if ((magic1 & 0xFFFF0000) != 0xCAFE0000) {
        if (!ignore_hltroi_magic_flag) B2WARNING("ONSEN Trigger Magic 1 error $" << std::hex << magic1);
        m_errorMask |= PXDError::EPXDErrMask::c_HLTROI_MAGIC;
      }
      if ((magic2 & 0xFFFF0000) != 0xCAFE0000) {
        if (!ignore_hltroi_magic_flag) B2WARNING("ONSEN Trigger Magic 2 error $" << std::hex << magic2);
        m_errorMask |= PXDError::EPXDErrMask::c_HLTROI_MAGIC;
      }
      if (is_fake_datcon()) {
        if (!ignore_datcon_flag) B2INFO("ONSEN Trigger Frame: No DATCON data $" << std::hex << trignr1 << "!=$" << trignr2);
        m_errorMask |= PXDError::EPXDErrMask::c_NO_DATCON;
      } else {
        if (trignr1 != trignr2) {
          if (!ignore_merger_mm_flag) B2WARNING("ONSEN Trigger Frame Trigger Nr Mismatch $" << std::hex << trignr1 << "!=$" << trignr2);
          m_errorMask |= PXDError::EPXDErrMask::c_MERGER_TRIGNR;
        }
      }
      return m_errorMask;
    };

    PXDError::PXDErrorFlags dhc_onsen_roi_frame::check_error(int length, bool ignore_inv_size_flag) const
    {
      PXDError::PXDErrorFlags m_errorMask = PXDError::EPXDErrMask::c_NO_ERROR;
      // 4 byte header, ROIS (n*8), 4 byte copy of inner CRC, 4 byte outer CRC
      if (length < getMinSize()) {
        if (!ignore_inv_size_flag) B2WARNING("DHC ONSEN HLT/ROI Frame too small to hold any ROIs!");
        m_errorMask |= PXDError::c_ROI_PACKET_INV_SIZE;
      } else if ((length - getMinSize()) % 8 != 0) {
        if (!ignore_inv_size_flag) B2WARNING("DHC ONSEN HLT/ROI Frame holds fractional ROIs, last ROI might not be saved!");
        m_errorMask |= PXDError::c_ROI_PACKET_INV_SIZE;
      }
      return m_errorMask;
    };
    void dhc_onsen_roi_frame::print(void) const
    {
      word0.print();
      B2DEBUG(99, "DHC HLT/ROI Frame");
    };

    void dhc_ghost_frame::print(void) const
    {
      word0.print();
      B2DEBUG(99, "DHC Ghost Frame TNRLO $" << std::hex << trigger_nr_lo << " DHE ID $" << getDHEId() << " DHP port $" << getDHPPort() <<
              " CRC $");
    };

    unsigned short dhc_ghost_frame::getErrorBits(void) const
    {
      unsigned short value = (word0.data & 0xC) >> 2;  // lower two bits
      if (word0.data & 0x0400) value |= 0x4; // high bit
      return value;
    };

    bool dhc_end_frame::isFakedData(void) const
    {
      if (word0.data != 0x6000) return false;
      if (trigger_nr_lo != 0) return false;
      if (wordsinevent != 0) return false;
      if (errorinfo != 0) return false;
      if (crc32 != 0xF7BCA507) return false;
      return true;
    };
    void dhc_end_frame::print(void) const
    {
      word0.print();
      B2DEBUG(99, "DHC End Frame TNRLO $" << std::hex << trigger_nr_lo << " WIEVT $" << std::hex << wordsinevent << " ERR $" << std::hex
              << errorinfo << " CRC " << std::hex << crc32);
    };


    void dhc_dhe_end_frame::print(void) const
    {
      word0.print();
      B2DEBUG(99, "DHC DHE End Frame TNRLO $" << std::hex << trigger_nr_lo << " WIEVT $" << std::hex << wordsineventhi << "." <<
              wordsineventlo << " ERR $" << std::hex << errorinfo << " CRC " << std::hex << crc32);
    };

    unsigned int dhc_dhe_end_frame::getErrorStateMachineDHP(int dhpid) const
    {
      switch (dhpid) {
        case 0: return ((uint32_t)errorinfo >> 24) & 0xFF;
        case 1: return ((uint32_t)errorinfo >> 16) & 0xFF;
        case 2: return ((uint32_t)errorinfo >>  8) & 0xFF;
        case 3: return errorinfo & 0xFF;
        default: return 0;
      }
    };

    unsigned int dhc_dhe_end_frame::getErrorStateMachineStartDHP(int dhpid) const
    {
      switch (dhpid) {
        case 0: return ((uint32_t)errorinfo >> 24) & 0xF;
        case 1: return ((uint32_t)errorinfo >> 16) & 0xF;
        case 2: return ((uint32_t)errorinfo >>  8) & 0xF;
        case 3: return errorinfo & 0xF;
        default: return 0;
      }
    };

    unsigned int dhc_dhe_end_frame::getErrorStateMachineEndDHP(int dhpid) const
    {
      switch (dhpid) {
        case 0: return ((uint32_t)errorinfo >> 28) & 0xF;
        case 1: return ((uint32_t)errorinfo >> 20) & 0xF;
        case 2: return ((uint32_t)errorinfo >> 12) & 0xF;
        case 3: return ((uint32_t)errorinfo >>  4) & 0xF;
        default: return 0;
      }
    };

    PXDError::PXDErrorFlags dhc_frames::check_padding(void)
    {
      unsigned int crc = *(ubig32_t*)(((unsigned char*)data) + length - 4);
      if ((crc & 0xFFFF0000) == 0 || (crc & 0xFFFF) == 0) {
        /// TODO many false positives, we should remove that check after we KNOW that it has been fixed in DHH Firmware
        B2INFO("Suspicious Padding $" << std::hex << crc);
        return PXDError::EPXDErrMask::c_SUSP_PADDING;
      }
      return PXDError::EPXDErrMask::c_NO_ERROR;
    };

    PXDError::PXDErrorFlags dhc_frames::check_crc(bool ignore_crc_flag)
    {
      dhc_crc_32_type bocrc;

      if (length > 65536 * 16) {
        if (!ignore_crc_flag) B2WARNING("DHC Data Frame CRC not calculated because of too large packet (>1MB)!");
        return PXDError::EPXDErrMask::c_NO_ERROR; // such large packets should trigger an error elsewhere
      } else {
        bocrc.process_bytes(data, length - 4);
      }
      unsigned int c;
      c = bocrc.checksum();

      ubig32_t crc32;
      crc32 = *(ubig32_t*)(((unsigned char*)data) + length - 4);

      if (c != crc32) {
        if (!ignore_crc_flag) {
          B2WARNING("DHC Data Frame CRC FAIL");
          B2DEBUG(1, "DHC Data Frame CRC FAIL: " << std::hex << c << "!=" << crc32 << " data "
                  << * (unsigned int*)(((unsigned char*)data) + length - 8) << " "
                  << * (unsigned int*)(((unsigned char*)data) + length - 6) << " "
                  << * (unsigned int*)(((unsigned char*)data) + length - 4) << " len $" << length);
        }
        return PXDError::EPXDErrMask::c_DHE_CRC;
      }
      return PXDError::EPXDErrMask::c_NO_ERROR;
    };


    unsigned int dhc_frames::getFixedSize(void)
    {
      unsigned int s = 0;
      switch (getFrameType()) {
        case EDHCFrameHeaderDataType::c_DHP_RAW:
          s = 0; /// size is not a fixed number
          break;
        case EDHCFrameHeaderDataType::c_ONSEN_DHP:
        case EDHCFrameHeaderDataType::c_DHP_ZSD:
          s = 0; /// size is not a fixed number
          break;
        case EDHCFrameHeaderDataType::c_ONSEN_FCE:
        case EDHCFrameHeaderDataType::c_FCE_RAW:
          s = 0; /// size is not a fixed number
          break;
        case EDHCFrameHeaderDataType::c_COMMODE:
          s = data_commode_frame->getFixedSize();
          break;
        case EDHCFrameHeaderDataType::c_GHOST:
          s = data_ghost_frame->getFixedSize();
          break;
        case EDHCFrameHeaderDataType::c_DHE_START:
          s = data_dhe_start_frame->getFixedSize();
          break;
        case EDHCFrameHeaderDataType::c_DHE_END:
          s = data_dhe_end_frame->getFixedSize();
          break;
        case EDHCFrameHeaderDataType::c_DHC_START:
          s = data_dhc_start_frame->getFixedSize();
          break;
        case EDHCFrameHeaderDataType::c_DHC_END:
          s = data_dhc_end_frame->getFixedSize();
          break;
        case EDHCFrameHeaderDataType::c_ONSEN_ROI:
          s = 0; /// size is not a fixed number
          break;
        case EDHCFrameHeaderDataType::c_ONSEN_TRG:
          s = data_onsen_trigger_frame->getFixedSize();
          break;
        default:
          B2INFO("Error: not a valid data frame!");
          // Error will be set elsewhere in another check
          s = 0;
          break;
      }
      datasize = s;
      return s;
    };

  };
};
