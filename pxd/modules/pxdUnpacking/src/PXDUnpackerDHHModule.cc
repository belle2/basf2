/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdUnpacking/PXDUnpackerDHHModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/spirit/home/support/detail/endian.hpp>

// DHP modes are the same as for DHE envelope
#define DHP_FRAME_HEADER_DATA_TYPE_RAW  0x0
#define DHP_FRAME_HEADER_DATA_TYPE_ZSD  0x5

// DHE like before, but now 4 bits
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW     0x0
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD     0x5
#define DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW     0x1 //CLUSTER FRAME
#define DHC_FRAME_HEADER_DATA_TYPE_COMMODE     0x6
#define DHC_FRAME_HEADER_DATA_TYPE_GHOST       0x2
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_START   0x3
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_END     0x4
// DHC envelope, new
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_START  0xB
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_END    0xC
// Onsen processed data, new
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP     0xD
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE     0x9
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI     0xF
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG     0xE
// Free IDs are 0x7 0x8 0xA

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

using namespace boost::spirit::endian;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDUnpackerDHH)

/// If you change this list, change the NAMEs in the terminate function, too
//#define ONSEN_ERR_FLAG_FTSW_DHC_MM 0x00000001ul// unused
#define ONSEN_ERR_FLAG_DHC_DHE_MM  0x00000002ul
#define ONSEN_ERR_FLAG_DHC_META_MM  0x00000004ul
#define ONSEN_ERR_FLAG_ONSEN_TRG_FIRST 0x00000008ul
#define ONSEN_ERR_FLAG_DHC_END   0x00000010ul
#define ONSEN_ERR_FLAG_DHE_START  0x00000020ul
#define ONSEN_ERR_FLAG_DHC_FRAMECOUNT 0x00000040ul
#define ONSEN_ERR_FLAG_DATA_OUTSIDE 0x00000080ul
#define ONSEN_ERR_FLAG_DHC_START_SECOND  0x00000100ul
//#define ONSEN_ERR_FLAG_DHC_END2  0x00000200ul// unused
#define ONSEN_ERR_FLAG_FIX_SIZE   0x00000400ul
#define ONSEN_ERR_FLAG_DHE_CRC    0x00000800ul
#define ONSEN_ERR_FLAG_DHC_UNKNOWN   0x00001000ul
#define ONSEN_ERR_FLAG_MERGER_CRC   0x00002000ul
#define ONSEN_ERR_FLAG_PACKET_SIZE  0x00004000ul
#define ONSEN_ERR_FLAG_MAGIC    0x00008000ul
#define ONSEN_ERR_FLAG_FRAME_NR   0x00010000ul
#define ONSEN_ERR_FLAG_FRAME_SIZE 0x00020000ul
#define ONSEN_ERR_FLAG_HLTROI_MAGIC 0x00040000ul
#define ONSEN_ERR_FLAG_MERGER_TRIGNR  0x00080000ul
#define ONSEN_ERR_FLAG_DHP_SIZE   0x00100000ul
#define ONSEN_ERR_FLAG_DHE_DHP_DHEID  0x00200000ul
#define ONSEN_ERR_FLAG_DHE_DHP_PORT 0x00400000ul
#define ONSEN_ERR_FLAG_DHP_PIX_WO_ROW 0x00800000ul
#define ONSEN_ERR_FLAG_DHE_START_END_ID 0x01000000ul
#define ONSEN_ERR_FLAG_DHE_START_ID 0x02000000ul
#define ONSEN_ERR_FLAG_DHE_START_WO_END 0x04000000ul
#define ONSEN_ERR_FLAG_NO_PXD   0x08000000ul
#define ONSEN_ERR_FLAG_NO_DATCON   0x10000000ul
#define ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG   0x20000000ul
#define ONSEN_ERR_FLAG_DHE_ACTIVE   0x40000000ul
#define ONSEN_ERR_FLAG_DHP_ACTIVE   0x80000000ul

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

/// define our CRC function
using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhe_crc_32_type;

///*********************************************************************************
///****************** DHC Data Frame Code starts here *****************************
///*********************************************************************************

