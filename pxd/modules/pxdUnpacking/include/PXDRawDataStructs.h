
#include <pxd/dataobjects/PXDErrorFlags.h>
#include <pxd/modules/pxdUnpacking/PXDRawDataDefinitions.h>
#include <framework/logging/Logger.h>

#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::PXD::PXDError;

using namespace boost::spirit::endian;


/// define our CRC function
using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhe_crc_32_type;


///*********************************************************************************
///****************** DHC Data Frame structs starts here ***************************
///*********************************************************************************

namespace Belle2 {

  namespace PXD {

    struct dhc_frame_header_word0 {
      const ubig16_t data;
      /// fixed length
      inline ubig16_t getData(void) const
      {
        return data;
      };
      inline unsigned short getFrameType(void) const
      {
        return (data & 0x7800) >> 11;
      };
      inline unsigned short getErrorFlag(void) const
      {
        return (data & 0x8000) >> 15;
      };
      inline unsigned short getMisc(void) const
      {
        return data & 0x3FF;
      };
      void print(void) const
      {
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
        B2DEBUG(20, "DHC FRAME TYP $" << hex << getFrameType() << " -> " << dhc_type_name[getFrameType()] << " (ERR " << getErrorFlag() <<
                ") data " << data);
      };
    };

    struct dhc_start_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig16_t trigger_nr_hi;
      const ubig16_t time_tag_lo_and_type;
      const ubig16_t time_tag_mid;
      const ubig16_t time_tag_hi;
      const ubig16_t run_subrun;
      const ubig16_t exp_run;
      const unsigned int crc32;
      /// fixed length, only for reading

      inline unsigned short getRunSubrun(void) const { return run_subrun; };
      inline unsigned short getExpRun(void) const { return exp_run;  };
      inline unsigned short getEventNrLo(void) const { return trigger_nr_lo; };
      inline unsigned short getEventNrHi(void) const {    return trigger_nr_hi;  };

      bool isFakedData(void) const
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
      inline unsigned int getFixedSize(void) const
      {
        return 20;// bytes
      };
      void print(void) const
      {
        word0.print();
        B2DEBUG(20, "DHC Start Frame TNRLO $" << hex << trigger_nr_lo << " TNRHI $" << hex << trigger_nr_hi << " TTLO $" << hex <<
                time_tag_lo_and_type
                << " TTMID $" << hex << time_tag_mid << " TTHI $" << hex << time_tag_hi << " Exp/Run/Subrun $" << hex << exp_run << " $" <<
                run_subrun
                << " CRC $" << hex << crc32);
      };
      inline unsigned short get_active_dhe_mask(void) const {return word0.getMisc() & 0x1F;};
      inline unsigned short get_dhc_id(void) const {return (word0.getMisc() >> 5) & 0xF;};
      inline unsigned short get_subrun(void) const {return run_subrun & 0x00FF;};
      inline unsigned short get_run(void) const {return (((run_subrun & 0xFF00) >> 8)  | ((exp_run & 0x003F) << 8));};
      inline unsigned short get_experiment(void) const {return (exp_run & 0xFFC0) >> 6 ;};
    };

    struct dhc_dhe_start_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig16_t trigger_nr_hi;
      const ubig16_t dhe_time_tag_lo;
      const ubig16_t dhe_time_tag_hi;
      const ubig16_t sfnr_offset;
      const unsigned int crc32;
      /// fixed length

      inline unsigned short getEventNrLo(void) const   {    return trigger_nr_lo;  };
      inline unsigned short getEventNrHi(void) const  {    return trigger_nr_hi;  };
      inline unsigned short getStartFrameNr(void) const  {    return (sfnr_offset & 0xFC00) >> 10;  };  // last DHP frame before trigger
      inline unsigned short getTriggerOffsetRow(void) const   {    return sfnr_offset & 0x03FF;  }; // and trigger row offset
      inline unsigned int getFixedSize(void) const  {    return 16; };// 8 words

