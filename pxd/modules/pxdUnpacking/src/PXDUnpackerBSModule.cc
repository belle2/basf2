/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdUnpacking/PXDRawDataDefinitions.h>
#include <pxd/modules/pxdUnpacking/PXDUnpackerBSModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::PXD::PXDError;

using namespace boost::spirit::endian;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDUnpackerBS)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

/// define our CRC function
using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhe_crc_32_type;

///*********************************************************************************
///****************** DHC Data Frame Code starts here *****************************
///*********************************************************************************

// namespace for keeping structs private to translation unit
namespace {

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
    const ubig16_t trignr0;// not used
    /// plus n* ROIs (64 bit)
    /// plus checksum 32bit

    inline unsigned short get_trig_nr0(void) const   {    return trignr0;  };
    PXDErrorFlags check_error(unsigned int length) const
    {
      PXDErrorFlags m_errorMask = 0;
      if (length < 4 + 4 + 4) {
        B2ERROR("DHC ONSEN HLT/ROI Frame too small to hold any ROIs, did not save anything!");
        m_errorMask |= c_ROI_PACKET_INV_SIZE;
      } else if ((length - 4 - 4 - 4) % 8 != 0) {
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

    unsigned int check_inner_crc(unsigned int /*length*/) const
    {
      /// Parts of the data are now in the ONSEN Trigger frame, therefore the inner CRC cannot be checked that easily!
      // TODO can be re-implemented if needed
      return 0;
    };
    void save(StoreArray<PXDRawROIs>& sa, unsigned int length, unsigned int* data) const
    {
      // 4 byte header, ROIS (n*8), 4 byte copy of inner CRC, 4 byte outer CRC
      if (length < 4 + 4 + 4) {
        return;
      }
      if ((length - 4 - 4 - 4) % 8 != 0) {
        // PXDUnpackerBSModule::dump_roi(data, length - 4); // minus CRC
      }
      unsigned int l;
      l = (length - 4 - 4 - 4) / 8;
      // Endian swapping is done in Contructor of RawRoi object
      sa.appendNew(l, &data[1]);
    }

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

///******************************************************************
///*********************** Main unpacker code ***********************
///******************************************************************

PXDUnpackerBSModule::PXDUnpackerBSModule() :
  Module(),
  m_storeRawHits(),
  m_storeROIs(),
  m_storeRawAdc(),
  m_storeRawPedestal(),

  ////Cluster store
  m_storeRawCluster()
{
  //Set module properties
  setDescription("Unpack Raw PXD Hits from ONSEN data stream");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("RawPXDsName", m_RawPXDsName, "The name of the StoreArray of RawPXDs to be processed", std::string(""));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "The name of the StoreArray of generated PXDRawHits", std::string(""));
  addParam("PXDRawAdcsName", m_PXDRawAdcsName, "The name of the StoreArray of generated PXDRawAdcs", std::string(""));
  addParam("PXDRawPedestalsName", m_PXDRawPedestalsName, "The name of the StoreArray of generated PXDRawPedestals", std::string(""));
  addParam("PXDRawROIsName", m_PXDRawROIsName, "The name of the StoreArray of generated PXDRawROIs", std::string(""));
  addParam("HeaderEndianSwap", m_headerEndianSwap, "Swap the endianess of the ONSEN header", true);
  addParam("IgnoreDATCON", m_ignoreDATCON, "Ignore missing DATCON ROIs", false);
  addParam("IgnoreMetaFlags", m_ignoreMetaFlags, "Ignore wrong Meta event flags", false);
  addParam("DoNotStore", m_doNotStore, "only unpack and check, but do not store", false);
  addParam("ClusterName", m_RawClusterName, "The name of the StoreArray of PXD Clusters to be processed", std::string(""));
  addParam("DESY16FixTrigOffset", m_DESY16_FixTrigOffset,
           "Fix trigger offset (only trigger number, not data) between Meta Event and HLT", 0);
  addParam("DESY16FixRowOffset", m_DESY16_FixRowOffset, "Fix row offset by shifting row by value (one gates is 4 pixel rows)", 0);
  addParam("CriticalErrorMask", m_criticalErrorMask, "Set error mask which stops processing by returning false by task", (uint64_t)0);
//   (
//              /*EPXDErrFlag::c_DHC_END | EPXDErrFlag::c_DHE_START | EPXDErrFlag::c_DATA_OUTSIDE |*/
//              EPXDErrFlag::c_FIX_SIZE | EPXDErrFlag::c_DHE_CRC | EPXDErrFlag::c_DHC_UNKNOWN | /*EPXDErrFlag::c_MERGER_CRC |*/
//              EPXDErrFlag::c_DHP_SIZE | /*EPXDErrFlag::c_DHP_PIX_WO_ROW | EPXDErrFlag::c_DHE_START_END_ID | EPXDErrFlag::c_DHE_START_ID |*/
//              EPXDErrFlag::c_DHE_START_WO_END | EPXDErrFlag::c_DHP_NOT_CONT
//            ));
}

void PXDUnpackerBSModule::initialize()
{
  m_storeRawPXD.isRequired(m_RawPXDsName);
  //Register output collections
  m_storeRawHits.registerInDataStore(m_PXDRawHitsName);
  m_storeRawAdc.registerInDataStore(m_PXDRawAdcsName);
  m_storeRawPedestal.registerInDataStore(m_PXDRawPedestalsName);
  m_storeROIs.registerInDataStore(m_PXDRawROIsName);
  m_storeDAQEvtStats.registerInDataStore();
  m_storeRawCluster.registerInDataStore(m_RawClusterName);
  /// actually, later we do not want to store ROIs and Pedestals into output file ...  aside from debugging

  B2INFO("HeaderEndianSwap: " << m_headerEndianSwap);
  B2INFO("Ignore(missing)DATCON: " << m_ignoreDATCON);
  B2INFO("Ignore (some) missing Meta flags: " << m_ignoreMetaFlags);

  ignore_datcon_flag = m_ignoreDATCON;

  m_sendunfiltered = 0;
  m_sendrois = 0;
  m_notaccepted = 0;
  m_unpackedEventsCount = 0;
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) m_errorCounter[i] = 0;

  // test_mapping();// write out the mapping table which is used for comparison ...
}

void PXDUnpackerBSModule::terminate()
{
  int flag = 0;
  string errstr = "Statistic ( ;";
  errstr += to_string(m_unpackedEventsCount) + ";";
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) { errstr += to_string(m_errorCounter[i]) + ";"; flag |= m_errorCounter[i];}
  if (flag != 0) {
    B2RESULT("PXD Unpacker --> Error Statistics (counted once per event!) in Events: " << m_unpackedEventsCount);
    B2RESULT(errstr + " )");
    for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      if (m_errorCounter[i]) {
        B2RESULT(getPXDBitErrorName(1ull << i) << ": " << m_errorCounter[i]);
      }
    }
  } else {
    B2INFO("PXD Unpacker --> No Error found in Events: " << m_unpackedEventsCount);
  }
  B2RESULT("Statistic 2: !Accepted: " << m_notaccepted << " SendROIs: " << m_sendrois << " Unfiltered: " << m_sendunfiltered);
}

void PXDUnpackerBSModule::event()
{
  StoreObjPtr<EventMetaData> evtPtr;/// what will happen if it does not exist???

  int nRaws = m_storeRawPXD.getEntries();
  if (verbose) {
    B2DEBUG(20, "PXD Unpacker --> RawPXD Objects in event: " << nRaws);
  };

  m_errorMask = 0;
  m_errorMaskEvent = 0;

  m_meta_event_nr = evtPtr->getEvent();
  if (m_DESY16_FixTrigOffset != 0) {
    m_meta_event_nr += m_DESY16_FixTrigOffset;
  }
  m_meta_run_nr = evtPtr->getRun();
  m_meta_subrun_nr = evtPtr->getSubrun();
  m_meta_experiment = evtPtr->getExperiment();
  m_meta_time = evtPtr->getTime();

  PXDDAQStatus daqevtstat(EPXDErrMask::c_NO_ERROR);

  int inx = 0; // count index for output objects
  for (auto& it : m_storeRawPXD) {
    if (verbose) {
      B2DEBUG(20, "PXD Unpacker --> Unpack Objects: ");
    };
    unpack_rawpxd(it, inx++, daqevtstat);
  }

  if (nRaws == 0) m_errorMask |= EPXDErrMask::c_NO_PXD;

  m_errorMaskEvent |= m_errorMask;
  daqevtstat.setErrorMask(m_errorMaskEvent);
  m_storeDAQEvtStats.appendNew(daqevtstat);

  m_unpackedEventsCount++;
  {
    uint64_t j = 1;
    for (unsigned int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      if (m_errorMaskEvent & j) m_errorCounter[i]++;
      j <<= 1;
    }
  }

  setReturnValue(0 == (m_criticalErrorMask & m_errorMaskEvent));
}