struct dhc_frame_header_word0 {
  const ubig16_t data; // ENDIAN
  /// fixed length
  inline ubig16_t getData(void) const // ENDIAN
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
    B2INFO("DHC FRAME TYP " << hex << getFrameType() << " -> " << dhc_type_name[getFrameType()] << " (ERR " << getErrorFlag() <<
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

  unsigned short getRunSubrun(void) const
  {
    return run_subrun;
  };

  unsigned short getExp(void) const
  {
    return exp_run;
  };

  unsigned short getEventNrLo(void) const
  {
    return trigger_nr_lo;
  };

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
    B2INFO("DHC Start Frame TNRLO $" << hex << trigger_nr_lo << " TNRHI $" << hex << trigger_nr_hi << " TTLO $" << hex <<
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

  inline unsigned short getEventNrLo(void) const
  {
    return trigger_nr_lo;
  };
  inline unsigned short getStartFrameNr(void) const  // last DHP frame before trigger
  {
    return (sfnr_offset & 0xFC00) >> 10;
  };
  inline unsigned short getTriggerOffsetRow(void) const  // and trigger row offset
  {
    return sfnr_offset & 0x03FF;
  };
  inline unsigned int getFixedSize(void) const
  {
    return 16;// 8 words
  };
  void print(void) const
  {
    word0.print();
    B2INFO("DHC Event Frame TNRLO $" << hex << trigger_nr_lo  << " DTTLO $" << hex << dhe_time_tag_lo << " DTTHI $" << hex <<
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

  inline unsigned short getEventNrLo(void) const
  {
    return trigger_nr_lo;
  };
  inline unsigned int getFixedSize(void) const
  {
    return (4 + 96) * 2;// 100 words
  };
  inline unsigned int getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
};

struct dhc_direct_readout_frame {
  const dhc_frame_header_word0 word0;
  const ubig16_t trigger_nr_lo;
  /// an unbelievable amount of words may follow
  /// and finally a 32 bit checksum

  inline unsigned short getEventNrLo(void) const
  {
    return trigger_nr_lo;
  };
  void print(void) const
  {
    word0.print();
    B2INFO("DHC Direct Readout (Raw|ZSD|ONS) Frame TNRLO $" << hex << trigger_nr_lo << " DHE ID $" << getDHEId() << " DHP port $" <<
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
  //! This should not be show up in BonnDAQ DHH Dumps
  const dhc_frame_header_word0 word0;
  const ubig16_t trignr0;
  const ubig32_t magic1;//! CAFExxxx , redundant
  const ubig32_t trignr1;//! HLT Trigger/Tag part 1
  const ubig32_t trigtag1;//! HLT Trigger/Tag part 2
  const ubig32_t magic2;/// CAFExxxx, redundant
  const ubig32_t trignr2;/// redundant, DATCON Trigger/Tag part 1
  const ubig32_t trigtag2;/// redundant, DATCON Trigger/Tag part 2
  const unsigned int crc32;

  inline unsigned int getFixedSize(void) const
  {
    return 32;//  8*4 bytes might still be changed
  };
  inline unsigned short get_trig_nr0(void) const
  {
    return trignr0;
  };
  inline unsigned int get_trig_nr1(void) const
  {
    return trignr1;
  };
  inline unsigned int get_trig_nr2(void) const
  {
    return trignr2;
  };
  void print(void) const
  {
    word0.print();
    B2INFO("ONSEN Trigger Frame TNRLO $" << hex << trignr0);
  };
  unsigned int check_error(bool ignore_datcon_flag = false) const
  {
    unsigned int m_errorMask = 0;
    if ((magic1 & 0xFFFF0000) != 0xCAFE0000) {
      B2ERROR("ONSEN Trigger Magic 1 error $" << hex << magic1);
      m_errorMask |= ONSEN_ERR_FLAG_HLTROI_MAGIC;
    }
    if ((magic2 & 0xFFFF0000) != 0xCAFE0000) {
      B2ERROR("ONSEN Trigger Magic 2 error $" << hex << magic2);
      m_errorMask |= ONSEN_ERR_FLAG_HLTROI_MAGIC;
    }
    if (magic2 == 0xCAFE0000 && trignr2 == 0x00000000) {
      if (!ignore_datcon_flag) B2WARNING("ONSEN Trigger Frame: No DATCON data " << hex << trignr1 << "!=$" << trignr2);
      m_errorMask |= ONSEN_ERR_FLAG_NO_DATCON;
    } else {
      if (trignr1 != trignr2) {
        B2ERROR("ONSEN Trigger Frame Trigger Nr Mismatch $" << hex << trignr1 << "!=$" << trignr2);
        m_errorMask |= ONSEN_ERR_FLAG_MERGER_TRIGNR;
      }
    }
    return m_errorMask;
  };
};

struct dhc_onsen_roi_frame {
  const dhc_frame_header_word0 word0;/// mainly empty
  const ubig16_t trignr0;// not used
  /// plus n* ROIs (64 bit)
  /// plus checksum 32bit

  inline unsigned short get_trig_nr0(void) const
  {
    return trignr0;
  };
  unsigned int check_error(void) const
  {
    unsigned int m_errorMask = 0;
    // there is nothing to check here...
    return m_errorMask;
  };
  void print(void) const
  {
    word0.print();
    B2INFO("DHC HLT/ROI Frame");
  };

  unsigned int check_inner_crc(unsigned int /*length*/) const
  {
    /// Parts of the data are now in the ONSEN Trigger frame, therefore the inner CRC cannot be checked that easily!
    // TODO can be re-implemented if needed
    return 0;
  };
  void save(StoreArray<PXDRawROIs>& sa, unsigned int length, unsigned int* data) const
  {
    // not clear what will remain here, as part of data (headers) will go to trigger frame.
    if (length < 4 + 4) {
      B2ERROR("DHC ONSEN HLT/ROI Frame too small to hold any ROIs, did not save anything!");
      return;
    }
    unsigned int l;
    l = (length - 4 - 4) / 8;
    // Endian swapping is done in Contructor of RawRoi object
    sa.appendNew(l, &data[1]);
  }

};

struct dhc_ghost_frame {
  const dhc_frame_header_word0 word0;
  const ubig16_t trigger_nr_lo;
  const unsigned int crc32;
  /// fixed length

  inline unsigned int getFixedSize(void) const
  {
    return 8;
  };
  void print(void) const
  {
    word0.print();
    B2INFO("DHC Ghost Frame TNRLO " << hex << trigger_nr_lo << " DHE ID $" << getDHEId() << " DHP port $" << getDHPPort() << " CRC $");
  };
  inline unsigned short getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
  inline unsigned short getDHPPort(void) const {return (word0.getMisc()) & 0x3;};
};

struct dhc_end_frame {
  const dhc_frame_header_word0 word0;
  const ubig16_t trigger_nr_lo;
  const unsigned int wordsinevent;
  const unsigned int errorinfo;
  const unsigned int crc32;
  /// fixed length

  unsigned int get_words(void) const
  {
    return wordsinevent;
  }
  inline unsigned int getFixedSize(void) const
  {
    return 16;
  };
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
    B2INFO("DHC End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
           << " CRC " << hex << crc32);
  };
  inline unsigned int get_dhc_id(void) const {return (word0.getMisc() >> 5) & 0xF;};
};

struct dhc_dhe_end_frame {
  const dhc_frame_header_word0 word0;
  const ubig16_t trigger_nr_lo;
  const unsigned int wordsinevent;
  const unsigned int errorinfo;
  const unsigned int crc32;
  /// fixed length

  unsigned int get_words(void) const
  {
    return wordsinevent;
  }
  inline unsigned int getFixedSize(void) const
  {
    return 16;
  };
  void print(void) const
  {
    word0.print();
    B2INFO("DHC DHE End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
           << " CRC " << hex << crc32);
  };
  inline unsigned int getDHEId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
};


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
  unsigned int getEventNrLo(void)
  {
    return ((ubig16_t*)data)[1];
  };
  unsigned int check_crc(void)
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
      return ONSEN_ERR_FLAG_DHE_CRC;
    }
    return 0;
  };


  unsigned int getFixedSize(void)
  {
    unsigned int s = 0;
    switch (getFrameType()) {
      case DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW:
        s = 0; /// size is not a fixed number
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP:
      case DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD:
        s = 0; /// size is not a fixed number
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE:
      case DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW:
        s = 0; /// size is not a fixed number
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_COMMODE:
        s = data_commode_frame->getFixedSize();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_GHOST:
        s = data_ghost_frame->getFixedSize();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_DHE_START:
        s = data_dhe_start_frame->getFixedSize();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_DHE_END:
        s = data_dhe_end_frame->getFixedSize();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_DHC_START:
        s = data_dhc_start_frame->getFixedSize();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_DHC_END:
        s = data_dhc_end_frame->getFixedSize();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI:
        s = 0; /// size is not a fixed number
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG:
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

PXDUnpackerDHHModule::PXDUnpackerDHHModule() :
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

  addParam("RawDHHsName", m_RawDHHsName, "The name of the StoreArray of RawDHHs to be processed", std::string(""));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "The name of the StoreArray of generated PXDRawHits", std::string(""));
  addParam("PXDRawAdcsName", m_PXDRawAdcsName, "The name of the StoreArray of generated PXDRawAdcs", std::string(""));
  addParam("PXDRawPedestalsName", m_PXDRawPedestalsName, "The name of the StoreArray of generated PXDRawPedestals", std::string(""));
  addParam("PXDRawROIsName", m_PXDRawROIsName, "The name of the StoreArray of generated PXDRawROIs", std::string(""));
  addParam("HeaderEndianSwap", m_headerEndianSwap, "Swap the endianess of the ONSEN header", true);
  addParam("IgnoreDATCON", m_ignoreDATCON, "Ignore missing DATCON ROIs", false);
  addParam("DoNotStore", m_doNotStore, "only unpack and check, but do not store", false);
  addParam("ClusterName", m_RawClusterName, "The name of the StoreArray of PXD Clusters to be processed", std::string(""));
}

void PXDUnpackerDHHModule::initialize()
{
  StoreArray<RawDHH>::required(m_RawDHHsName);
  //Register output collections
  m_storeRawHits.registerInDataStore(m_PXDRawHitsName);
  m_storeRawAdc.registerInDataStore(m_PXDRawAdcsName);
  m_storeRawPedestal.registerInDataStore(m_PXDRawPedestalsName);
  m_storeROIs.registerInDataStore(m_PXDRawROIsName);

  ////CLUSTER CLASS
  m_storeRawCluster.registerInDataStore(m_RawClusterName);
  /// actually, later we do not want to store ROIs and Pedestals into output file ...  aside from debugging

  B2INFO("HeaderEndianSwap: " << m_headerEndianSwap);
  B2INFO("Ignore(missing)DATCON: " << m_ignoreDATCON);

  ignore_datcon_flag = m_ignoreDATCON;

  m_unpackedEventsCount = 0;
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) m_errorCounter[i] = 0;
}

void PXDUnpackerDHHModule::terminate()
{
  const string error_name[ONSEN_MAX_TYPE_ERR] = {
    "-unused-", "DHC/DHE mismatch", "EvtMeta/DHC mismatch", "ONSEN Trigger is not first frame",
    "DHC_END missing", "DHE_START missing", "DHC Framecount mismatch", "DATA outside of DHE",
    "DHC_START is not second frame", "-unused-", "Fixed size frame wrong size", "DHE CRC Error:",
    "Unknown DHC type", "Merger CRC Error", "Event Header Full Packet Size Error", "Event Header Magic Error",
    "Event Header Frame Count Error", "Event header Frame Size Error", "HLTROI Magic Error", "Merger HLT/DATCON TrigNr Mismatch",
    "DHP Size too small", "DHP-DHE DHEID mismatch", "DHP-DHE Port mismatch", "DHP Pix w/o row",
    "DHE START/END ID mismatch", "DHE ID mismatch of START and this frame", "DHE_START w/o prev END", "Nr PXD data ==0",
    "Missing Datcon", "NO DHC data for Trigger", "DHE active mismatch", "DHP active mismatch"
  };

  int flag = 0;
  string errstr = "Statistic ( ;";
  errstr += to_string(m_unpackedEventsCount) + ";";
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) { errstr += to_string(m_errorCounter[i]) + ";"; flag |= m_errorCounter[i];}
  if (flag != 0) {
    B2ERROR("PXD DHH Unpacker --> Error Statistics (counted once per event!) in Events: " << m_unpackedEventsCount);
    B2ERROR(errstr + " )");
    for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      if (m_errorCounter[i]) {
        B2ERROR(error_name[i] << ": " << m_errorCounter[i]);
        //printf("%s %d\n",error_name[i].c_str(), m_errorCounter[i]);
      }
    }
  } else {
    B2INFO("PXD DHH Unpacker --> No Error found in Events: " << m_unpackedEventsCount);
  }
}