      void print(void) const
      {
        word0.print();
        B2DEBUG(20, "DHC Event Frame TNRLO $" << hex << trigger_nr_lo  << " DTTLO $" << hex << dhe_time_tag_lo << " DTTHI $" << hex <<
                dhe_time_tag_hi
                << " DHEID $" << hex << getDHEId()
                << " DHPMASK $" << hex << getActiveDHPMask()
                << " SFNR $" << hex << getStartFrameNr()
                << " OFF $" << hex << getTriggerOffsetRow()
                << " CRC " << hex << crc32);
      };
      inline unsigned int getActiveDHPMask(void) const {return word0.getMisc() & 0xF;};
      inline unsigned int getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
    };

    struct dhc_commode_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig16_t data[96];
      const unsigned int crc32;
      /// fixed length

      inline unsigned short getEventNrLo(void) const  {    return trigger_nr_lo;  };
      inline unsigned int getFixedSize(void) const  {    return (4 + 96) * 2; };// 100 words
      inline unsigned int getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
    };

    struct dhc_direct_readout_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      /// an unbelievable amount of words may follow
      /// and finally a 32 bit checksum

      inline unsigned short getEventNrLo(void) const   {    return trigger_nr_lo;  };
      void print(void) const
      {
        word0.print();
        B2DEBUG(20, "DHC Direct Readout (Raw|ZSD|ONS) Frame TNRLO $" << hex << trigger_nr_lo << " DHE ID $" << getDHEId() << " DHP port $"
                <<
                getDHPPort());
      };
      inline unsigned short getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
      inline unsigned short getDHPPort(void) const {return (word0.getMisc()) & 0x3;};
      inline bool getDataReformattedFlag(void) const {return (word0.getMisc() >> 3) & 0x1;};
    };


    struct dhc_direct_readout_frame_raw : public dhc_direct_readout_frame {
    };

    struct dhc_direct_readout_frame_zsd : public dhc_direct_readout_frame {
    };

    struct dhc_onsen_trigger_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trignr0;
      const ubig32_t magic1;//! CAFExxxx , redundant
      const ubig32_t trignr1;//! HLT Trigger/Tag part 1
      const ubig32_t trigtag1;//! HLT Trigger/Tag part 2
      const ubig32_t magic2;/// CAFExxxx, redundant
      const ubig32_t trignr2;/// redundant, DATCON Trigger/Tag part 1
      const ubig32_t trigtag2;/// redundant, DATCON Trigger/Tag part 2
      const unsigned int crc32;

      inline unsigned int getFixedSize(void) const  {    return 32;  }; //  8*4 bytes might still be changed
      inline unsigned short get_trig_nr0(void) const   {    return trignr0;  };
      inline unsigned int get_trig_nr1(void) const  {    return trignr1;  };
      inline unsigned int get_trig_nr2(void) const  {    return trignr2;  };
      inline unsigned int get_trig_tag1(void) const  {    return trigtag1;  };
      inline unsigned int get_trig_tag2(void) const  {    return trigtag2;  };
      inline unsigned short get_subrun1(void) const {return trigtag1 & 0xFF;};
      inline unsigned short get_run1(void) const {return ((trigtag1 & 0x003FFF00) >> 8);};
      inline unsigned short get_experiment1(void) const {return (trigtag1 & 0xFFC00000) >> 22 ;};
      inline unsigned short get_subrun2(void) const {return trigtag2 & 0xFF;};
      inline unsigned short get_run2(void) const {return ((trigtag2 & 0x003FFF00) >> 8);};
      inline unsigned short get_experiment2(void) const {return (trigtag2 & 0xFFC00000) >> 22 ;};
      void print(void) const
      {
        word0.print();
        B2DEBUG(20, "ONSEN Trigger Frame TNRLO $" << hex << trignr0);
      };
      PXDErrorFlags check_error(bool ignore_datcon_flag = false) const
      {
        PXDErrorFlags m_errorMask = EPXDErrMask::c_NO_ERROR;
        if ((magic1 & 0xFFFF0000) != 0xCAFE0000) {
          B2ERROR("ONSEN Trigger Magic 1 error $" << hex << magic1);
          m_errorMask |= EPXDErrMask::c_HLTROI_MAGIC;
        }
        if ((magic2 & 0xFFFF0000) != 0xCAFE0000) {
          B2ERROR("ONSEN Trigger Magic 2 error $" << hex << magic2);
          m_errorMask |= EPXDErrMask::c_HLTROI_MAGIC;
        }
        if (is_fake_datcon()) {
          if (!ignore_datcon_flag) B2WARNING("ONSEN Trigger Frame: No DATCON data $" << hex << trignr1 << "!=$" << trignr2);
          m_errorMask |= EPXDErrMask::c_NO_DATCON;
        } else {
          if (trignr1 != trignr2) {
            B2ERROR("ONSEN Trigger Frame Trigger Nr Mismatch $" << hex << trignr1 << "!=$" << trignr2);
            m_errorMask |= EPXDErrMask::c_MERGER_TRIGNR;
          }
        }
        return m_errorMask;
      };

      inline bool is_fake_datcon(void) const { return (magic2 == 0xCAFE0000 && trignr2 == 0x00000000 && trigtag2 == 0x00000000);};
      inline bool is_Accepted(void) const  {    return (magic1 & 0x8000) != 0;  };
      inline bool is_SendROIs(void) const  {    return (magic1 & 0x2000) != 0;  }
      inline bool is_SendUnfiltered(void) const  {    return (magic1 & 0x4000) != 0;  };
    };

    struct dhc_onsen_roi_frame {
      const dhc_frame_header_word0 word0;/// mainly empty
      const ubig16_t trignr0;
      /// plus n* ROIs (64 bit)
      /// plus inner checksum 32bit
      /// plus checksum 32bit

      inline unsigned short get_trig_nr0(void) const   {    return trignr0;  };
      PXDErrorFlags check_error(int length) const
      {
        PXDErrorFlags m_errorMask = 0;
        // 4 byte header, ROIS (n*8), 4 byte copy of inner CRC, 4 byte outer CRC
        if (length < minSize()) {
          B2ERROR("DHC ONSEN HLT/ROI Frame too small to hold any ROIs!");
          m_errorMask |= c_ROI_PACKET_INV_SIZE;
        } else if ((length - minSize()) % 8 != 0) {
          B2ERROR("DHC ONSEN HLT/ROI Frame holds fractional ROIs, last ROI might not be saved!");
          m_errorMask |= c_ROI_PACKET_INV_SIZE;
        }
        return m_errorMask;
      };
      void print(void) const
      {
        word0.print();
        B2DEBUG(20, "DHC HLT/ROI Frame");
      };
      // 4 byte header, ROIS (n*8), 4 byte copy of inner CRC, 4 byte outer CRC
      inline int minSize(void) const {return 4 + 4 + 4;};
      unsigned int check_inner_crc(unsigned int /*length*/) const
      {
        /// Parts of the data are now in the ONSEN Trigger frame, therefore the inner CRC cannot be checked that easily!
        // TODO can be re-implemented if needed
        return 0;
      };

    };

    struct dhc_ghost_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const unsigned int crc32;
      /// fixed length

      inline unsigned int getFixedSize(void) const  {    return 8;  };
      void print(void) const
      {
        word0.print();
        B2DEBUG(20, "DHC Ghost Frame TNRLO $" << hex << trigger_nr_lo << " DHE ID $" << getDHEId() << " DHP port $" << getDHPPort() <<
                " CRC $");
      };
      inline unsigned short getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
      inline unsigned short getDHPPort(void) const {return (word0.getMisc()) & 0x3;};
    };

    struct dhc_end_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig32_t wordsinevent;
      const unsigned int errorinfo;
      const unsigned int crc32;
      /// fixed length

      unsigned int get_words(void) const  {    return wordsinevent;  }
      inline unsigned int getFixedSize(void) const  {    return 16;  };
      bool isFakedData(void) const
      {
        if (word0.data != 0x6000) return false;
        if (trigger_nr_lo != 0) return false;
        if (wordsinevent != 0) return false;
        if (errorinfo != 0) return false;
        if (crc32 != 0xF7BCA507) return false;
        return true;
      };
      void print(void) const
      {
        word0.print();
        B2DEBUG(20, "DHC End Frame TNRLO $" << hex << trigger_nr_lo << " WIEVT $" << hex << wordsinevent << " ERR $" << hex << errorinfo
                << " CRC " << hex << crc32);
      };
      inline unsigned int get_dhc_id(void) const {return (word0.getMisc() >> 5) & 0xF;};
    };

    struct dhc_dhe_end_frame {
      const dhc_frame_header_word0 word0;
      const ubig16_t trigger_nr_lo;
      const ubig16_t wordsineventlo; // words swapped... because of DHE 16 bit handling
      const ubig16_t wordsineventhi;
      const unsigned int errorinfo;// not well defined yet
      const unsigned int crc32;
      /// fixed length

      unsigned int get_words(void) const  {    return wordsineventlo | ((unsigned int)wordsineventhi << 16);  }
      inline unsigned int getFixedSize(void) const  {    return 16;  };
      void print(void) const
      {
        word0.print();
        B2DEBUG(20, "DHC DHE End Frame TNRLO $" << hex << trigger_nr_lo << " WIEVT $" << hex << wordsineventhi << "." << wordsineventlo <<
                " ERR $"
                << hex << errorinfo
                << " CRC " << hex << crc32);
      };
      inline unsigned int getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
    };

  } // end namespace for keeping structs private to translation unit