void PXDUnpackerBSModule::unpack_rawpxd(RawPXD& px, int inx, PXDDAQStatus& daqevtstat)
{
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  m_errorMaskDHE = 0;
  m_errorMaskDHC = 0;
  m_errorMaskPacket = 0;
  PXDDAQPacketStatus daqpktstat(inx, EPXDErrMask::c_NO_ERROR);
  std::vector <PXDDAQDHEStatus> daqdhevect;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD Unpacker --> invalid packet size (32bit words) $" << hex << px.size());
    m_errorMask |= EPXDErrMask::c_PACKET_SIZE;
    return;
  }
  std::vector<unsigned int> data(px.size());
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  std::copy_n(px.data(), px.size(), data.begin());

  if (fullsize < 8) {
    B2ERROR("Data is to small to hold a valid Header! Will not unpack anything. Size:" << fullsize);
    m_errorMask |= EPXDErrMask::c_PACKET_SIZE;
    return;
  }

  if (data[0] != 0xCAFEBABE && data[0] != 0xBEBAFECA) {
    B2ERROR("Magic invalid: Will not unpack anything. Header corrupted! $" << hex << data[0]);
    m_errorMask |= EPXDErrMask::c_MAGIC;
    return;
  }

  if (m_headerEndianSwap) Frames_in_event = ((ubig32_t*)data.data())[1];
  else Frames_in_event = ((ulittle32_t*)data.data())[1];
  if (Frames_in_event < 0 || Frames_in_event > 256) {
    B2ERROR("Number of Frames invalid: Will not unpack anything. Header corrupted! Frames in event: " << Frames_in_event);
    m_errorMask |= EPXDErrMask::c_FRAME_NR;
    return;
  }

  /// NEW format
  if (verbose) {
    B2DEBUG(20, "PXD Unpacker --> data[0]: <-- Magic $" << hex << data[0]);
    B2DEBUG(20, "PXD Unpacker --> data[1]: <-- #Frames $" << hex << data[1]);
    if (data[1] >= 1 && fullsize < 12) B2DEBUG(20, "PXD Unpacker --> data[2]: <-- Frame 1 len $" << hex << data[2]);
    if (data[1] >= 2 && fullsize < 16) B2DEBUG(20, "PXD Unpacker --> data[3]: <-- Frame 2 len $" << hex << data[3]);
    if (data[1] >= 3 && fullsize < 20) B2DEBUG(20, "PXD Unpacker --> data[4]: <-- Frame 3 len $" << hex << data[4]);
    if (data[1] >= 4 && fullsize < 24) B2DEBUG(20, "PXD Unpacker --> data[5]: <-- Frame 4 len $" << hex << data[5]);
  };

  unsigned int* tableptr;
  tableptr = &data[2]; // skip header!!!

  unsigned int* dataptr;
  dataptr = &tableptr[Frames_in_event];
  datafullsize = fullsize - 2 * 4 - Frames_in_event * 4; // minus header, minus table

  int ll = 0; // Offset in dataptr in bytes
  for (int j = 0; j < Frames_in_event; j++) {
    int lo;/// len of frame in bytes
    if (m_headerEndianSwap) lo = ((ubig32_t*)tableptr)[j];
    else lo = ((ulittle32_t*)tableptr)[j];
    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      m_errorMask |= EPXDErrMask::c_FRAME_SIZE;
      return;
    }
    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " <<
              datafullsize << " of fullsize " << fullsize);
      m_errorMask |= EPXDErrMask::c_FRAME_SIZE;
      return;
    }
    if (lo & 0x3) {
      B2ERROR("SKIP Frame with Data with not MOD 4 length " << " ( $" << hex << lo << " ) ");
      ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
    } else {
      B2INFO("unpack DHE(C) frame: " << j << " with size " << lo << " at byte offset in dataptr " << ll);
      unpack_dhc_frame(ll + (char*)dataptr, lo, j, Frames_in_event, daqpktstat, daqdhevect);
      ll += lo; /// no rounding needed
    }
    m_errorMaskDHE |= m_errorMask;
    m_errorMaskDHC |= m_errorMask;
    m_errorMaskPacket |= m_errorMask;
    m_errorMaskEvent |= m_errorMask;
    m_errorMask = 0;
  }
  daqpktstat.setErrorMask(m_errorMaskPacket);
  daqevtstat.addPacket(daqpktstat);
}

void PXDUnpackerBSModule::unpack_dhp_raw(void* data, unsigned int frame_len, unsigned int dhe_ID, unsigned dhe_DHPport,
                                         VxdID vxd_id)
{
//   unsigned int nr_words = frame_len / 2; // frame_len in bytes (excl. CRC)!!!
  ubig16_t* dhp_pix = (ubig16_t*)data;
  // ADC/ADC and ADC/PEDESTAL can only be distinguised by length of frame

  //! *************************************************************
  //! Important Remark:
  //! Up to now the format for Raw frames as well as size etc
  //! is not well defined. It will most likely change!
  //! E.g. not the whole mem is dumped, but only a part of it.
  //! *************************************************************

  if (frame_len != 0x10008 && frame_len != 0x20008) {
    B2ERROR("Frame size unsupported for RAW pedestal frame! $" << hex << frame_len << " bytes");
    return;
  }
  unsigned int dhp_header_type  = 0;
//   unsigned int dhp_reserved     = 0;
  unsigned int dhp_dhe_id       = 0;
  unsigned int dhp_dhp_id       = 0;

  dhp_header_type  = (dhp_pix[2] & 0xE000) >> 13;
//   dhp_reserved     = (dhp_pix[2] >> 8) & 0x1F;
  dhp_dhe_id       = (dhp_pix[2] & 0x00FC) >> 2;
  dhp_dhp_id       =  dhp_pix[2] & 0x0003;

  if (dhe_ID != dhp_dhe_id) {
    B2ERROR("DHE ID in DHE and DHP header differ $" << hex << dhe_ID << " != $" << dhp_dhe_id);
    m_errorMask |= EPXDErrMask::c_DHE_DHP_DHEID;
  }
  if (dhe_DHPport != dhp_dhp_id) {
    B2ERROR("DHP ID (Chip/Port) in DHE and DHP header differ $" << hex << dhe_DHPport << " != $" << dhp_dhp_id);
    m_errorMask |= EPXDErrMask::c_DHE_DHP_PORT;
  }

  if (dhp_header_type != EDHPFrameHeaderDataType::c_RAW) {
    B2ERROR("Header type invalid for this kind of DHE frame: $" << hex << dhp_header_type);
    return;
  }

  /// Endian Swapping is done in Contructors of Raw Objects!
  if (frame_len == 0x10008) { // 64k
    B2DEBUG(20, "Pedestal Data - (ADC:ADC)");
    m_storeRawAdc.appendNew(vxd_id, data, false);
  } else if (frame_len == 0x20008) { // 128k
    B2DEBUG(20, "Pedestal Data - (ADC:Pedestal)");
    m_storeRawAdc.appendNew(vxd_id, data, true);
    m_storeRawPedestal.appendNew(vxd_id, data);
  } else {
    // checked already above
  }
};

void PXDUnpackerBSModule::unpack_fce(unsigned short* data, unsigned int length, VxdID vxd_id)
{
  ubig16_t* cluster = (ubig16_t*)data;
  int nr_words; //words in dhp frame
  unsigned int words_in_cluster = 0; //counts 16bit words in cluster
  nr_words = length / 2;
  ubig16_t sor;
  sor = 0x0000;

  for (int i = 2 ; i < nr_words ; i++) {
    if (i != 2) { //skip header
      if ((((cluster[i] & 0x8000) == 0)
           && ((cluster[i] & 0x4000) >> 14) == 1)) {  //searches for start of row frame with start of cluster flag = 1 => new cluster
        if (!m_doNotStore) m_storeRawCluster.appendNew(&data[i - words_in_cluster], words_in_cluster, vxd_id);
        words_in_cluster = 0;
      }
    }
    if ((cluster[i] & 0x8000) == 0) {
      sor = cluster[i];
    }
    words_in_cluster++;

    if ((cluster[nr_words - 1] & 0xFFFF) == (sor &
                                             0xFFFF)) {//if frame is not 32bit aligned last word will be the last start of row word
      cluster[nr_words - 1] = 0x0000;//overwrites the last redundant word with zero to make checking easier in PXDHardwareClusterUnpacker
    }

    if (i == nr_words - 1) {
      if (!m_doNotStore) m_storeRawCluster.appendNew(&data[i - words_in_cluster + 1], words_in_cluster, vxd_id);
    }
  }
}