void PXDUnpackerDHHModule::event()
{
  StoreArray<RawDHH> storeRaws(m_RawDHHsName);
  StoreObjPtr<EventMetaData> evtPtr;/// what will happen if it does not exist???

  int nRaws = storeRaws.getEntries();
  if (verbose) {
    B2INFO("PXD DHH Unpacker --> RawDHH Objects in event: " << nRaws);
  };

  m_errorMask = 0;

  m_meta_event_nr = evtPtr->getEvent();
  m_meta_run_nr = evtPtr->getRun();
  m_meta_subrun_nr = evtPtr->getSubrun();
  m_meta_experiment = evtPtr->getExperiment();
  /// evtPtr->getTime()


  int nsr = 0;// number of packets
  for (auto& it : storeRaws) {
    if (verbose) {
      B2INFO("PXD DHH Unpacker --> Unpack Objects: ");
    };
    unpack_event(it);
    nsr++;
  }

  if (nsr == 0) m_errorMask |= ONSEN_ERR_FLAG_NO_PXD;

  m_unpackedEventsCount++;
  for (unsigned int i = 0, j = 1; i < ONSEN_MAX_TYPE_ERR; i++) {
    if (m_errorMask & j) m_errorCounter[i]++;
    j += j;
  }
}

void endian_swapper(void* a, unsigned int len)
{
  // Quick and Dirty swapper for BonnDAQ
  ubig16_t* p;
  ulittle16_t* q;
  p = (ubig16_t*)a;
  q = (ulittle16_t*)a;
  len /= 2;
  for (unsigned int i = 0; i < len; i++, q++, p++) { *q = *p;}
}