///*********************************************************************************
///****************** DHC Frame Wrapper Code starts here **************************
///*********************************************************************************

  class dhc_frames {
  public:
    union {
      const void* data;/// no type
      const dhc_onsen_trigger_frame* data_onsen_trigger_frame;
      const dhc_start_frame* data_dhc_start_frame;
      const dhc_end_frame* data_dhc_end_frame;
      const dhc_dhe_start_frame* data_dhe_start_frame;
      const dhc_dhe_end_frame* data_dhe_end_frame;
      const dhc_commode_frame* data_commode_frame;
      const dhc_direct_readout_frame* data_direct_readout_frame;
      const dhc_direct_readout_frame_raw* data_direct_readout_frame_raw;
      const dhc_direct_readout_frame_zsd* data_direct_readout_frame_zsd;
      const dhc_ghost_frame* data_ghost_frame;
      const dhc_onsen_roi_frame* data_onsen_roi_frame;
    };
    unsigned int datasize;
    int type;
    int length;

    dhc_frames(void)
    {
      data = 0;
      datasize = 0;
      type = -1;
      length = 0;
    };
    int getFrameType(void)
    {
      return type;
    };
    void set(void* d, unsigned int t)
    {
      data = d;
      type = t;
      length = 0;
    };
    void set(void* d, unsigned int t, unsigned int l)
    {
      data = d;
      type = t;
      length = l;
    };
    void set(void* d)
    {
      data = d;
      type = ((dhc_frame_header_word0*)data)->getFrameType();
      length = 0;
    };
    inline unsigned int getEventNrLo(void) const   {    return ((ubig16_t*)data)[1];  };
    PXDErrorFlags check_padding(void)
    {
      unsigned int crc = *(ubig32_t*)(((unsigned char*)data) + length - 4);
      if ((crc & 0xFFFF0000) == 0 || (crc & 0xFFFF) == 0) {
        B2WARNING("Suspicious Padding $" << hex << crc);
        return EPXDErrMask::c_SUSP_PADDING;
      }
      return EPXDErrMask::c_NO_ERROR;
    };

    PXDErrorFlags check_crc(void)
    {
      dhe_crc_32_type bocrc;

      if (length > 65536 * 16) {
        B2WARNING("DHC Data Frame CRC not calculated because of too large packet (>1MB)!");
      } else {
        bocrc.process_bytes(data, length - 4);
      }
      unsigned int c;
      c = bocrc.checksum();

      ubig32_t crc32;
      crc32 = *(ubig32_t*)(((unsigned char*)data) + length - 4);

      if (c == crc32) {
//       if (verbose)
        //         B2INFO("DHE Data Frame CRC: " << hex << c << "==" << crc32);
//         B2INFO("DHC Data Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
//                << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " len $" << length);
      } else {
//       crc_error++;
//       if (verbose) {
        B2ERROR("DHC Data Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(((
                  unsigned char*)data) + length - 8) << " "
                << * (unsigned int*)(((unsigned char*)data) + length - 6) << " " << * (unsigned int*)(((unsigned char*)data) + length - 4) <<
                " len $" << length);
        /// others would be interessting but possible subjects to access outside of buffer
        /// << " " << * (unsigned int*)(d + length - 2) << " " << * (unsigned int*)(d + length + 0) << " " << * (unsigned int*)(d + length + 2));
        //if (length <= 32) {
        //  for (int i = 0; i < length / 4; i++) {
        //    B2ERROR("== " << i << "  $" << hex << ((unsigned int*)d)[i]);
        //  }
        //}
//       };
//       error_flag = true;
        return EPXDErrMask::c_DHE_CRC;
      }
      return EPXDErrMask::c_NO_ERROR;
    };


    unsigned int getFixedSize(void)
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
          B2ERROR("Error: not a valid data frame!");
//         error_flag = true;
          s = 0;
          break;
      }
      datasize = s;
      return s;
    };

  };
};