void PXDUnpackerBSModule::dump_dhp(void* data, unsigned int frame_len)
{
  unsigned int w = frame_len / 2;
  ubig16_t* d = (ubig16_t*)data;

  B2ERROR("HEADER --  $" << hex << d[0] << ",$" << hex << d[1] << ",$" << hex << d[2] << ",$" << hex << d[3] << " -- ");

  auto dhp_header_type  = (d[2] & 0xE000) >> 13;
  auto dhp_reserved     = (d[2] & 0x1F00) >> 8;
  auto dhp_dhe_id       = (d[2] & 0x00FC) >> 2;
  auto dhp_dhp_id       =  d[2] & 0x0003;

  B2ERROR("DHP type     | $" << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
  B2ERROR("DHP reserved | $" << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
  B2ERROR("DHP DHE ID   | $" << hex << dhp_dhe_id << " ( " << dec << dhp_dhe_id << " ) ");
  B2ERROR("DHP DHP ID   | $" << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  for (unsigned int i = 4; i < w; i++) {
    B2ERROR("DHP DATA $" << hex << d[i]);
  }
  B2ERROR("DHP CRC $" << hex << d[w] << ",$" << hex << d[w + 1]);
}

void PXDUnpackerBSModule::dump_roi(void* data, unsigned int frame_len)
{
  unsigned int w = frame_len / 4;
  ubig32_t* d = (ubig32_t*)data;

  B2ERROR("HEADER --  $" << hex << d[0] << ",$" << hex << d[1] << ",$" << hex << d[2] << ",$" << hex << d[3] << " -- Len $" << hex <<
          frame_len);

  for (unsigned int i = 0; i < w; i++) {
    B2ERROR("ROI DATA $" << hex << d[i]);
  }
  B2ERROR("ROI CRC $" << hex << d[w]);
}

void PXDUnpackerBSModule::unpack_dhp(void* data, unsigned int frame_len, unsigned int dhe_first_readout_frame_id_lo,
                                     unsigned int dhe_ID, unsigned dhe_DHPport, unsigned dhe_reformat, unsigned short toffset, VxdID vxd_id)
{
  unsigned int nr_words = frame_len / 2; // frame_len in bytes (excl. CRC)!!!
  bool printflag = false;
  ubig16_t* dhp_pix = (ubig16_t*)data;

  unsigned int dhp_readout_frame_lo = 0;
  unsigned int dhp_header_type  = 0;
  unsigned int dhp_reserved     = 0;
  unsigned int dhp_dhe_id       = 0;
  unsigned int dhp_dhp_id       = 0;

  unsigned int dhp_row = 0, dhp_col = 0, dhp_adc = 0, dhp_cm = 0;
//   unsigned int dhp_offset = 0;
  bool rowflag = false;

  if (nr_words < 4) {
    B2ERROR("DHP frame size error (too small) " << nr_words);
    m_errorMask |= EPXDErrMask::c_DHP_SIZE;
//     dhp_size_error++;
    return;
    //return -1;
  }

  if (printflag)
    B2INFO("HEADER -- $" << hex << dhp_pix[0] << hex << dhp_pix[1] << hex << dhp_pix[2] << hex << dhp_pix[3] << " -- ");

  if (printflag)
    B2INFO("DHP Header   | $" << hex << dhp_pix[2] << " ( " << dec << dhp_pix[2] << " ) ");
  dhp_header_type  = (dhp_pix[2] & 0xE000) >> 13;
  dhp_reserved     = (dhp_pix[2] & 0x1F00) >> 8;
  dhp_dhe_id       = (dhp_pix[2] & 0x00FC) >> 2;
  dhp_dhp_id       =  dhp_pix[2] & 0x0003;

  if (printflag) {
    B2INFO("DHP type     | $" << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
    B2INFO("DHP reserved | $" << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
    B2INFO("DHP DHE ID   | $" << hex << dhp_dhe_id << " ( " << dec << dhp_dhe_id << " ) ");
    B2INFO("DHP DHP ID   | $" << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  }

  if (dhe_ID != dhp_dhe_id) {
    B2ERROR("DHE ID in DHE and DHP header differ $" << hex << dhe_ID << " != $" << dhp_dhe_id);
    m_errorMask |= EPXDErrMask::c_DHE_DHP_DHEID;
  }
  if (dhe_DHPport != dhp_dhp_id) {
    B2ERROR("DHP ID (Chip/Port) in DHE and DHP header differ $" << hex << dhe_DHPport << " != $" << dhp_dhp_id);
    m_errorMask |= EPXDErrMask::c_DHE_DHP_PORT;
  }

  if (dhp_header_type != EDHPFrameHeaderDataType::c_ZSD) {
    B2ERROR("Header type invalid for this kind of DHE frame: $" << hex << dhp_header_type);
    return;
  }

//  static int offtab[4] = {0, 64, 128, 192};
//   dhp_offset = offtab[dhp_dhp_id];

  dhp_readout_frame_lo  = dhp_pix[3] & 0xFFFF;
  if (printflag)
    B2INFO("DHP Frame Nr     |  $" << hex << dhp_readout_frame_lo << " ( " << dec << dhp_readout_frame_lo << " ) ");

  if (((dhp_readout_frame_lo - dhe_first_readout_frame_id_lo) & 0x3F) > 1) {
    B2ERROR("DHP Frame Nr differ from DHE Frame Nr by >1 " << dhe_first_readout_frame_id_lo << " != " << (dhp_readout_frame_lo & 0x3F));
    m_errorMask |= EPXDErrMask::c_DHP_DHE_FRAME_DIFFER;
  }
  if (last_dhp_readout_frame_lo[dhp_dhp_id] != -1) {
    if (((dhp_readout_frame_lo - last_dhp_readout_frame_lo[dhp_dhp_id]) & 0xFFFF) != 1) {
      B2ERROR("Two DHP Frames per sensor which frame number differ more than one! " << last_dhp_readout_frame_lo[dhp_dhp_id] << ", " <<
              dhp_readout_frame_lo);
      m_errorMask |= EPXDErrMask::c_DHP_NOT_CONT;
    }
  }
  for (auto j = 0; j < 4; j++) {
    if (last_dhp_readout_frame_lo[j] != -1) {
      if (((dhp_readout_frame_lo - last_dhp_readout_frame_lo[j]) & 0xFFFF) > 1) {
        B2ERROR("Two DHP Frames (different DHP) per sensor which frame number differ more than one! " << last_dhp_readout_frame_lo[j] <<
                ", " <<
                dhp_readout_frame_lo);
        m_errorMask |= EPXDErrMask::c_DHP_DHP_FRAME_DIFFER;
        break;// give msg only once
      }
    }
  }
  last_dhp_readout_frame_lo[dhp_dhp_id] = dhp_readout_frame_lo;

  if (dhp_pix[2] == dhp_pix[4] && dhp_pix[3] + 1 == dhp_pix[5]) {
    // We see a second "header" with framenr+1 ...
    B2ERROR("DHP data: seems to be double header! skipping ... len " << frame_len);
    m_errorMask |= EPXDErrMask::c_DHP_DBL_HEADER;
    dump_dhp(data, frame_len);
//    B2ERROR("Mask $" << hex <<m_errorMask);
    return;
  }

  for (unsigned int i = 4; i < nr_words ; i++) {

    if (printflag)
      B2INFO("-- $" << hex << dhp_pix[i] << " --   " << dec << i);
    {
      if (((dhp_pix[i] & 0x8000) >> 15) == 0) {
        rowflag = true;
        dhp_row = (dhp_pix[i] & 0xFFC0) >> 5;
        dhp_cm  = dhp_pix[i] & 0x3F;
        if (printflag)
          B2INFO("SetRow: $" << hex << dhp_row << " CM $" << hex << dhp_cm);
      } else {
        if (!rowflag) {
          B2ERROR("DHP Unpacking: Pix without Row!!! skip dhp data ");
          m_errorMask |= EPXDErrMask::c_DHP_PIX_WO_ROW;
//           dhp_pixel_error++;
          dump_dhp(data, frame_len);
          return;
        } else {
          dhp_row = (dhp_row & 0xFFE) | ((dhp_pix[i] & 0x4000) >> 14);
          dhp_col = ((dhp_pix[i]  & 0x3F00) >> 8);
          unsigned int v_cellID, u_cellID;
          v_cellID = dhp_row;// defaults for no mapping
          if (dhp_row >= 768) {
            B2ERROR("DHP ROW Overflow " << dhp_row);
            m_errorMask |= EPXDErrMask::c_ROW_OVERFLOW;
          }
          u_cellID = dhp_col;// defaults for no mapping
          ///  remapping flag
          /// if (dhe_reformat == 0) dhp_col ^= 0x3C ; /// 0->60 61 62 63 4->56 57 58 59 ...
          //dhp_col += dhp_offset;

          if (dhe_reformat == 0) {
            // data has not been pre-processed by DHH, thus we have to do the mapping ourselves
            if ((dhe_ID & 0x21) == 0x00 || (dhe_ID & 0x21) == 0x21) {
              // if IFOB
              remap_IF_OB(v_cellID, u_cellID, dhp_dhp_id, dhe_ID);
            } else { // else OFIB
              remap_IB_OF(v_cellID, u_cellID, dhp_dhp_id, dhe_ID);
            }
          }
          dhp_adc = dhp_pix[i] & 0xFF;
          if (printflag)
            B2INFO("SetPix: Row $" << hex << dhp_row << " Col $" << hex << dhp_col << " ADC $" << hex << dhp_adc
                   << " CM $" << hex << dhp_cm);

          /*if (verbose) {
            B2INFO("raw    |   " << hex << d[i]);
            B2INFO("row " << hex << ((d[i] >> 20) & 0xFFF) << "(" << ((d[i] >> 20) & 0xFFF) << ")" << " col " << "(" << hex << ((d[i] >> 8) & 0xFFF) << ((d[i] >> 8) & 0xFFF)
                   << " adc " << "(" << hex << (d[i] & 0xFF) << (d[i] & 0xFF) << ")");
            B2INFO("dhe_ID " << dhe_ID);
            B2INFO("start-Frame-Nr " << dec << dhe_first_readout_frame_id_lo);
            B2INFO("toffset " << toffset);
          };*/

          if (!m_doNotStore) m_storeRawHits.appendNew(vxd_id, v_cellID, u_cellID, dhp_adc,
                                                        toffset, (dhp_readout_frame_lo - dhe_first_readout_frame_id_lo) & 0x3F,
                                                        dhp_cm/*, dhp_readout_frame_lo, dhe_first_readout_frame_id_lo*/
                                                       );
        }
      }
    }
  }

  if (printflag) {
    B2INFO("(DHE) DHE_ID $" << hex << dhe_ID << " (DHE) DHP ID $" << hex << dhe_DHPport << " (DHP) DHE_ID $" << hex << dhp_dhe_id <<
           " (DHP) DHP ID $" << hex << dhp_dhp_id);
    /*for (int i = 0; i < raw_nr_words ; i++) {
      B2INFO("RAW      |   " << hex << p_pix[i]);
      printf("raw %08X  |  ", p_pix[i]);
      B2INFO("row " << hex << ((p_pix[i] >> 20) & 0xFFF) << dec << " ( " << ((p_pix[i] >> 20) & 0xFFF) << " ) " << " col " << hex << ((p_pix[i] >> 8) & 0xFFF)
             << " ( " << dec << ((p_pix[i] >> 8) & 0xFFF) << " ) " << " adc " << hex << (p_pix[i] & 0xFF) << " ( " << (p_pix[i] & 0xFF) << " ) "
            );
    }*/
  }
};

int PXDUnpackerBSModule::nr5bits(int i)
{
  /// too lazy to count the bits myself, thus using a small lookup table
  const int lut[32] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5
  };
  return lut[i & 0x1F];
}

void PXDUnpackerBSModule::unpack_dhc_frame(void* data, const int len, const int Frame_Number, const int Frames_in_event,
                                           PXDDAQPacketStatus& daqpktstat, std::vector <PXDDAQDHEStatus>& daqdhevect)
{
  /// The following STATIC variables are used to save some state or count some things
  /// while depacking the frames. they are in most cases (re)set on the first frame or ONSEN trg frame
  /// Most could put in as a class member, but they are only needed within this function
  static unsigned int eventNrOfOnsenTrgFrame = 0;
  static int countedBytesInDHC =
    -0x7FFFFFFF;// Set the counted size invalid if negativ, needs a large negative value because we are adding up to that
  static int countedBytesInDHE =
    -0x7FFFFFFF;// Set the counted size invalid if negativ, needs a large negative value because we are adding up to that
  static int countedDHEStartFrames = 0;
  static int countedDHEEndFrames = 0;
  //  static int nr_of_frames_dhc = 0;/// tbd if and where this number will still show up in te DHC format TODO
  static int countedDHCFrames = 0;/// would not be needed if nr_of_frames_dhc is not within the data structure
  static int mask_active_dhe = 0;// DHE mask (5 bit)
  static int nr_active_dhe =
    0;// just count the active DHEs. Until now, it is not possible to check for the bit mask. we would need the info on which DHE connects to which DHC at which port from gearbox/geometry?
  static int mask_active_dhp = 0;// DHP active mask, 4 bit, per current DHE
  static int found_mask_active_dhp = 0;// mask which DHP send data and check on DHE END frame if it matches
  static unsigned int dhe_first_readout_frame_id_lo = 0;
  static unsigned int dhe_first_offset = 0;
  static unsigned int currentDHCID = 0xFFFFFFFF;
  static unsigned int currentDHEID = 0xFFFFFFFF;
  static unsigned int currentVxdId = 0;
  static bool isFakedData_event = false;
  static bool isUnfiltered_event = false;


  dhc_frame_header_word0* hw = (dhc_frame_header_word0*)data;
//   error_flag = false;

  dhc_frames dhc;
  dhc.set(data, hw->getFrameType(), len);
  int s;
  s = dhc.getFixedSize();
  if (len != s && s != 0) {
    B2ERROR("Fixed frame type size does not match specs: expect " << len << " != " << s << " (in data) ");
    m_errorMask |= EPXDErrMask::c_FIX_SIZE;
  }

  unsigned int eventNrOfThisFrame = dhc.getEventNrLo();
  int type = dhc.getFrameType();

  if (Frame_Number == 0) { /// We reset the counters on the first event
    countedDHEStartFrames = 0;
    countedDHEEndFrames = 0;
    countedBytesInDHC = 0;
    countedBytesInDHE = 0;
    currentDHCID = 0xFFFFFFFF;
    currentDHEID = 0xFFFFFFFF;
    currentVxdId = 0;
    isUnfiltered_event = false;
    isFakedData_event = false;
    if (type == EDHCFrameHeaderDataType::c_DHC_START) {
      B2ERROR("This looks like this is the old Desy 2013/14 testbeam format. Please use the pxdUnpackerDesy1314 module.");
    }
  }

  if (Frame_Number == 1) {
    if (type == EDHCFrameHeaderDataType::c_DHC_START) {
      isFakedData_event = dhc.data_dhc_start_frame->isFakedData();
    }
  }

  // please check if this mask is suitable. At least here we are limited by the 16 bit trigger number in the DHH packet header.
  // we can use more bits in the DHC and DHE START Frame
  if ((eventNrOfThisFrame & 0xFFFF) != (m_meta_event_nr & 0xFFFF)) {
    if (!isFakedData_event) {
      B2ERROR("Event Numbers do not match for this frame $" << hex << eventNrOfThisFrame << "!=$" << m_meta_event_nr <<
              "(MetaInfo) mask");
      m_errorMask |= EPXDErrMask::c_META_MM;
    }
  }

  if (Frame_Number > 1 && Frame_Number < Frames_in_event - 1) {
    if (countedDHEStartFrames != countedDHEEndFrames + 1)
      if (type != EDHCFrameHeaderDataType::c_ONSEN_ROI && type != EDHCFrameHeaderDataType::c_DHE_START) {
        B2ERROR("Data Frame outside a DHE START/END");
        m_errorMask |= EPXDErrMask::c_DATA_OUTSIDE;
      }
  }

  if (hw->getErrorFlag()) {
    if (type != EDHCFrameHeaderDataType::c_GHOST) {
      m_errorMask |= EPXDErrMask::c_HEADER_ERR;
    }
  } else {
    if (type == EDHCFrameHeaderDataType::c_GHOST) {
      m_errorMask |= EPXDErrMask::c_HEADER_ERR_GHOST;
    }
  }
  switch (type) {
    case EDHCFrameHeaderDataType::c_DHP_RAW: {
      countedDHCFrames++;

      if (verbose) dhc.data_direct_readout_frame_raw->print();
      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_direct_readout_frame_raw->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();

//       stat_raw++;

      unpack_dhp_raw(data, len - 4,
                     dhc.data_direct_readout_frame->getDHEId(),
                     dhc.data_direct_readout_frame->getDHPPort(),
                     currentVxdId);

      break;
    };
    case EDHCFrameHeaderDataType::c_ONSEN_DHP:
      // Set the counted size invalid if negativ, needs a large negative value because we are adding up to that
      countedBytesInDHC = -0x7FFFFFFF;
      countedBytesInDHE = -0x7FFFFFFF;
      [[fallthrough]];
    case EDHCFrameHeaderDataType::c_DHP_ZSD: {
      countedDHCFrames++;
      if (verbose) dhc.data_direct_readout_frame->print();
      if (isUnfiltered_event) {
        if (type == EDHCFrameHeaderDataType::c_ONSEN_DHP) m_errorMask |= EPXDErrMask::c_SENDALL_TYPE;
      } else {
        if (type == EDHCFrameHeaderDataType::c_DHP_ZSD) m_errorMask |= EPXDErrMask::c_NOTSENDALL_TYPE;
      }

      //m_errorMask |= dhc.data_direct_readout_frame->check_error();

      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_direct_readout_frame_raw->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();
//       stat_zsd++;
      m_errorMask |= dhc.check_padding();// isUnfiltered_event


      unpack_dhp(data, len - 4,
                 dhe_first_readout_frame_id_lo,
                 dhc.data_direct_readout_frame->getDHEId(),
                 dhc.data_direct_readout_frame->getDHPPort(),
                 dhc.data_direct_readout_frame->getDataReformattedFlag(),
                 dhe_first_offset, currentVxdId);
      break;
    };
    case EDHCFrameHeaderDataType::c_ONSEN_FCE:
      // Set the counted size invalid if negativ, needs a large negative value because we are adding up to that
      countedBytesInDHC = -0x7FFFFFFF;
      countedBytesInDHE = -0x7FFFFFFF;
      [[fallthrough]];
    case EDHCFrameHeaderDataType::c_FCE_RAW: {
      countedDHCFrames++;
      if (verbose) hw->print();
      if (isUnfiltered_event) {
        if (type == EDHCFrameHeaderDataType::c_ONSEN_FCE) m_errorMask |= EPXDErrMask::c_SENDALL_TYPE;
      } else {
        if (type == EDHCFrameHeaderDataType::c_FCE_RAW) m_errorMask |= EPXDErrMask::c_NOTSENDALL_TYPE;
      }

      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_direct_readout_frame_raw->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();

      B2INFO("UNPACK FCE FRAME with len $" << hex << len);
      unpack_fce((unsigned short*) data, len - 4, currentVxdId);

      break;
    };
    case EDHCFrameHeaderDataType::c_COMMODE: {
      countedDHCFrames++;

      if (verbose) hw->print();
      if (currentDHEID != dhc.data_commode_frame->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_commode_frame->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      break;
    };
    case EDHCFrameHeaderDataType::c_DHC_START: {
      countedBytesInDHC = 0;
      if (isFakedData_event != dhc.data_dhc_start_frame->isFakedData()) {
        B2ERROR("DHC START is but no Fake event OR Fake Event but DHE END is not.");
      }
      if (dhc.data_dhc_start_frame->isFakedData()) {
        B2WARNING("Faked DHC START Data -> trigger without Data!");
        m_errorMask |= EPXDErrMask::c_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhc.data_dhc_start_frame->print();
      }

//      eventNrOfOnsenTrgFrame = eventNrOfThisFrame;
      currentDHEID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      currentDHCID = dhc.data_dhc_start_frame->get_dhc_id();
      //nr_of_frames_dhc = dhc.data_dhc_start_frame->get_dhc_nr_frames();
      countedDHCFrames = 1;
      m_errorMask |= dhc.check_crc();
//       stat_start++;

      if (!isFakedData_event) {
        /// TODO here we should check full(!) Event Number, Run Number, Subrun Nr and Exp Number
        /// of this frame against the one from MEta Event Info
        if (dhc.data_dhc_start_frame->get_experiment() != m_meta_experiment) B2ERROR("DHC EXP MM: " <<
              dhc.data_dhc_start_frame->get_experiment() << " META " << m_meta_experiment);
        if (dhc.data_dhc_start_frame->get_run() != m_meta_run_nr) B2ERROR("DHC RUN MM: " << dhc.data_dhc_start_frame->get_run() << " META "
              << m_meta_run_nr);
        if (dhc.data_dhc_start_frame->get_subrun() != m_meta_subrun_nr) B2ERROR("DHC SUBRUN MM: " << dhc.data_dhc_start_frame->get_subrun()
              << " META " << m_meta_subrun_nr);
        if ((((unsigned int)dhc.data_dhc_start_frame->getEventNrHi() << 16) | dhc.data_dhc_start_frame->getEventNrLo()) !=
            (m_meta_event_nr & 0xFFFFFFFF)) {
          B2ERROR("DHC EVT32b MM: " << ((dhc.data_dhc_start_frame->getEventNrHi() << 16) | dhc.data_dhc_start_frame->getEventNrLo()) <<
                  " META " << (unsigned int)(m_meta_event_nr & 0xFFFFFFFF));
          m_errorMask |= EPXDErrMask::c_META_MM_DHC;
        }
        uint32_t tt = (((unsigned int)dhc.data_dhc_start_frame->time_tag_mid & 0x7FFF) << 12) | ((unsigned int)
                      dhc.data_dhc_start_frame->time_tag_lo_and_type >> 4);
        uint32_t mm = (unsigned int)((m_meta_time % 1000000000ull) * 0.127216 + 0.5);
        // uint64_t cc = (unsigned int)(m_meta_time / 1000000000ull);
        // B2ERROR("Meta / 1e9: " << hex << cc << " Diff: " << (dhc.data_dhc_start_frame->time_tag_hi-cc));
        if ((tt - mm) != 0) {
          if (!m_ignoreMetaFlags) {
            B2ERROR("DHC TT: $" << hex << dhc.data_dhc_start_frame->time_tag_hi << "." << dhc.data_dhc_start_frame->time_tag_mid << "." <<
                    dhc.data_dhc_start_frame->time_tag_lo_and_type << " META " << m_meta_time << " TRG Type " <<
                    (dhc.data_dhc_start_frame->time_tag_lo_and_type & 0xF));
            B2ERROR("Meta ns from 127MHz: " << hex << mm << " Diff: " << (tt - mm));
          }
        } else {
          B2INFO("DHC TT: $" << hex << dhc.data_dhc_start_frame->time_tag_hi << "." << dhc.data_dhc_start_frame->time_tag_mid << "." <<
                 dhc.data_dhc_start_frame->time_tag_lo_and_type << " META " << m_meta_time << " TRG Type " <<
                 (dhc.data_dhc_start_frame->time_tag_lo_and_type & 0xF));
        }
      }
      mask_active_dhe = dhc.data_dhc_start_frame->get_active_dhe_mask();
      nr_active_dhe = nr5bits(mask_active_dhe);
      break;
    };
    case EDHCFrameHeaderDataType::c_DHE_START: {
      countedBytesInDHE = 0;
      last_dhp_readout_frame_lo[0] = -1;
      last_dhp_readout_frame_lo[1] = -1;
      last_dhp_readout_frame_lo[2] = -1;
      last_dhp_readout_frame_lo[3] = -1;
      countedDHCFrames++;
      if (verbose)dhc.data_dhe_start_frame->print();
      dhe_first_readout_frame_id_lo = dhc.data_dhe_start_frame->getStartFrameNr();
      dhe_first_offset = dhc.data_dhe_start_frame->getTriggerOffsetRow();
      if (currentDHEID != 0xFFFFFFFF && (currentDHEID & 0xFFFF) >= dhc.data_dhe_start_frame->getDHEId()) {
        B2ERROR("DHH IDs are not in expected order! " << (currentDHEID & 0xFFFF) << " >= " << dhc.data_dhe_start_frame->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_WRONG_ID_SEQ;
      }
      currentDHEID = dhc.data_dhe_start_frame->getDHEId();
      m_errorMask |= dhc.check_crc();

      if (countedDHEStartFrames != countedDHEEndFrames) {
        B2ERROR("EDHCFrameHeaderDataType::c_DHE_START without EDHCFrameHeaderDataType::c_DHE_END");
        m_errorMask |= EPXDErrMask::c_DHE_START_WO_END;
      }
      countedDHEStartFrames++;

      found_mask_active_dhp = 0;
      mask_active_dhp = dhc.data_dhe_start_frame->getActiveDHPMask();
      ///      nr_active_dhp = nr5bits(mask_active_dhp);// unused

      /// ATTENTION seems to the Hi Word is not set!!!!
//       if ((((unsigned int)dhc.data_dhe_start_frame->getEventNrHi() << 16) | dhc.data_dhe_start_frame->getEventNrLo()) != (unsigned int)(
//             m_meta_event_nr & 0x0000FFFF)) {
//         B2ERROR("DHE EVT32b: " << ((dhc.data_dhe_start_frame->getEventNrHi() << 16) |
//                                    dhc.data_dhe_start_frame->getEventNrLo()) << " META "              << (m_meta_event_nr & 0xFFFFFFFF));
//           m_errorMask |= EPXDErrMask::c_META_MM_DHE;
//       } else
      if ((((unsigned int)dhc.data_dhe_start_frame->getEventNrHi() << 16) | dhc.data_dhe_start_frame->getEventNrLo()) != (unsigned int)(
            m_meta_event_nr & 0xFFFFFFFF)) {
        B2ERROR("DHE EVT32b (HI WORD): " << ((dhc.data_dhe_start_frame->getEventNrHi() << 16) | dhc.data_dhe_start_frame->getEventNrLo()) <<
                " META " << (m_meta_event_nr & 0xFFFFFFFF));
        m_errorMask |= EPXDErrMask::c_META_MM_DHE;
      }
//        B2ERROR("DHE TT: $" << hex << dhc.data_dhe_start_frame->dhe_time_tag_hi << "." << dhc.data_dhe_start_frame->dhe_time_tag_lo <<
//                " META " << m_meta_time);

      if (currentDHEID == 0) {
        B2WARNING("DHE ID is invalid=0 (not initialized)");
        m_errorMask |= EPXDErrMask::c_DHE_ID_INVALID;
      }
      // calculate the VXDID for DHE and save them for DHP unpacking
      {
        /// refering to BelleII Note Nr 0010, the numbers run from ... to
        ///   unsigned int layer, ladder, sensor;
        ///   layer= vxdid.getLayerNumber();/// 1 ... 2
        ///   ladder= vxdid.getLadderNumber();/// 1 ... 8 and 1 ... 12
        ///   sensor= vxdid.getSensorNumber();/// 1 ... 2
        ///   dhe_id = ((layer-1)<<5) | ((ladder)<<1) | (sensor-1);
        unsigned short sensor, ladder, layer;
        sensor = (currentDHEID & 0x1) + 1;
        ladder = (currentDHEID & 0x1E) >> 1; // no +1
        layer = ((currentDHEID & 0x20) >> 5) + 1;
        currentVxdId = VxdID(layer, ladder, sensor);
        if (ladder == 0 || (layer == 1 && ladder > 8) || (layer == 2 && ladder > 12)) {
          B2WARNING("DHE ID is invalid = $" << hex << currentDHEID << ", " << dec << currentDHEID << " (" << layer << "," << ladder << "," <<
                    sensor << ")");
          m_errorMask |= EPXDErrMask::c_DHE_ID_INVALID;
        }
      }

      break;
    };
    case EDHCFrameHeaderDataType::c_GHOST:
      countedDHCFrames++;
      if (verbose)dhc.data_ghost_frame->print();
      if (currentDHEID != dhc.data_ghost_frame->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_ghost_frame->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_ID;
      }
      /// Attention: Firmware might be changed such, that ghostframe come for all DHPs, not only active ones...
      found_mask_active_dhp |= 1 << dhc.data_ghost_frame->getDHPPort();

      //found_mask_active_dhp = mask_active_dhp;/// TODO Workaround for DESY TB 2016 doesnt work

      m_errorMask |= dhc.check_crc();
//       stat_ghost++;

      break;
    case EDHCFrameHeaderDataType::c_DHC_END: {
      if (dhc.data_dhc_end_frame->isFakedData() != isFakedData_event) {
        B2ERROR("DHC END is but no Fake event OR Fake Event but DHE END is not.");
      }
      countedDHCFrames++;
      if (isFakedData_event) {
        B2WARNING("Faked DHC END Data -> trigger without Data!");
        m_errorMask |= EPXDErrMask::c_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhc.data_dhc_end_frame->print();
      }
//       stat_end++;

      if (!isFakedData_event) {
        if (dhc.data_dhc_end_frame->get_dhc_id() != currentDHCID) {
          m_errorMask |= EPXDErrMask::c_DHC_DHCID_START_END_MM;
          B2ERROR("DHC ID Mismatch between Start and End $" << std::hex << currentDHCID << "!=$" << dhc.data_dhc_end_frame->get_dhc_id());
        }
//         if (countedDHCFrames != nr_of_frames_dhc) { /// Nr Frames has been removed .. tbd
//           if (!m_ignore_headernrframes) B2ERROR("Number of DHC Frames in Header(??? Tail?) " << nr_of_frames_dhc << " != " << countedDHCFrames << " Counted");
//           m_errorMask |= EPXDErrMask::c_DHC_FRAMECOUNT;
//         }
        int w;
        w = dhc.data_dhc_end_frame->get_words() * 4;
        if (countedBytesInDHC >= 0) {
          if (countedBytesInDHC != w) {
            B2ERROR("Error: WIE $" << hex << countedBytesInDHC << " != DHC END $" << hex << w);
            m_errorMask |= EPXDErrMask::c_DHC_WIE;
          } else {
            if (verbose)
              B2INFO("EVT END: WIE $" << hex << countedBytesInDHC << " == DHC END $" << hex << w);
          }
          // else ... processed data -> length invalid
        }
      }
      m_errorMask |= dhc.check_crc();
      m_errorMaskDHC |= m_errorMask; // do latest updates
      PXDDAQDHCStatus daqdhcstat(currentDHCID, m_errorMaskDHC, dhc.data_dhc_end_frame->get_words() * 4, countedBytesInDHC);
      daqdhcstat.addDHEs(daqdhevect);
      daqdhevect.clear();
      daqpktstat.addDHC(daqdhcstat);
//       m_storeDHCStats.appendNew(currentDHCID, m_errorMaskDHC, dhc.data_dhc_end_frame->get_words() * 4,countedBytesInDHC);
      m_errorMaskDHC = 0;
      currentDHEID = 0xFFFFFFFF;
      currentDHCID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      break;
    };
    case EDHCFrameHeaderDataType::c_DHE_END: {
      countedDHCFrames++;
      if (verbose) dhc.data_dhe_end_frame->print();
      if (currentDHEID != dhc.data_dhe_end_frame->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_dhe_end_frame->getDHEId());
        m_errorMask |= EPXDErrMask::c_DHE_START_END_ID;
      }
      m_errorMask |= dhc.check_crc();
      if (found_mask_active_dhp != mask_active_dhp) {
        B2WARNING("DHE_END: DHP active mask $" << hex << mask_active_dhp << " != $" << hex << found_mask_active_dhp <<
                  " mask of found dhp/ghost frames");
        m_errorMask |= EPXDErrMask::c_DHP_ACTIVE;
      }
      countedDHEEndFrames++;
      if (countedDHEStartFrames != countedDHEEndFrames) {
        B2ERROR("DHE_END without DHE_START");
        m_errorMask |= EPXDErrMask::c_DHE_END_WO_START;
      }
      {
        int w;
        w = dhc.data_dhe_end_frame->get_words() * 2;
        if (countedBytesInDHE >= 0) {
          if (countedBytesInDHE != w) {
            B2ERROR("Error: WIE $" << hex << countedBytesInDHE << " != DHE END $" << hex << w);
            m_errorMask |= EPXDErrMask::c_DHE_WIE;
          } else {
            if (verbose)
              B2INFO("EVT END: WIE $" << hex << countedBytesInDHE << " == DHE END $" << hex << w);
          }
          // else ... processed data -> length invalid
        }
      }
      m_errorMaskDHE |= m_errorMask; // do latest updates

      PXDDAQDHEStatus daqdhestat(currentVxdId, currentDHEID, m_errorMaskDHE, dhc.data_dhe_end_frame->get_words() * 2, countedBytesInDHE,
                                 dhe_first_offset, dhe_first_readout_frame_id_lo
                                );
      daqdhevect.push_back(daqdhestat);
//       m_storeDHEStats.appendNew(currentVxdId, currentDHEID, m_errorMaskDHE);
      m_errorMaskDHE = 0;
      currentDHEID |= 0xFF000000;// differenciate from 0xFFFFFFFFF as initial value
      currentVxdId = 0; /// invalid
      break;
    };
    case EDHCFrameHeaderDataType::c_ONSEN_ROI:
      countedDHCFrames += 0; /// DO NOT COUNT!!!!
      if (verbose) dhc.data_onsen_roi_frame->print();
      m_errorMask |= dhc.data_onsen_roi_frame->check_error(len);
      m_errorMask |= dhc.data_onsen_roi_frame->check_inner_crc(len - 4); /// CRC is without the DHC header, see reason in function
      m_errorMask |= dhc.check_crc();
      if (!m_doNotStore) dhc.data_onsen_roi_frame->save(m_storeROIs, len, (unsigned int*) data);
      break;
    case EDHCFrameHeaderDataType::c_ONSEN_TRG:
      countedDHCFrames += 0; /// DO NOT COUNT!!!!
      eventNrOfOnsenTrgFrame = eventNrOfThisFrame;
      if (dhc.data_onsen_trigger_frame->get_trig_nr1() != (unsigned int)(m_meta_event_nr & 0xFFFFFFFF))
        B2ERROR("TRG HLT MM: $" << dhc.data_onsen_trigger_frame->get_trig_nr1() << " META " <<
                (m_meta_event_nr & 0xFFFFFFFF));
      if (dhc.data_onsen_trigger_frame->get_experiment1() != m_meta_experiment) {
        if (!m_ignoreMetaFlags) {
          B2ERROR("TRG HLT EXP MM: $" << dhc.data_onsen_trigger_frame->get_experiment1() << " META " <<
                  m_meta_experiment);
        }
      }
      if (dhc.data_onsen_trigger_frame->get_run1() != m_meta_run_nr)
        B2ERROR("TRG HLT RUN MM: $" << dhc.data_onsen_trigger_frame->get_run1() << " META " <<
                m_meta_run_nr);
      if (dhc.data_onsen_trigger_frame->get_subrun1() != m_meta_subrun_nr)
        B2ERROR("TRG HLT SUBRUN MM: $" << dhc.data_onsen_trigger_frame->get_subrun1() << " META " <<
                m_meta_subrun_nr);

      if (!dhc.data_onsen_trigger_frame->is_fake_datcon()) {
        if (dhc.data_onsen_trigger_frame->get_trig_nr2() != (unsigned int)(m_meta_event_nr & 0xFFFFFFFF))
          B2ERROR("TRG DC MM: $" << dhc.data_onsen_trigger_frame->get_trig_nr2() << " META " <<
                  (m_meta_event_nr & 0xFFFFFFFF));
        if (dhc.data_onsen_trigger_frame->get_experiment2() != m_meta_experiment) {
          if (!m_ignoreMetaFlags) {
            B2ERROR("TRG DC EXP MM: $" << dhc.data_onsen_trigger_frame->get_experiment2() << " META " <<
                    m_meta_experiment);
          }
        }
        if (dhc.data_onsen_trigger_frame->get_run2() != m_meta_run_nr)
          B2ERROR("TRG DC RUN MM: $" << dhc.data_onsen_trigger_frame->get_run2() << " META " <<
                  m_meta_run_nr);
        if (dhc.data_onsen_trigger_frame->get_subrun2() != m_meta_subrun_nr)
          B2ERROR("TRG DC SUBRUN MM: $" << dhc.data_onsen_trigger_frame->get_subrun2() << " META " <<
                  m_meta_subrun_nr);
      }

//       B2ERROR("TRG TAG HLT: $" << hex << dhc.data_onsen_trigger_frame->get_trig_tag1() << " DATCON $" <<  dhc.data_onsen_trigger_frame->get_trig_tag2() << " META " << m_meta_time);

      if (verbose) dhc.data_onsen_trigger_frame->print();
      m_errorMask |= dhc.data_onsen_trigger_frame->check_error(ignore_datcon_flag);
      m_errorMask |= dhc.check_crc();
      if (Frame_Number != 0) {
        B2ERROR("ONSEN TRG Frame must be the first one.");
      }
      isUnfiltered_event = dhc.data_onsen_trigger_frame->is_SendUnfiltered();
      if (isUnfiltered_event) m_sendunfiltered++;
      if (dhc.data_onsen_trigger_frame->is_SendROIs()) m_sendrois++;
      if (!dhc.data_onsen_trigger_frame->is_Accepted()) m_notaccepted++;
      break;
    default:
      B2ERROR("UNKNOWN DHC frame type");
      m_errorMask |= EPXDErrMask::c_DHC_UNKNOWN;
//      type_error++;
      if (verbose) hw->print();
//       error_flag = true;
      break;
  }

  if (eventNrOfThisFrame != eventNrOfOnsenTrgFrame && !isFakedData_event) {
    B2ERROR("Frame TrigNr != ONSEN Trig Nr $" << hex << eventNrOfThisFrame << " != $" << eventNrOfOnsenTrgFrame);
    m_errorMask |= EPXDErrMask::c_FRAME_TNR_MM;
//     evtnr_error++;
//     error_flag = true;
  }

  if (Frame_Number == 0) {
    /// Check that DHC Start is first Frame
    if (type != EDHCFrameHeaderDataType::c_ONSEN_TRG) {
      B2ERROR("First frame is not a ONSEN Trigger frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_ONSEN_TRG_FIRST;
    }
  } else { // (Frame_Number != 0 &&
    /// Check that there is no other DHC Start
    if (type == EDHCFrameHeaderDataType::c_ONSEN_TRG) {
      B2ERROR("More than one ONSEN Trigger frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_ONSEN_TRG_FIRST;
    }
  }

  if (Frame_Number == 1) {
    /// Check that DHC Start is first Frame
    if (type != EDHCFrameHeaderDataType::c_DHC_START) {
      B2ERROR("Second frame is not a DHC start of subevent frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_DHC_START_SECOND;
    }
  } else { // (Frame_Number != 0 &&
    /// Check that there is no other DHC Start
    if (type == EDHCFrameHeaderDataType::c_DHC_START) {
      B2ERROR("More than one DHC start of subevent frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_DHC_START_SECOND;
    }
  }

  if (Frame_Number == Frames_in_event - 1) {
    /// Check that DHC End is last Frame
    if (type != EDHCFrameHeaderDataType::c_DHC_END) {
      B2ERROR("Last frame is not a DHC end of subevent frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_DHC_END_MISS;
    }

    /// As we now have processed the whole event, we can do some more consistency checks!
    if (countedDHEStartFrames != countedDHEEndFrames || countedDHEStartFrames != nr_active_dhe) {
      B2ERROR("The number of DHE Start/End does not match the number of active DHE in DHC Header! Header: " << nr_active_dhe <<
              " Start: " << countedDHEStartFrames << " End: " << countedDHEEndFrames << " Mask: $" << hex << mask_active_dhe << " in Event Nr " <<
              eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_DHE_ACTIVE;
    }

  } else { //  (Frame_Number != Frames_in_event - 1 &&
    /// Check that there is no other DHC End
    if (type == EDHCFrameHeaderDataType::c_DHC_END) {
      B2ERROR("More than one DHC end of subevent frame in frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= EPXDErrMask::c_DHC_END_DBL;
    }
  }

  /// Check that (if there is at least one active DHE) the second Frame is DHE Start, actually this is redundant if the other checks work
  if (Frame_Number == 2 && nr_active_dhe != 0 && type != EDHCFrameHeaderDataType::c_DHE_START) {
    B2ERROR("Third frame is not a DHE start frame in Event Nr " << eventNrOfThisFrame);
    m_errorMask |= EPXDErrMask::c_DHE_START_MISS;
  }

  if (type != EDHCFrameHeaderDataType::c_ONSEN_ROI  && type != EDHCFrameHeaderDataType::c_ONSEN_TRG) {
    // actually, they should not be withing Start and End, but better be sure.
    countedBytesInDHC += len;
    countedBytesInDHE += len;
  }
  B2DEBUG(20, "DHC/DHE $" << hex << countedBytesInDHC << ", $" << hex << countedBytesInDHE);
}

//Remaps of inner forward (IF) and outer backward (OB) modules of the PXD
void PXDUnpackerBSModule::remap_IF_OB(unsigned int& v_cellID, unsigned int& u_cellID, unsigned int dhp_id, unsigned int dhe_ID)
{
#if 0
  unsigned int uu = u_cellID, vv = v_cellID;

  {
    unsigned int ch = (u_cellID << 2) + (v_cellID & 0x3);
    unsigned int DCD_col = u_cellID >> 2;
    unsigned int DCD_col_sw = 0xf & ~DCD_col;
    unsigned int drain = (ch & 0xF) + (DCD_col_sw << 4) + 256 * dhp_id;// or 250??

    if (ch >= 10 and ch <= 15) drain = 1023;

    u_cellID = drain >> 2;
    unsigned int row = (v_cellID & ~0x3)  + (drain & 0x3);
    if ((dhe_ID & 0x20) == 0) {//if inner module
      v_cellID = 768 - 1 - row ;
    } else {//if outer module
      v_cellID = row ;
    }
  }

  unsigned int uu1 = u_cellID, vv1 = v_cellID;
  u_cellID = uu;
  v_cellID = vv;
#endif
  {
    unsigned int DCD_channel = 0;
    unsigned int Drain = 0;
    unsigned int row = 0;
    std::vector <unsigned int> LUT_IF_OB = {
      240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 1023, 1023, 1023, 1023, 1023, 1023, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
      208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
      176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
      144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
      112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
      80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
      48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
      16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
      490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 1023, 1023, 1023, 1023, 1023, 1023, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489,
      458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457,
      426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425,
      394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393,
      362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361,
      330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329,
      298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297,
      266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265,
      740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 1023, 1023, 1023, 1023, 1023, 1023, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739,
      708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707,
      676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675,
      644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 628, 629, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 641, 642, 643,
      612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611,
      580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579,
      548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547,
      516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515,
      990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1023, 1023, 1023, 1023, 1023, 1023, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989,
      958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957,
      926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925,
      894, 895, 896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 878, 879, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893,
      862, 863, 864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861,
      830, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829,
      798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797,
      766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765
    };

    // B2INFO("Remapped :: From COL $" << u_cellID << " ROW $" << v_cellID);
    DCD_channel = (u_cellID << 2) + (v_cellID & 0x3) + 256 * dhp_id;
    Drain = LUT_IF_OB[DCD_channel];
    u_cellID = Drain >> 2;
    row = (v_cellID & ~0x3)  + (Drain & 0x3); // no ~ bei drain
    row = (row + m_DESY16_FixRowOffset) % 768;
    if ((dhe_ID & 0x20) == 0) {//if inner module
      v_cellID = 768 - 1 - row ;

    } else {//if outer module
      v_cellID = row ;
    }
  }
// if(uu1!=u_cellID || vv1!=v_cellID){
//   B2ERROR("Remapped (1) : " <<uu <<"," <<vv << " to " <<uu1 <<"," <<vv1<<" and "<< u_cellID << "," << v_cellID);
// }

//     u_cellID=uu1;
//     v_cellID=vv1;

#if 0
  unsigned int ch = 4 * u_cellID + v_cellID % 4;
  unsigned int DCD_col = ch / 16;
  unsigned int DCD_col_sw = 15 - DCD_col;
  unsigned int  drain = ch % 16 + DCD_col_sw * 16 + 250 * dhp_id;

  if (ch >= 10 and ch <= 15) drain = 1023;

  u_cellID = drain / 4;
  unsigned int row = (v_cellID / 4) * 4  + drain % 4;
  if (((dhe_ID >> 5) & 0x1) == 0) {//if inner module
    v_cellID = 768 - 1 - row ;
  } else {//if outer module
    v_cellID = row ;
  }
#endif
#if 0
  unsigned int DCD_channel = 0;
  unsigned int Drain = 0;
  unsigned int row = 0;
  std::vector <unsigned int> LUT_IF_OB = {
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 1023, 1023, 1023, 1023, 1023, 1023, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 1023, 1023, 1023, 1023, 1023, 1023, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489,
    458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457,
    426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425,
    394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393,
    362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361,
    330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329,
    298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297,
    266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265,
    740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 1023, 1023, 1023, 1023, 1023, 1023, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739,
    708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707,
    676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675,
    644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 628, 629, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 641, 642, 643,
    612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611,
    580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579,
    548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547,
    516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515,
    990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1023, 1023, 1023, 1023, 1023, 1023, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989,
    958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957,
    926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925,
    894, 895, 896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 878, 879, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893,
    862, 863, 864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861,
    830, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829,
    798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797,
    766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765
  };

  B2INFO("Remapped :: From COL $" << u_cellID << " ROW $" << v_cellID);
  DCD_channel = 4 * u_cellID + v_cellID % 4 + 256 * dhp_id;
  Drain = LUT_IF_OB[DCD_channel];
  B2INFO("in remap ROW ... DCD_channel :: " << DCD_channel << " DRAIN :: " << Drain);
  u_cellID = Drain / 4;
  row = (v_cellID / 4) * 4  + Drain % 4;
  if (((dhe_ID >> 5) & 0x1) == 0) {//if inner module
    v_cellID = 768 - 1 - row ;
  } else {//if outer module
    v_cellID = row ;
  }
#endif
//  B2INFO("Remapped ::To  COL COL $" << u_cellID << " ROW $" << v_cellID);
}

//Remaps of inner backward (IB) and outer forward (OF) modules of the PXD
void PXDUnpackerBSModule::remap_IB_OF(unsigned int& v_cellID, unsigned int& u_cellID, unsigned int dhp_id, unsigned int dhe_ID)
{
#if 0
  unsigned int uu = u_cellID, vv = v_cellID;

  {
    unsigned int ch = (u_cellID << 2) + (v_cellID & 0x3);
    unsigned int DCD_col = u_cellID >> 2;
    unsigned int DCD_col_sw = 0xF & ~DCD_col;
    //unsigned int ch_map = 4 * u_cellID + (3 - v_cellID) % 4;
    unsigned int ch_map = (u_cellID << 2) + (0x3 & ~v_cellID) ;
    unsigned int drain = (ch_map & 0xF) + (DCD_col_sw << 4) + 256 * dhp_id;// or 250??

    if (dhp_id == 1 or dhp_id == 3) {
      if (v_cellID / 2 == 0) {
        drain -= 4;
      } else {
        drain += 4;
      }
    }

    if (ch >= 10 and ch <= 15) drain = 1023;

    u_cellID = 250 - 1 - drain / 4;
    if (u_cellID >= 250) u_cellID = 255; // workaround for negative values!!!

//  unsigned int row = (v_cellID / 4) * 4  + drain % 4;
    unsigned int row = (v_cellID & ~0x3)  + (drain & 0x3);
//  if (((dhe_ID >> 5) & 0x1) == 0) { //if inner module
    if ((dhe_ID & 0x20) == 0) { //if inner module
      v_cellID = 768 - 1 - row ;
    } else { //if outer module
      v_cellID = row ;
    }

  }

  unsigned int uu1 = u_cellID, vv1 = v_cellID;
  u_cellID = uu;
  v_cellID = vv;
#endif
  {
    unsigned int DCD_channel = 0;
    unsigned int Drain = 0;
    unsigned int row = 0;
    std::vector <unsigned int> LUT_IB_OF = {
      243, 242, 241, 240, 247, 246, 245, 244, 251, 250, 1023, 1023, 1023, 1023, 1023, 1023, 227, 226, 225, 224, 231, 230, 229, 228, 235, 234, 233, 232, 239, 238, 237, 236,
      211, 210, 209, 208, 215, 214, 213, 212, 219, 218, 217, 216, 223, 222, 221, 220, 195, 194, 193, 192, 199, 198, 197, 196, 203, 202, 201, 200, 207, 206, 205, 204,
      179, 178, 177, 176, 183, 182, 181, 180, 187, 186, 185, 184, 191, 190, 189, 188, 163, 162, 161, 160, 167, 166, 165, 164, 171, 170, 169, 168, 175, 174, 173, 172,
      147, 146, 145, 144, 151, 150, 149, 148, 155, 154, 153, 152, 159, 158, 157, 156, 131, 130, 129, 128, 135, 134, 133, 132, 139, 138, 137, 136, 143, 142, 141, 140,
      115, 114, 113, 112, 119, 118, 117, 116, 123, 122, 121, 120, 127, 126, 125, 124, 99, 98, 97, 96, 103, 102, 101, 100, 107, 106, 105, 104, 111, 110, 109, 108,
      83, 82, 81, 80, 87, 86, 85, 84, 91, 90, 89, 88, 95, 94, 93, 92, 67, 66, 65, 64, 71, 70, 69, 68, 75, 74, 73, 72, 79, 78, 77, 76,
      51, 50, 49, 48, 55, 54, 53, 52, 59, 58, 57, 56, 63, 62, 61, 60, 35, 34, 33, 32, 39, 38, 37, 36, 43, 42, 41, 40, 47, 46, 45, 44,
      19, 18, 17, 16, 23, 22, 21, 20, 27, 26, 25, 24, 31, 30, 29, 28, 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12,
      489, 488, 495, 494, 493, 492, 499, 498, 497, 496, 1023, 1023, 1023, 1023, 1023, 1023, 473, 472, 479, 478, 477, 476, 483, 482, 481, 480, 487, 486, 485, 484, 491, 490,
      457, 456, 463, 462, 461, 460, 467, 466, 465, 464, 471, 470, 469, 468, 475, 474, 441, 440, 447, 446, 445, 444, 451, 450, 449, 448, 455, 454, 453, 452, 459, 458,
      425, 424, 431, 430, 429, 428, 435, 434, 433, 432, 439, 438, 437, 436, 443, 442, 409, 408, 415, 414, 413, 412, 419, 418, 417, 416, 423, 422, 421, 420, 427, 426,
      393, 392, 399, 398, 397, 396, 403, 402, 401, 400, 407, 406, 405, 404, 411, 410, 377, 376, 383, 382, 381, 380, 387, 386, 385, 384, 391, 390, 389, 388, 395, 394,
      361, 360, 367, 366, 365, 364, 371, 370, 369, 368, 375, 374, 373, 372, 379, 378, 345, 344, 351, 350, 349, 348, 355, 354, 353, 352, 359, 358, 357, 356, 363, 362,
      329, 328, 335, 334, 333, 332, 339, 338, 337, 336, 343, 342, 341, 340, 347, 346, 313, 312, 319, 318, 317, 316, 323, 322, 321, 320, 327, 326, 325, 324, 331, 330,
      297, 296, 303, 302, 301, 300, 307, 306, 305, 304, 311, 310, 309, 308, 315, 314, 281, 280, 287, 286, 285, 284, 291, 290, 289, 288, 295, 294, 293, 292, 299, 298,
      265, 264, 271, 270, 269, 268, 275, 274, 273, 272, 279, 278, 277, 276, 283, 282, 249, 248, 255, 254, 253, 252, 259, 258, 257, 256, 263, 262, 261, 260, 267, 266,
      743, 742, 741, 740, 747, 746, 745, 744, 751, 750, 1023, 1023, 1023, 1023, 1023, 1023, 727, 726, 725, 724, 731, 730, 729, 728, 735, 734, 733, 732, 739, 738, 737, 736,
      711, 710, 709, 708, 715, 714, 713, 712, 719, 718, 717, 716, 723, 722, 721, 720, 695, 694, 693, 692, 699, 698, 697, 696, 703, 702, 701, 700, 707, 706, 705, 704,
      679, 678, 677, 676, 683, 682, 681, 680, 687, 686, 685, 684, 691, 690, 689, 688, 663, 662, 661, 660, 667, 666, 665, 664, 671, 670, 669, 668, 675, 674, 673, 672,
      647, 646, 645, 644, 651, 650, 649, 648, 655, 654, 653, 652, 659, 658, 657, 656, 631, 630, 629, 628, 635, 634, 633, 632, 639, 638, 637, 636, 643, 642, 641, 640,
      615, 614, 613, 612, 619, 618, 617, 616, 623, 622, 621, 620, 627, 626, 625, 624, 599, 598, 597, 596, 603, 602, 601, 600, 607, 606, 605, 604, 611, 610, 609, 608,
      583, 582, 581, 580, 587, 586, 585, 584, 591, 590, 589, 588, 595, 594, 593, 592, 567, 566, 565, 564, 571, 570, 569, 568, 575, 574, 573, 572, 579, 578, 577, 576,
      551, 550, 549, 548, 555, 554, 553, 552, 559, 558, 557, 556, 563, 562, 561, 560, 535, 534, 533, 532, 539, 538, 537, 536, 543, 542, 541, 540, 547, 546, 545, 544,
      519, 518, 517, 516, 523, 522, 521, 520, 527, 526, 525, 524, 531, 530, 529, 528, 503, 502, 501, 500, 507, 506, 505, 504, 511, 510, 509, 508, 515, 514, 513, 512,
      989, 988, 995, 994, 993, 992, 999, 998, 997, 996, 1023, 1023, 1023, 1023, 1023, 1023, 973, 972, 979, 978, 977, 976, 983, 982, 981, 980, 987, 986, 985, 984, 991, 990,
      957, 956, 963, 962, 961, 960, 967, 966, 965, 964, 971, 970, 969, 968, 975, 974, 941, 940, 947, 946, 945, 944, 951, 950, 949, 948, 955, 954, 953, 952, 959, 958,
      925, 924, 931, 930, 929, 928, 935, 934, 933, 932, 939, 938, 937, 936, 943, 942, 909, 908, 915, 914, 913, 912, 919, 918, 917, 916, 923, 922, 921, 920, 927, 926,
      893, 892, 899, 898, 897, 896, 903, 902, 901, 900, 907, 906, 905, 904, 911, 910, 877, 876, 883, 882, 881, 880, 887, 886, 885, 884, 891, 890, 889, 888, 895, 894,
      861, 860, 867, 866, 865, 864, 871, 870, 869, 868, 875, 874, 873, 872, 879, 878, 845, 844, 851, 850, 849, 848, 855, 854, 853, 852, 859, 858, 857, 856, 863, 862,
      829, 828, 835, 834, 833, 832, 839, 838, 837, 836, 843, 842, 841, 840, 847, 846, 813, 812, 819, 818, 817, 816, 823, 822, 821, 820, 827, 826, 825, 824, 831, 830,
      797, 796, 803, 802, 801, 800, 807, 806, 805, 804, 811, 810, 809, 808, 815, 814, 781, 780, 787, 786, 785, 784, 791, 790, 789, 788, 795, 794, 793, 792, 799, 798,
      765, 764, 771, 770, 769, 768, 775, 774, 773, 772, 779, 778, 777, 776, 783, 782, 749, 748, 755, 754, 753, 752, 759, 758, 757, 756, 763, 762, 761, 760, 767, 766
    };

    DCD_channel = (u_cellID << 2) + (v_cellID & 0x3) + 256 * dhp_id;
    Drain = LUT_IB_OF[DCD_channel];
    u_cellID = 250 - 1 - (Drain >> 2);
    if (u_cellID >= 250) u_cellID = 255; // workaround for negative values!!! fix LUT above!
//   row = (v_cellID / 4) * 4  + Drain % 4;
    row = (v_cellID & ~0x3)  + ((~Drain) & 0x3); // ~ bei drain
    row = (row + m_DESY16_FixRowOffset) % 768;
    if ((dhe_ID  & 0x20) == 0) { //if inner module
      v_cellID = 768 - 1 - row ;
    } else { //if outer module
      v_cellID = row ;
    }
  }
// if(uu1!=u_cellID || vv1!=v_cellID){
//   B2ERROR("Remapped (2) : " <<uu <<"," <<vv << " to " <<uu1 <<"," <<vv1<<" and "<< u_cellID << "," << v_cellID);
// }

//     u_cellID=uu1;
//     v_cellID=vv1;

#if 0
  unsigned int ch = 4 * u_cellID + v_cellID % 4;
  unsigned int DCD_col = ch / 16;
  unsigned int DCD_col_sw = 15 - DCD_col;
  unsigned int ch_map = 4 * u_cellID + (3 - v_cellID) % 4;
  unsigned int drain = ch_map % 16 + DCD_col_sw * 16 + 250 * dhp_id;

  if (dhp_id == 1 or dhp_id == 3) {
    if (v_cellID / 2 == 0) {
      drain -= 4;
    } else {
      drain += 4;
    }
  }

  if (ch >= 10 and ch <= 15) drain = 1023;

  u_cellID = 250 - 1 - drain / 4;
  if (u_cellID >= 250) u_cellID = 255; // TODO workaround for negative values!!!

  unsigned int row = (v_cellID / 4) * 4  + drain % 4;
  if (((dhe_ID >> 5) & 0x1) == 0) { //if inner module
    v_cellID = 768 - 1 - row ;
  } else { //if outer module
    v_cellID = row ;
  }
#endif
#if 0
  unsigned int DCD_channel = 0;
  unsigned int Drain = 0;
  unsigned int row = 0;
  std::vector <unsigned int> LUT_IB_OF = {
    243, 242, 241, 240, 247, 246, 245, 244, 251, 250, 1023, 1023, 1023, 1023, 1023, 1023, 227, 226, 225, 224, 231, 230, 229, 228, 235, 234, 233, 232, 239, 238, 237, 236,
    211, 210, 209, 208, 215, 214, 213, 212, 219, 218, 217, 216, 223, 222, 221, 220, 195, 194, 193, 192, 199, 198, 197, 196, 203, 202, 201, 200, 207, 206, 205, 204,
    179, 178, 177, 176, 183, 182, 181, 180, 187, 186, 185, 184, 191, 190, 189, 188, 163, 162, 161, 160, 167, 166, 165, 164, 171, 170, 169, 168, 175, 174, 173, 172,
    147, 146, 145, 144, 151, 150, 149, 148, 155, 154, 153, 152, 159, 158, 157, 156, 131, 130, 129, 128, 135, 134, 133, 132, 139, 138, 137, 136, 143, 142, 141, 140,
    115, 114, 113, 112, 119, 118, 117, 116, 123, 122, 121, 120, 127, 126, 125, 124, 99, 98, 97, 96, 103, 102, 101, 100, 107, 106, 105, 104, 111, 110, 109, 108,
    83, 82, 81, 80, 87, 86, 85, 84, 91, 90, 89, 88, 95, 94, 93, 92, 67, 66, 65, 64, 71, 70, 69, 68, 75, 74, 73, 72, 79, 78, 77, 76,
    51, 50, 49, 48, 55, 54, 53, 52, 59, 58, 57, 56, 63, 62, 61, 60, 35, 34, 33, 32, 39, 38, 37, 36, 43, 42, 41, 40, 47, 46, 45, 44,
    19, 18, 17, 16, 23, 22, 21, 20, 27, 26, 25, 24, 31, 30, 29, 28, 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12,
    489, 488, 495, 494, 493, 492, 499, 498, 497, 496, 1023, 1023, 1023, 1023, 1023, 1023, 473, 472, 479, 478, 477, 476, 483, 482, 481, 480, 487, 486, 485, 484, 491, 490,
    457, 456, 463, 462, 461, 460, 467, 466, 465, 464, 471, 470, 469, 468, 475, 474, 441, 440, 447, 446, 445, 444, 451, 450, 449, 448, 455, 454, 453, 452, 459, 458,
    425, 424, 431, 430, 429, 428, 435, 434, 433, 432, 439, 438, 437, 436, 443, 442, 409, 408, 415, 414, 413, 412, 419, 418, 417, 416, 423, 422, 421, 420, 427, 426,
    393, 392, 399, 398, 397, 396, 403, 402, 401, 400, 407, 406, 405, 404, 411, 410, 377, 376, 383, 382, 381, 380, 387, 386, 385, 384, 391, 390, 389, 388, 395, 394,
    361, 360, 367, 366, 365, 364, 371, 370, 369, 368, 375, 374, 373, 372, 379, 378, 345, 344, 351, 350, 349, 348, 355, 354, 353, 352, 359, 358, 357, 356, 363, 362,
    329, 328, 335, 334, 333, 332, 339, 338, 337, 336, 343, 342, 341, 340, 347, 346, 313, 312, 319, 318, 317, 316, 323, 322, 321, 320, 327, 326, 325, 324, 331, 330,
    297, 296, 303, 302, 301, 300, 307, 306, 305, 304, 311, 310, 309, 308, 315, 314, 281, 280, 287, 286, 285, 284, 291, 290, 289, 288, 295, 294, 293, 292, 299, 298,
    265, 264, 271, 270, 269, 268, 275, 274, 273, 272, 279, 278, 277, 276, 283, 282, 249, 248, 255, 254, 253, 252, 259, 258, 257, 256, 263, 262, 261, 260, 267, 266,
    743, 742, 741, 740, 747, 746, 745, 744, 751, 750, 1023, 1023, 1023, 1023, 1023, 1023, 727, 726, 725, 724, 731, 730, 729, 728, 735, 734, 733, 732, 739, 738, 737, 736,
    711, 710, 709, 708, 715, 714, 713, 712, 719, 718, 717, 716, 723, 722, 721, 720, 695, 694, 693, 692, 699, 698, 697, 696, 703, 702, 701, 700, 707, 706, 705, 704,
    679, 678, 677, 676, 683, 682, 681, 680, 687, 686, 685, 684, 691, 690, 689, 688, 663, 662, 661, 660, 667, 666, 665, 664, 671, 670, 669, 668, 675, 674, 673, 672,
    647, 646, 645, 644, 651, 650, 649, 648, 655, 654, 653, 652, 659, 658, 657, 656, 631, 630, 629, 628, 635, 634, 633, 632, 639, 638, 637, 636, 643, 642, 641, 640,
    615, 614, 613, 612, 619, 618, 617, 616, 623, 622, 621, 620, 627, 626, 625, 624, 599, 598, 597, 596, 603, 602, 601, 600, 607, 606, 605, 604, 611, 610, 609, 608,
    583, 582, 581, 580, 587, 586, 585, 584, 591, 590, 589, 588, 595, 594, 593, 592, 567, 566, 565, 564, 571, 570, 569, 568, 575, 574, 573, 572, 579, 578, 577, 576,
    551, 550, 549, 548, 555, 554, 553, 552, 559, 558, 557, 556, 563, 562, 561, 560, 535, 534, 533, 532, 539, 538, 537, 536, 543, 542, 541, 540, 547, 546, 545, 544,
    519, 518, 517, 516, 523, 522, 521, 520, 527, 526, 525, 524, 531, 530, 529, 528, 503, 502, 501, 500, 507, 506, 505, 504, 511, 510, 509, 508, 515, 514, 513, 512,
    989, 988, 995, 994, 993, 992, 999, 998, 997, 996, 1023, 1023, 1023, 1023, 1023, 1023, 973, 972, 979, 978, 977, 976, 983, 982, 981, 980, 987, 986, 985, 984, 991, 990,
    957, 956, 963, 962, 961, 960, 967, 966, 965, 964, 971, 970, 969, 968, 975, 974, 941, 940, 947, 946, 945, 944, 951, 950, 949, 948, 955, 954, 953, 952, 959, 958,
    925, 924, 931, 930, 929, 928, 935, 934, 933, 932, 939, 938, 937, 936, 943, 942, 909, 908, 915, 914, 913, 912, 919, 918, 917, 916, 923, 922, 921, 920, 927, 926,
    893, 892, 899, 898, 897, 896, 903, 902, 901, 900, 907, 906, 905, 904, 911, 910, 877, 876, 883, 882, 881, 880, 887, 886, 885, 884, 891, 890, 889, 888, 895, 894,
    861, 860, 867, 866, 865, 864, 871, 870, 869, 868, 875, 874, 873, 872, 879, 878, 845, 844, 851, 850, 849, 848, 855, 854, 853, 852, 859, 858, 857, 856, 863, 862,
    829, 828, 835, 834, 833, 832, 839, 838, 837, 836, 843, 842, 841, 840, 847, 846, 813, 812, 819, 818, 817, 816, 823, 822, 821, 820, 827, 826, 825, 824, 831, 830,
    797, 796, 803, 802, 801, 800, 807, 806, 805, 804, 811, 810, 809, 808, 815, 814, 781, 780, 787, 786, 785, 784, 791, 790, 789, 788, 795, 794, 793, 792, 799, 798,
    765, 764, 771, 770, 769, 768, 775, 774, 773, 772, 779, 778, 777, 776, 783, 782, 749, 748, 755, 754, 753, 752, 759, 758, 757, 756, 763, 762, 761, 760, 767, 766
  };

  B2INFO("Remapped :: From COL $" << u_cellID << " ROW $" << v_cellID);
  DCD_channel = 4 * u_cellID + v_cellID % 4 + 256 * dhp_id;
  Drain = LUT_IB_OF[DCD_channel];
  B2INFO("in remap ... DCD_channel :: " << DCD_channel << " DRAIN :: " << Drain);
  u_cellID = 250 - 1 - Drain / 4;
  if (u_cellID >= 250) u_cellID = 255; // TODO workaround for negative values!!!

  row = (v_cellID / 4) * 4  + Drain % 4;
  if (((dhe_ID >> 5) & 0x1) == 0) { //if inner module
    v_cellID = 768 - 1 - row ;
  } else { //if outer module
    v_cellID = row ;
  }
#endif
//  B2INFO("Remapped ::To  COL COL $" << u_cellID << " ROW $" << v_cellID);
}

void PXDUnpackerBSModule::test_mapping(void)
{
  FILE* file = fopen("lut.csv", "wt+");
  if (file) {
    for (unsigned int row = 0; row < 4; ++row) {
      for (unsigned int dhp_id = 0; dhp_id < 4; ++dhp_id) {
        for (unsigned int col = 0; col < 64; ++col) {
          unsigned int coli;
          unsigned int rowi;
          coli = col;
          rowi = row;
          remap_IF_OB(rowi,   coli,   dhp_id, 0x00);
          fprintf(file, "%d; %d; %d; %d; %d; " , row, dhp_id, col, rowi, coli);
          coli = col;
          rowi = row;
          remap_IB_OF(rowi,  coli,  dhp_id, 0x00);
          fprintf(file, "%d; %d\n" , rowi, coli);
        }
      }
    }

    for (unsigned int row = 0; row < 4; ++row) {
      for (unsigned int dhp_id = 0; dhp_id < 4; ++dhp_id) {
        for (unsigned int col = 0; col < 64; ++col) {
          unsigned int coli;
          unsigned int rowi;
          coli = col;
          rowi = row;
          remap_IF_OB(rowi,   coli,   dhp_id, 0x01);
          fprintf(file, "%d; %d; %d; %d; %d; " , row, dhp_id, col, rowi, coli);
          coli = col;
          rowi = row;
          remap_IB_OF(rowi,  coli,  dhp_id, 0x01);
          fprintf(file, "%d; %d\n" , rowi, coli);
        }
      }
    }


    for (unsigned int row = 0; row < 4; ++row) {
      for (unsigned int dhp_id = 0; dhp_id < 4; ++dhp_id) {
        for (unsigned int col = 0; col < 64; ++col) {
          unsigned int coli;
          unsigned int rowi;
          coli = col;
          rowi = row;
          remap_IF_OB(rowi,   coli,   dhp_id, 0x20);
          fprintf(file, "%d; %d; %d; %d; %d; " , row, dhp_id, col, rowi, coli);
          coli = col;
          rowi = row;
          remap_IB_OF(rowi,  coli,  dhp_id, 0x20);
          fprintf(file, "%d; %d\n" , rowi, coli);
        }
      }
    }


    for (unsigned int row = 0; row < 4; ++row) {
      for (unsigned int dhp_id = 0; dhp_id < 4; ++dhp_id) {
        for (unsigned int col = 0; col < 64; ++col) {
          unsigned int coli;
          unsigned int rowi;
          coli = col;
          rowi = row;
          remap_IF_OB(rowi,   coli,   dhp_id, 0x21);
          fprintf(file, "%d; %d; %d; %d; %d; " , row, dhp_id, col, rowi, coli);
          coli = col;
          rowi = row;
          remap_IB_OF(rowi,  coli,  dhp_id, 0x21);
          fprintf(file, "%d; %d\n" , rowi, coli);
        }
      }
    }

    fclose(file);
  }
}