void PXDUnpackerDHHModule::unpack_event(RawDHH& px)
{
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD DHH Unpacker --> invalid packet size (32bit words) " << hex << px.size());
    m_errorMask |= ONSEN_ERR_FLAG_PACKET_SIZE;
    return;
  }
  std::vector<unsigned int> data(px.size());
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  std::copy_n(px.data(), px.size(), data.begin());

  if (fullsize < 16) {
    B2ERROR("Data is to small to hold a valid Header! Will not unpack anything. Size:" << fullsize);
    m_errorMask |= ONSEN_ERR_FLAG_PACKET_SIZE;
    return;
  }

  unsigned int* dataptr;
  dataptr = &data[5];
  datafullsize = fullsize - 5 * 4; // minus header

  int ll = 0; // Offset in dataptr in bytes
  int j = 0;
  while (datafullsize - ll >= 16) {
    if (dataptr[0] != 0xCAFEBABE) {
      B2ERROR("Magic invalid in frame " << j << " (" << datafullsize << "," << ll << "): Will not unpack anything. Header corrupted! $" <<
              hex << dataptr[0] << " $" << hex << dataptr[1] << " $" << hex << dataptr[2] << " $" << hex << dataptr[3]);
      m_errorMask |= ONSEN_ERR_FLAG_MAGIC;
      break;
    }

    int lo;// len of frame in bytes
    lo = dataptr[1];
    dataptr += 4; // Cafebabe header
    ll += 16;

    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << " size $" << std::hex << lo << " at byte offset " << std::hex << ll);
      m_errorMask |= ONSEN_ERR_FLAG_FRAME_SIZE;
      return;
    }

    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset " << ll << " of datafullsize " <<
              datafullsize << " of fullsize " << fullsize);
      m_errorMask |= ONSEN_ERR_FLAG_FRAME_SIZE;
      return;
    }
    if (lo & 0x3) {
      B2ERROR("SKIP Frame with Data with not MOD 4 length " << " ( $" << hex << lo << " ) ");
      lo = (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
    } else {
      B2INFO("unpack DHE(C) frame: " << j << " with size $" << std::hex << lo << " at byte offset " << std::hex << ll);
      endian_swapper((char*)dataptr, lo);
      unpack_dhc_frame((char*)dataptr, lo, j, 256);
    }
    ll += lo;
    dataptr += lo / 4;
    j++;// framecounter
  }

}

void PXDUnpackerDHHModule::unpack_dhp_raw(void* data, unsigned int frame_len, unsigned int dhe_ID, unsigned dhe_DHPport,
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
    m_errorMask |= ONSEN_ERR_FLAG_DHE_DHP_DHEID;
  }
  if (dhe_DHPport != dhp_dhp_id) {
    B2ERROR("DHP ID (Chip/Port) in DHE and DHP header differ $" << hex << dhe_DHPport << " != $" << dhp_dhp_id);
    m_errorMask |= ONSEN_ERR_FLAG_DHE_DHP_PORT;
  }

  if (dhp_header_type != DHP_FRAME_HEADER_DATA_TYPE_RAW) {
    B2ERROR("Header type invalid for this kind of DHE frame: $" << hex << dhp_header_type);
    return;
  }

  /// Endian Swapping is done in Contructors of Raw Objects!
  if (frame_len == 0x10008) { // 64k
    B2INFO("Pedestal Data - (ADC:ADC)");
    m_storeRawAdc.appendNew(vxd_id, data, false);
  } else if (frame_len == 0x20008) { // 128k
    B2INFO("Pedestal Data - (ADC:Pedestal)");
    m_storeRawAdc.appendNew(vxd_id, data, true);
    m_storeRawPedestal.appendNew(vxd_id, data);
  } else {
    // checked already above
  }
};

void PXDUnpackerDHHModule::unpack_fce(unsigned short* data, unsigned int length, VxdID vxd_id)
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

void PXDUnpackerDHHModule::unpack_dhp(void* data, unsigned int frame_len, unsigned int dhe_first_readout_frame_id_lo,
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
  unsigned int dhp_offset = 0;
  bool rowflag = false;

  if (nr_words < 4) {
    B2ERROR("DHP frame size error (too small) " << nr_words);
    m_errorMask |= ONSEN_ERR_FLAG_DHP_SIZE;
//     dhp_size_error++;
    return;
    //return -1;
  }

  if (printflag)
    B2INFO("HEADER --  " << hex << dhp_pix[0] << hex << dhp_pix[1] << hex << dhp_pix[2] << hex << dhp_pix[3] << " -- ");

  if (printflag)
    B2INFO("DHP Header   | " << hex << dhp_pix[2] << " ( " << hex << dhp_pix[2] << " ) ");
  dhp_header_type  = (dhp_pix[2] & 0xE000) >> 13;
  dhp_reserved     = (dhp_pix[2] & 0x1F00) >> 8;
  dhp_dhe_id       = (dhp_pix[2] & 0x00FC) >> 2;
  dhp_dhp_id       =  dhp_pix[2] & 0x0003;

  if (printflag) {
    B2INFO("DHP type     | " << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
    B2INFO("DHP reserved | " << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
    B2INFO("DHP DHE ID   | " << hex << dhp_dhe_id << " ( " << dec << dhp_dhe_id << " ) ");
    B2INFO("DHP DHP ID   | " << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  }

  if (dhe_ID != dhp_dhe_id) {
    B2ERROR("DHE ID in DHE and DHP header differ $" << hex << dhe_ID << " != $" << dhp_dhe_id);
    m_errorMask |= ONSEN_ERR_FLAG_DHE_DHP_DHEID;
  }
  if (dhe_DHPport != dhp_dhp_id) {
    B2ERROR("DHP ID (Chip/Port) in DHE and DHP header differ $" << hex << dhe_DHPport << " != $" << dhp_dhp_id);
    m_errorMask |= ONSEN_ERR_FLAG_DHE_DHP_PORT;
  }

  if (dhp_header_type != DHP_FRAME_HEADER_DATA_TYPE_ZSD) {
    B2ERROR("Header type invalid for this kind of DHE frame: $" << hex << dhp_header_type);
    return;
  }

  static int offtab[4] = {0, 64, 128, 192};
  dhp_offset = offtab[dhp_dhp_id];

  dhp_readout_frame_lo  = dhp_pix[3] & 0xFFFF;
  if (printflag)
    B2INFO("DHP Frame Nr     |   " << hex << dhp_readout_frame_lo << " ( " << hex << dhp_readout_frame_lo << " ) ");

  for (unsigned int i = 4; i < nr_words ; i++) {

    if (printflag)
      B2INFO("-- " << hex << dhp_pix[i] << " --   " << dec << i);
    {
      if (((dhp_pix[i] & 0x8000) >> 15) == 0) {
        rowflag = true;
        dhp_row = (dhp_pix[i] & 0xFFC0) >> 5;
        dhp_cm  = dhp_pix[i] & 0x3F;
        if (printflag)
          B2INFO("SetRow: " << hex << dhp_row << " CM " << hex << dhp_cm);
      } else {
        if (!rowflag) {
          B2ERROR("DHP Unpacking: Pix without Row!!! skip dhp data ");
          m_errorMask |= ONSEN_ERR_FLAG_DHP_PIX_WO_ROW;
//           dhp_pixel_error++;
          return;
        } else {
          dhp_row = (dhp_row & 0xFFE) | ((dhp_pix[i] & 0x4000) >> 14);
          dhp_col = ((dhp_pix[i]  & 0x3F00) >> 8);
          ///  remapping flag
          if (dhe_reformat == 0) dhp_col ^= 0x3C ; /// 0->60 61 62 63 4->56 57 58 59 ...
          dhp_col += dhp_offset;
          dhp_adc = dhp_pix[i] & 0xFF;
          if (printflag)
            B2INFO("SetPix: Row " << hex << dhp_row << " Col " << hex << dhp_col << " ADC " << hex << dhp_adc
                   << " CM " << hex << dhp_cm);

          /*if (verbose) {
            B2INFO("raw    |   " << hex << d[i]);
            B2INFO("row " << hex << ((d[i] >> 20) & 0xFFF) << "(" << ((d[i] >> 20) & 0xFFF) << ")" << " col " << "(" << hex << ((d[i] >> 8) & 0xFFF) << ((d[i] >> 8) & 0xFFF)
                   << " adc " << "(" << hex << (d[i] & 0xFF) << (d[i] & 0xFF) << ")");
            B2INFO("dhe_ID " << dhe_ID);
            B2INFO("start-Frame-Nr " << dec << dhe_first_readout_frame_id_lo);
            B2INFO("toffset " << toffset);
          };*/

          if (!m_doNotStore) m_storeRawHits.appendNew(vxd_id, dhp_row, dhp_col, dhp_adc,
                                                        toffset, (dhp_readout_frame_lo - dhe_first_readout_frame_id_lo) & 0x3F, dhp_cm
                                                       );
        }
      }
    }
  }

  if (printflag) {
    B2INFO("(DHE) DHE_ID " << hex << dhe_ID << " (DHE) DHP ID  " << hex << dhe_DHPport << " (DHP) DHE_ID " << hex << dhp_dhe_id <<
           " (DHP) DHP ID " << hex << dhp_dhp_id);
    /*for (int i = 0; i < raw_nr_words ; i++) {
      B2INFO("RAW      |   " << hex << p_pix[i]);
      printf("raw %08X  |  ", p_pix[i]);
      B2INFO("row " << hex << ((p_pix[i] >> 20) & 0xFFF) << dec << " ( " << ((p_pix[i] >> 20) & 0xFFF) << " ) " << " col " << hex << ((p_pix[i] >> 8) & 0xFFF)
             << " ( " << dec << ((p_pix[i] >> 8) & 0xFFF) << " ) " << " adc " << hex << (p_pix[i] & 0xFF) << " ( " << (p_pix[i] & 0xFF) << " ) "
            );
    }*/
  }
};

int PXDUnpackerDHHModule::nr5bits(int i) const
{
  /// too lazy to count the bits myself, thus using a small lookup table
  const int lut[32] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5
  };
  return lut[i & 0x1F];
}

void PXDUnpackerDHHModule::unpack_dhc_frame(void* data, const int len, const int Frame_Number, const int Frames_in_event)
{
  /// The following STATIC variables are used to save some state or count some things
  /// while depacking the frames. they are in most cases (re)set on the first frame or ONSEN trg frame
  /// Most could put in as a class member, but they are only needed within this function
  static unsigned int eventNrOfOnsenTrgFrame = 0;
  static int countedWordsInEvent;// count the size of all frames in words
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
  static unsigned int currentDHEID = 0xFFFFFFFF;
  static unsigned int currentVxdId = 0;
  static bool isFakedData_event = false;



  dhc_frame_header_word0* hw = (dhc_frame_header_word0*)data;
//   error_flag = false;

  dhc_frames dhc;
  dhc.set(data, hw->getFrameType(), len);
  int s;
  s = dhc.getFixedSize();
  if (len != s && s != 0) {
    B2ERROR("Fixed frame type size does not match specs: expect " << len << " != " << s << " (in data) ");
    m_errorMask |= ONSEN_ERR_FLAG_FIX_SIZE;
  }

  unsigned int eventNrOfThisFrame = dhc.getEventNrLo();
  int type = dhc.getFrameType();

  if (Frame_Number == 0) { /// We reset the counters on the first event
    countedDHEStartFrames = 0;
    countedDHEEndFrames = 0;
    countedWordsInEvent = 0;
    currentDHEID = 0xFFFFFFFF;
    currentVxdId = 0;

    isFakedData_event = false;
    if (type == DHC_FRAME_HEADER_DATA_TYPE_DHC_START) {
      B2ERROR("This looks like this is the old Desy 2013/14 testbeam format. Please use the pxdUnpackerDesy1314 module.");
    }
  }

  if (Frame_Number == 1) {
    if (type == DHC_FRAME_HEADER_DATA_TYPE_DHC_START) {
      isFakedData_event = dhc.data_dhc_start_frame->isFakedData();
    }
  }

  // please check if this mask is suitable. At least here we are limited by the 16 bit trigger number in the DHH packet header.
  // we can use more bits in the START Frame
  if ((eventNrOfThisFrame & 0xFFFF) != (m_meta_event_nr & 0xFFFF)) {
    B2ERROR("Event Numbers do not match for this frame $" << hex << eventNrOfThisFrame << "!=$" << m_meta_event_nr <<
            "(MetaInfo) mask");
    m_errorMask |= ONSEN_ERR_FLAG_DHC_META_MM;
  }

  if (Frame_Number > 1 && Frame_Number < Frames_in_event - 1) {
    if (countedDHEStartFrames != countedDHEEndFrames + 1)
      if (type != DHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI && type != DHC_FRAME_HEADER_DATA_TYPE_DHE_START) {
        B2ERROR("Data Frame outside a DHE START/END");
        m_errorMask |= ONSEN_ERR_FLAG_DATA_OUTSIDE;
      }
  }

  switch (type) {
    case DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW: {
      countedDHCFrames++;

      if (verbose) dhc.data_direct_readout_frame_raw->print();
      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_direct_readout_frame_raw->getDHEId());
        m_errorMask |= ONSEN_ERR_FLAG_DHE_START_ID;
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
    case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP:
    case DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD: {
      countedDHCFrames++;

      if (verbose)dhc.data_direct_readout_frame->print();

      //m_errorMask |= dhc.data_direct_readout_frame->check_error();

      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_direct_readout_frame_raw->getDHEId());
        m_errorMask |= ONSEN_ERR_FLAG_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();
//       stat_zsd++;


      unpack_dhp(data, len - 4,
                 dhe_first_readout_frame_id_lo,
                 dhc.data_direct_readout_frame->getDHEId(),
                 dhc.data_direct_readout_frame->getDHPPort(),
                 dhc.data_direct_readout_frame->getDataReformattedFlag(),
                 dhe_first_offset, currentVxdId);
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE:
    case DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW: {

      countedDHCFrames++;
      if (verbose) hw->print();
      if (currentDHEID != dhc.data_direct_readout_frame_raw->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_direct_readout_frame_raw->getDHEId());
        m_errorMask |= ONSEN_ERR_FLAG_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      found_mask_active_dhp |= 1 << dhc.data_direct_readout_frame->getDHPPort();

      B2INFO("UNPACK FCE FRAME with len " << hex << len)
      unpack_fce((unsigned short*) data, len - 4, currentVxdId);

      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_COMMODE: {
      countedDHCFrames++;

      if (verbose) hw->print();
      if (currentDHEID != dhc.data_commode_frame->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_commode_frame->getDHEId());
        m_errorMask |= ONSEN_ERR_FLAG_DHE_START_ID;
      }
      m_errorMask |= dhc.check_crc();
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_DHC_START: {
      if (isFakedData_event != dhc.data_dhc_start_frame->isFakedData()) {
        B2ERROR("DHC START is but no Fake event OR Fake Event but DHE END is not.");
      }
      if (dhc.data_dhc_start_frame->isFakedData()) {
        B2WARNING("Faked DHC START Data -> trigger without Data!");
        m_errorMask |= ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhc.data_dhc_start_frame->print();
      }

//      eventNrOfOnsenTrgFrame = eventNrOfThisFrame;
      currentDHEID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      //nr_of_frames_dhc = dhc.data_dhc_start_frame->get_dhc_nr_frames();
      countedDHCFrames = 1;
      m_errorMask |= dhc.check_crc();
//       stat_start++;

      /// TODO here we should check full(!) Event Number, Run Number, Subrun Nr and Exp Number
      /// of this frame against the one from MEta Event Info
      ///   m_meta_event_nr=evtPtr->getEvent();// filled already above
      ///   m_meta_run_nr=evtPtr->getRun();// filled already above
      ///   n_meta_subrun_nr=evtPtr->getSubrun();// filled already above
      ///   m_meta_experiment=evtPtr->getExperiment();// filled already above

      mask_active_dhe = dhc.data_dhc_start_frame->get_active_dhe_mask();
      nr_active_dhe = nr5bits(mask_active_dhe);
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_DHE_START: {
      countedDHCFrames++;
      if (verbose)dhc.data_dhe_start_frame->print();
      dhe_first_readout_frame_id_lo = dhc.data_dhe_start_frame->getStartFrameNr();
      dhe_first_offset = dhc.data_dhe_start_frame->getTriggerOffsetRow();
      currentDHEID = dhc.data_dhe_start_frame->getDHEId();
      m_errorMask |= dhc.check_crc();

      if (countedDHEStartFrames != countedDHEEndFrames) {
        B2ERROR("DHC_FRAME_HEADER_DATA_TYPE_DHE_START without DHC_FRAME_HEADER_DATA_TYPE_DHE_END");
        m_errorMask |= ONSEN_ERR_FLAG_DHE_START_WO_END;
      }
      countedDHEStartFrames++;

      found_mask_active_dhp = 0;
      mask_active_dhp = dhc.data_dhe_start_frame->getActiveDHPMask();
      ///      nr_active_dhp = nr5bits(mask_active_dhp);// unused

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
        ladder = ((currentDHEID & 0x1E) >> 1) + 1 ;
        layer = ((currentDHEID & 0x20) >> 5) + 1;
        currentVxdId = VxdID(layer, ladder, sensor);

      }

      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_GHOST:
      countedDHCFrames++;
      if (verbose)dhc.data_ghost_frame->print();
      if (currentDHEID != dhc.data_ghost_frame->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_ghost_frame->getDHEId());
        m_errorMask |= ONSEN_ERR_FLAG_DHE_START_ID;
      }
      /// Attention: Firmware might be changed such, that ghostframe come for all DHPs, not only active ones...
      found_mask_active_dhp |= 1 << dhc.data_ghost_frame->getDHPPort();
      m_errorMask |= dhc.check_crc();
//       stat_ghost++;

      break;
    case DHC_FRAME_HEADER_DATA_TYPE_DHC_END: {
      if (dhc.data_dhc_end_frame->isFakedData() != isFakedData_event) {
        B2ERROR("DHC END is but no Fake event OR Fake Event but DHE END is not.");
      }
      countedDHCFrames++;
      currentDHEID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      if (isFakedData_event) {
        B2WARNING("Faked DHC END Data -> trigger without Data!");
        m_errorMask |= ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhc.data_dhc_end_frame->print();
      }
//       stat_end++;

      if (!isFakedData_event) {
//         if (countedDHCFrames != nr_of_frames_dhc) { /// Nr Frames has been removed .. tbd
//           if (!m_ignore_headernrframes) B2ERROR("Number of DHC Frames in Header(??? Tail?) " << nr_of_frames_dhc << " != " << countedDHCFrames << " Counted");
//           m_errorMask |= ONSEN_ERR_FLAG_DHC_FRAMECOUNT;
//         }
      }
      if (!isFakedData_event) {
        int w;
        w = dhc.data_dhc_end_frame->get_words() * 2;
        countedWordsInEvent += 2;
        if (verbose) {
          B2INFO("countedWordsInEvent " << countedWordsInEvent << " w " << w);
        };
        if (countedWordsInEvent != w) {
          if (verbose) {
            B2INFO("Error: WIE " << hex << countedWordsInEvent << " vs END " << hex << w);
          };
//           error_flag = true;
//           wie_error++;
        } else {
          if (verbose)
            B2INFO("EVT END: WIE " << hex << countedWordsInEvent << " == END " << hex << w);
        }
      }
      m_errorMask |= dhc.check_crc();
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_DHE_END: {
      countedDHCFrames++;
      if (verbose) dhc.data_dhe_end_frame->print();
      if (currentDHEID != dhc.data_dhe_end_frame->getDHEId()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << currentDHEID << " != $" <<
                dhc.data_dhe_end_frame->getDHEId());
        m_errorMask |= ONSEN_ERR_FLAG_DHE_START_END_ID;
      }
      currentDHEID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      m_errorMask |= dhc.check_crc();
      if (found_mask_active_dhp != mask_active_dhp) {
        B2ERROR("DHE_END: DHP active mask $" << hex << mask_active_dhp << " != $" << hex << found_mask_active_dhp <<
                " mask of found dhp/ghost frames");
        m_errorMask |= ONSEN_ERR_FLAG_DHP_ACTIVE;
      }
      countedDHEEndFrames++;
      if (countedDHEStartFrames != countedDHEEndFrames) {
        B2ERROR("DHC_FRAME_HEADER_DATA_TYPE_DHE_END without DHC_FRAME_HEADER_DATA_TYPE_DHE_START");
        m_errorMask |= ONSEN_ERR_FLAG_DHE_START;
      }
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI:
      countedDHCFrames += 0; /// DO NOT COUNT!!!!
      if (verbose) dhc.data_onsen_roi_frame->print();
      m_errorMask |= dhc.data_onsen_roi_frame->check_error();// dummy
      m_errorMask |= dhc.data_onsen_roi_frame->check_inner_crc(len - 4); /// CRC is without the DHC header, dummy see reason in function
      m_errorMask |= dhc.check_crc();
      if (!m_doNotStore) dhc.data_onsen_roi_frame->save(m_storeROIs, len, (unsigned int*) data);
      break;
    case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG:
      /// we do not expect to see this frame in BonnDAQ DHH data
      countedDHCFrames += 0; /// DO NOT COUNT!!!!
      eventNrOfOnsenTrgFrame = eventNrOfThisFrame;
//   m_meta_event_nr=evtPtr->getEvent();
//   m_meta_run_nr=evtPtr->getRun();
//   n_meta_subrun_nr=evtPtr->getSubrun();
//   m_meta_experiment=evtPtr->getExperiment();
      if (verbose) dhc.data_onsen_trigger_frame->print();
      m_errorMask |= dhc.data_onsen_trigger_frame->check_error(ignore_datcon_flag);
      m_errorMask |= dhc.check_crc();
      if (Frame_Number != 0) {
        B2ERROR("ONSEN TRG Frame must be the first one.");
      }
      break;
    default:
      B2ERROR("UNKNOWN DHC frame type");
      m_errorMask |= ONSEN_ERR_FLAG_DHC_UNKNOWN;
//      type_error++;
      if (verbose) hw->print();
//       error_flag = true;
      break;
  }

//   if (eventNrOfThisFrame != eventNrOfOnsenTrgFrame) {
//     B2ERROR("Frame TrigNr != ONSEN Trig Nr $" << hex << eventNrOfThisFrame << " != $" << eventNrOfOnsenTrgFrame);
//     m_errorMask |= ONSEN_ERR_FLAG_DHC_DHE_MM;
// //     evtnr_error++;
// //     error_flag = true;
//   }

//   if (Frame_Number == 0) {
//     /// Check that DHC Start is first Frame
//     if (type != DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG) {
//       B2ERROR("First frame is not a ONSEN Trigger frame in Event Nr " << eventNrOfThisFrame);
//       m_errorMask |= ONSEN_ERR_FLAG_ONSEN_TRG_FIRST;
//     }
//   } else { // (Frame_Number != 0 &&
//     /// Check that there is no other DHC Start
//     if (type == DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG) {
//       B2ERROR("More than one ONSEN Trigger frame in Event Nr " << eventNrOfThisFrame);
//       m_errorMask |= ONSEN_ERR_FLAG_ONSEN_TRG_FIRST;
//     }
//   }

//   if (Frame_Number == 0) { // was 0 for ONSEN
//     /// Check that DHC Start is first Frame
//     if (type != DHC_FRAME_HEADER_DATA_TYPE_DHC_START) {
//       B2ERROR("Second frame is not a DHC start of subevent frame in Event Nr " << eventNrOfThisFrame);
//       m_errorMask |= ONSEN_ERR_FLAG_DHC_START_SECOND;
//     }
//   } else { // (Frame_Number != 0 &&
//     /// Check that there is no other DHC Start
//     if (type == DHC_FRAME_HEADER_DATA_TYPE_DHC_START) {
//       B2ERROR("More than one DHC start of subevent frame in Event Nr " << eventNrOfThisFrame);
//       m_errorMask |= ONSEN_ERR_FLAG_DHC_START_SECOND;
//     }
//   }
//
//   if (Frame_Number == Frames_in_event - 1) {
//     /// Check that DHC End is last Frame
//     if (type != DHC_FRAME_HEADER_DATA_TYPE_DHC_END) {
//       B2ERROR("Last frame is not a DHC end of subevent frame in Event Nr " << eventNrOfThisFrame);
//       m_errorMask |= ONSEN_ERR_FLAG_DHC_END;
//     }
//
//     /// As we now have processed the whole event, we can do some more consistency checks!
//     if (countedDHEStartFrames != countedDHEEndFrames || countedDHEStartFrames != nr_active_dhe) {
//       B2ERROR("The number of DHE Start/End does not match the number of active DHE in DHC Header! Header: " << nr_active_dhe <<
//               " Start: " << countedDHEStartFrames << " End: " << countedDHEEndFrames << " Mask: $" << hex << mask_active_dhe << " in Event Nr " <<
//               eventNrOfThisFrame);
//       m_errorMask |= ONSEN_ERR_FLAG_DHE_ACTIVE;
//     }
//
//   } else { //  (Frame_Number != Frames_in_event - 1 &&
//     /// Check that there is no other DHC End
//     if (type == DHC_FRAME_HEADER_DATA_TYPE_DHC_END) {
//       B2ERROR("More than one DHC end of subevent frame in frame in Event Nr " << eventNrOfThisFrame);
//       m_errorMask |= ONSEN_ERR_FLAG_DHC_END;
//     }
//   }
//
//   /// Check that (if there is at least one active DHE) the second Frame is DHE Start, actually this is redundant if the other checks work
//   if (Frame_Number == 2 && nr_active_dhe != 0 && type != DHC_FRAME_HEADER_DATA_TYPE_DHE_START) {
//     B2ERROR("Third frame is not a DHE start frame in Event Nr " << eventNrOfThisFrame);
//     m_errorMask |= ONSEN_ERR_FLAG_DHE_START;
//   }

  countedWordsInEvent += len;

}



