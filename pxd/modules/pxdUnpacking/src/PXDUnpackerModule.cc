/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdUnpacking/PXDUnpackerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/spirit/home/support/detail/endian.hpp>

// DHP modes are the same as for DHH envelope
#define DHP_FRAME_HEADER_DATA_TYPE_RAW  0x0
#define DHP_FRAME_HEADER_DATA_TYPE_ZSD  0x5

// DHH like before, but now 4 bits
#define DHHC_FRAME_HEADER_DATA_TYPE_DHP_RAW     0x0
#define DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD     0x5
#define DHHC_FRAME_HEADER_DATA_TYPE_FCE_RAW     0x1
#define DHHC_FRAME_HEADER_DATA_TYPE_COMMODE     0x6
#define DHHC_FRAME_HEADER_DATA_TYPE_GHOST       0x2
#define DHHC_FRAME_HEADER_DATA_TYPE_DHH_START   0x3
#define DHHC_FRAME_HEADER_DATA_TYPE_DHH_END     0x4
// DHHC envelope, new
#define DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START  0xB
#define DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END    0xC
// Onsen processed data, new
#define DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP     0xD
#define DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE     0x9
#define DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI     0xF
#define DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG     0xE
// Free IDs are 0x7 0x8 0xA

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDUnpacker)

/// If you change this list, change the NAMEs in the terminate function, too
#define ONSEN_ERR_FLAG_FTSW_DHHC_MM 0x00000001ul
#define ONSEN_ERR_FLAG_DHHC_DHH_MM  0x00000002ul
//#define ONSEN_ERR_FLAG_DHHC_DHP_MM  0x00000004ul // unsused
#define ONSEN_ERR_FLAG_ONSEN_TRG_FIRST 0x00000008ul
#define ONSEN_ERR_FLAG_DHHC_END   0x00000010ul
#define ONSEN_ERR_FLAG_DHH_START  0x00000020ul
#define ONSEN_ERR_FLAG_DHHC_FRAMECOUNT 0x00000040ul
#define ONSEN_ERR_FLAG_DATA_OUTSIDE 0x00000080ul
#define ONSEN_ERR_FLAG_DHHC_START_SECOND  0x00000100ul
//#define ONSEN_ERR_FLAG_DHHC_END2  0x00000200ul
#define ONSEN_ERR_FLAG_FIX_SIZE   0x00000400ul
#define ONSEN_ERR_FLAG_DHH_CRC    0x00000800ul
#define ONSEN_ERR_FLAG_DHHC_UNKNOWN   0x00001000ul
#define ONSEN_ERR_FLAG_MERGER_CRC   0x00002000ul
#define ONSEN_ERR_FLAG_PACKET_SIZE  0x00004000ul
#define ONSEN_ERR_FLAG_MAGIC    0x00008000ul
#define ONSEN_ERR_FLAG_FRAME_NR   0x00010000ul
#define ONSEN_ERR_FLAG_FRAME_SIZE 0x00020000ul
#define ONSEN_ERR_FLAG_HLTROI_MAGIC 0x00040000ul
#define ONSEN_ERR_FLAG_MERGER_TRIGNR  0x00080000ul
#define ONSEN_ERR_FLAG_DHP_SIZE   0x00100000ul
#define ONSEN_ERR_FLAG_DHH_DHP_DHHID  0x00200000ul
#define ONSEN_ERR_FLAG_DHH_DHP_PORT 0x00400000ul
#define ONSEN_ERR_FLAG_DHP_PIX_WO_ROW 0x00800000ul
#define ONSEN_ERR_FLAG_DHH_START_END_ID 0x01000000ul
#define ONSEN_ERR_FLAG_DHH_START_ID 0x02000000ul
#define ONSEN_ERR_FLAG_DHH_START_WO_END 0x04000000ul
#define ONSEN_ERR_FLAG_NO_PXD   0x08000000ul
#define ONSEN_ERR_FLAG_NO_DATCON   0x10000000ul
#define ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG   0x20000000ul
#define ONSEN_ERR_FLAG_DHH_ACTIVE   0x40000000ul
#define ONSEN_ERR_FLAG_DHP_ACTIVE   0x80000000ul

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

/// define our CRC function
using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhh_crc_32_type;

///*********************************************************************************
///****************** DHHC Data Frame Code starts here *****************************
///*********************************************************************************

struct dhhc_frame_header_word0 {
  const unsigned short data;
  /// fixed length
  inline unsigned int getData(void) const {
    return data;
  };
  inline unsigned int getFrameType(void) const {
    return (data >> 11) & 0xF;
  };
  inline unsigned int getErrorFlag(void) const {
    return (data >> 15) & 0x1;
  };
  inline unsigned int getMisc(void) const {
    return data & 0x3FF;
  };
  void print(void) const {
    const char* dhhc_type_name[16] = {
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
    B2INFO("DHHC FRAME TYP " << hex << getFrameType() << " -> " << dhhc_type_name[getFrameType()] << " (ERR " << getErrorFlag() << ") data " << data);
  };
};

struct dhhc_start_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trigger_nr_lo;
  const unsigned short trigger_nr_hi;
  const unsigned short time_tag_lo_and_type;
  const unsigned short time_tag_mid;
  const unsigned short time_tag_hi;
  const unsigned short run_subrun;
  const unsigned short exp_run;
  const unsigned int crc32;
  /// fixed length, only for reading

  unsigned short getEventNrLo(void) const {
    return trigger_nr_lo;
  };
  bool isFakedData(void) const {
    if (word0.data != 0x5800) return false;
    if (trigger_nr_lo != 0) return false;
    if (trigger_nr_hi != 0) return false;
    if (time_tag_lo_and_type != 0) return false;
    if (time_tag_mid != 0) return false;
    if (time_tag_hi != 0) return false;
    if (run_subrun != 0) return false;
    if (exp_run != 0) return false;
    if (crc32 != 0x12345678) return false; // Recalculate!!! TODO
    return true;
  };
  inline unsigned int getFixedSize(void) const {
    return 20;// bytes
  };
  void print(void) const {
    word0.print();
    B2INFO("DHHC Start Frame TNRLO $" << hex << trigger_nr_lo << " TNRHI $" << hex << trigger_nr_hi << " TTLO $" << hex << time_tag_lo_and_type
           << " TTMID $" << hex << time_tag_mid << " TTHI $" << hex << time_tag_hi << " Exp/Run/Subrun $" << hex << exp_run << " $" << run_subrun
           << " CRC $" << hex << crc32);
  };
  inline unsigned int get_active_dhh_mask(void) const {return word0.getMisc() & 0x1F;};
  inline unsigned int get_dhhc_id(void) const {return (word0.getMisc() >> 5) & 0xF;};
  inline unsigned int get_subrun(void) const {return run_subrun & 0xFF;};
  inline unsigned int get_run(void) const {return ((run_subrun >> 8) & 0xFF) | ((exp_run << 8) & 0x3F00);};
  inline unsigned int get_experiment(void) const {return (exp_run >> 6) & 0x3FF;};
};

struct dhhc_dhh_start_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trigger_nr_lo;
  const unsigned short trigger_nr_hi;
  const unsigned short dhh_time_tag_lo;
  const unsigned short dhh_time_tag_hi;
  const unsigned short sfnr_offset;
  const unsigned int crc32;
  /// fixed length

  inline unsigned short getEventNrLo(void) const {
    return trigger_nr_lo;
  };
  inline unsigned short getStartFrameNr(void) const {// last DHP frame before trigger
    return (sfnr_offset >> 10) & 0x3F;
  };
  inline unsigned short getTriggerOffsetRow(void) const {// and trigger row offset
    return sfnr_offset & 0x3FF;
  };
  inline unsigned int getFixedSize(void) const {
    return 16;// 8 words
  };
  void print(void) const {
    word0.print();
    B2INFO("DHHC Event Frame TNRLO $" << hex << trigger_nr_lo  << " DTTLO $" << hex << dhh_time_tag_lo << " DTTHI $" << hex << dhh_time_tag_hi
           << " DHHID $" << hex << getDHHId()
           << " DHPMASK $" << hex << getActiveDHPMask()
           << " SFNR $" << hex << getStartFrameNr()
           << " OFF $" << hex << getTriggerOffsetRow()
           << " CRC " << hex << crc32);
  };
  inline unsigned int getActiveDHPMask(void) const {return word0.getMisc() & 0xF;};
  inline unsigned int getDHHId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
};

struct dhhc_commode_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trigger_nr_lo;
  const unsigned short data[96];
  const unsigned int crc32;
  /// fixed length

  inline unsigned short getEventNrLo(void) const {
    return trigger_nr_lo;
  };
  inline unsigned int getFixedSize(void) const {
    return (4 + 96) * 2;// 100 words
  };
  inline unsigned int getDHHId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
};

struct dhhc_direct_readout_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trigger_nr_lo;
  /// an unbelievable amount of words may follow
  /// and finally a 32 bit checksum

  inline unsigned short getEventNrLo(void) const {
    return trigger_nr_lo;
  };
  void print(void) const {
    word0.print();
    B2INFO("DHHC Direct Readout (Raw|ZSD|ONS) Frame TNRLO $" << hex << trigger_nr_lo << " DHH ID $" << getDHHId() << " DHP port $" << getDHPPort());
  };
  inline unsigned short getDHHId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
  inline unsigned short getDHPPort(void) const {return (word0.getMisc()) & 0x3;};
  inline bool getDataReformattedFlag(void) const {return (word0.getMisc() >> 3) & 0x1;};
};


struct dhhc_direct_readout_frame_raw : public dhhc_direct_readout_frame {
};

struct dhhc_direct_readout_frame_zsd : public dhhc_direct_readout_frame {
};


struct dhhc_onsen_trigger_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trignr0;
  const unsigned int magic1;/// ?? brauch ich nicht
  const unsigned int trignr1;
  const unsigned int trigtag1;
  const unsigned int magic2;/// ?? brauch ich nicht
  const unsigned int trignr2;/// ?? brauch ich nicht
  const unsigned int trigtag2;/// ?? brauch ich nicht
  const unsigned int crc32;

  inline unsigned int getFixedSize(void) const {
    return 32;//  8*4 bytes might still be changed
  };
  inline unsigned short get_trig_nr0(void) const {
    return trignr0;
  };
  inline unsigned int get_trig_nr1(void) const {
    return trignr1;
  };
  inline unsigned int get_trig_nr2(void) const {
    return trignr2;
  };
  void print(void) const {
    word0.print();
    B2INFO("DHHC Trigger Frame TNRLO $" << hex << trignr0);
  };
  unsigned int check_error(void) const {
    return 0;
  }
};

struct dhhc_onsen_roi_frame {
  const dhhc_frame_header_word0 word0;/// mainly empty
  const unsigned short trignr0;// not used
  const unsigned int magic1;// not clear what will remain here, as this info will go to trigger frame.
  const unsigned int trignr1;// not clear what will remain here, as this info will go to trigger frame.
  const unsigned int trigtag1;// not clear what will remain here, as this info will go to trigger frame.
  const unsigned int magic2;// not clear what will remain here, as this info will go to trigger frame.
  const unsigned int trignr2;// not clear what will remain here, as this info will go to trigger frame.
  const unsigned int trigtag2;// not clear what will remain here, as this info will go to trigger frame.
  /// plus n* ROIs (64 bit)
  /// plus checksum 32bit

  inline unsigned short get_trig_nr0(void) const {
    return trignr0;
  };
  inline unsigned int get_trig_nr1(void) const {
    return trignr1;
  };
  inline unsigned int get_trig_nr2(void) const {
    return trignr2;
  };
  unsigned int check_error(bool ignore_datcon_flag = false) const {
    unsigned int m_errorMask = 0;
    if ((magic1 & 0xFFFF) != 0xCAFE) {
      B2ERROR("DHHC HLT/ROI Magic 1 error $" << hex << magic1);
      m_errorMask |= ONSEN_ERR_FLAG_HLTROI_MAGIC;
    }
    if ((magic2 & 0xFFFF) != 0xCAFE) {
      B2ERROR("DHHC HLT/ROI Magic 2 error $" << hex << magic2);
      m_errorMask |= ONSEN_ERR_FLAG_HLTROI_MAGIC;
    }
    if (magic2 == 0x0000CAFE && trignr2 == 0x00000000) {
      if (!ignore_datcon_flag) B2WARNING("DHHC HLT/ROI Frame: No DATCON data " << hex << trignr1 << "!=$" << trignr2);
      m_errorMask |= ONSEN_ERR_FLAG_NO_DATCON;
    } else {
      if (trignr1 != trignr2) {
        B2ERROR("DHHC HLT/ROI Frame Trigger Nr Mismatch $" << hex << trignr1 << "!=$" << trignr2);
        m_errorMask |= ONSEN_ERR_FLAG_MERGER_TRIGNR;
      }
    }
    return m_errorMask;
  };
  void print(void) const {
    word0.print();
    B2INFO("DHHC HLT/ROI Frame for Trigger HLT, DATCON: $" << hex << trignr1 << ", $" << trignr2);
  };

  unsigned int check_inner_crc(unsigned int length) const {
    if (length < 8) {
      B2ERROR("DHHC ONSEN HLT/ROI Frame too small!!!");
      return ONSEN_ERR_FLAG_MERGER_CRC;
    }
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[65536 * 2]; /// 128kB
    d = (unsigned char*) &magic1;/// without the DHHC header as its only an inner checksum!!!

    if (length > 65536 * 2) {
      B2WARNING("DHHC ONSEN HLT/ROI Frame CRC FAIL because of too large packet (>128kB)!");
    } else {
      for (unsigned int k = 0; k < length - 4; k += 2) { // -4
        crcbuffer[k] = d[k + 1];
        crcbuffer[k + 1] = d[k];
      }
      bocrc.process_bytes(crcbuffer, length - 8); /// -4
    }
    unsigned int c;
    c = bocrc.checksum();

    boost::spirit::endian::ubig32_t crc32;
    crc32 = *(boost::spirit::endian::ubig32_t*)(crcbuffer + length - 8);  /// -4

    if (c == crc32) {
//       if (verbose)
//         B2INFO("DHHC ONSEN HLT/ROI Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
//                << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " len $" << length);
      return 0;// O.K.
    } else {
//       crc_error++;
//       if (verbose) {
      B2ERROR("DHHC ONSEN HLT/ROI Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
              << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " len $" << length);
      /// others would be interessting but possible subjects to access outside of buffer
      /// << " " << * (unsigned int*)(d + length - 2) << " " << * (unsigned int*)(d + length + 0) << " " << * (unsigned int*)(d + length + 2));
      //if (length <= 64) {
      //  for (unsigned int i = 0; i < length / 4; i++) {
      //    B2ERROR("== " << i << "  $" << hex << ((unsigned int*)d)[i]);
      //  }
      //}
//       };
//       error_flag = true;
      return ONSEN_ERR_FLAG_MERGER_CRC;
    }
    return 0;// never reached
  };
  void save(StoreArray<PXDRawROIs>& sa, unsigned int length, unsigned int* data) const {
    // not clear what will remain here, as part of data (headers) will go to trigger frame.
    if (length < 4 + 4 + 4 * 4) {
      B2ERROR("DHHC ONSEN HLT/ROI Frame too small to hold any ROIs, did not save anything!");
      return;
    }
    unsigned int l;
    l = (length - 4 - 4 - 4 * 4) / 8;
    // for(unsigned int i=0; i<l*2; i++) data[5+i]=((data[5+i]>>16)&0xFFFF)| ((data[5+i]&0xFFFF)<<16);// dont do it here ... CRC will fail
    sa.appendNew(l, magic1, trignr1, magic2, trignr2, &data[5]);
  }

};

struct dhhc_ghost_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trigger_nr_lo;
  const unsigned int crc32;
  /// fixed length

  inline unsigned int getFixedSize(void) const {
    return 8;
  };
  void print(void) const {
    word0.print();
    B2INFO("DHHC Ghost Frame TNRLO " << hex << trigger_nr_lo << " DHH ID $" << getDHHId() << " DHP port $" << getDHPPort() << " CRC $");
  };
  inline unsigned short getDHHId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
  inline unsigned short getDHPPort(void) const {return (word0.getMisc()) & 0x3;};
};

struct dhhc_end_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trigger_nr_lo;
  const unsigned int wordsinevent;
  const unsigned int errorinfo;
  const unsigned int crc32;
  /// fixed length

  unsigned int get_words(void) const {
    return wordsinevent;
  }
  inline unsigned int getFixedSize(void) const {
    return 16;
  };
  bool isFakedData(void) const {
    if (word0.data != 0x6000) return false;
    if (trigger_nr_lo != 0) return false;
    if (wordsinevent != 0) return false;
    if (errorinfo != 0) return false;
    if (crc32 != 0xF7BCA507) return false;
    return true;
  };
  void print(void) const {
    word0.print();
    B2INFO("DHHC End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
           << " CRC " << hex << crc32);
  };
  inline unsigned int get_dhhc_id(void) const {return (word0.getMisc() >> 5) & 0xF;};
};

struct dhhc_dhh_end_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trigger_nr_lo;
  const unsigned int wordsinevent;
  const unsigned int errorinfo;
  const unsigned int crc32;
  /// fixed length

  unsigned int get_words(void) const {
    return wordsinevent;
  }
  inline unsigned int getFixedSize(void) const {
    return 16;
  };
  void print(void) const {
    word0.print();
    B2INFO("DHHC DHH End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
           << " CRC " << hex << crc32);
  };
  inline unsigned int getDHHId(void) const {return (word0.getMisc() >> 4) & 0x3F;};
};

///*********************************************************************************
///****************** DHHC Frame Wrapper Code starts here **************************
///*********************************************************************************

class dhhc_frames {
public:
  union {
    const void* data;/// no type
    const dhhc_onsen_trigger_frame* data_onsen_trigger_frame;
    const dhhc_start_frame* data_dhhc_start_frame;
    const dhhc_end_frame* data_dhhc_end_frame;
    const dhhc_dhh_start_frame* data_dhh_start_frame;
    const dhhc_dhh_end_frame* data_dhh_end_frame;
    const dhhc_commode_frame* data_commode_frame;
    const dhhc_direct_readout_frame* data_direct_readout_frame;
    const dhhc_direct_readout_frame_raw* data_direct_readout_frame_raw;
    const dhhc_direct_readout_frame_zsd* data_direct_readout_frame_zsd;
    const dhhc_ghost_frame* data_ghost_frame;
    const dhhc_onsen_roi_frame* data_onsen_roi_frame;
  };
  unsigned int datasize;
  int type;
  int length;

  dhhc_frames(void) {
    data = 0;
    datasize = 0;
    type = -1;
    length = 0;
  };
  int getFrameType(void) {
    return type;
  };
  void set(void* d, unsigned int t) {
    data = d;
    type = t;
    length = 0;
  };
  void set(void* d, unsigned int t, unsigned int l) {
    data = d;
    type = t;
    length = l;
  };
  void set(void* d) {
    data = d;
    type = ((dhhc_frame_header_word0*)data)->getFrameType();
    length = 0;
  };
  unsigned int getEventNrLo(void) {
    return ((unsigned short*)data)[1];
  };
  unsigned int check_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[65536 * 16]; /// 1MB
    d = (unsigned char*)data;

    if (length > 65536 * 16) {
      B2WARNING("DHHC Data Frame CRC not calculated because of too large packet (>1MB)!");
    } else {
      for (int k = 0; k < length - 4; k += 2) {
        crcbuffer[k] = d[k + 1];// annoying... revert endian swap :-b
        crcbuffer[k + 1] = d[k];
      }
      bocrc.process_bytes(crcbuffer, length - 4);
    }
    unsigned int c;
    c = bocrc.checksum();

    boost::spirit::endian::ubig32_t crc32;
    crc32 = *(boost::spirit::endian::ubig32_t*)(d + length - 4);

    if (c == crc32) {
//       if (verbose)
      //         B2INFO("DHH Data Frame CRC: " << hex << c << "==" << crc32);
//         B2INFO("DHHC Data Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
//                << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " len $" << length);
    } else {
//       crc_error++;
//       if (verbose) {
      B2ERROR("DHHC Data Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
              << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " len $" << length);
      /// others would be interessting but possible subjects to access outside of buffer
      /// << " " << * (unsigned int*)(d + length - 2) << " " << * (unsigned int*)(d + length + 0) << " " << * (unsigned int*)(d + length + 2));
      //if (length <= 32) {
      //  for (int i = 0; i < length / 4; i++) {
      //    B2ERROR("== " << i << "  $" << hex << ((unsigned int*)d)[i]);
      //  }
      //}
//       };
//       error_flag = true;
      return ONSEN_ERR_FLAG_DHH_CRC;
    }
    return 0;
  };


  unsigned int getFixedSize(void) {
    unsigned int s = 0;
    switch (getFrameType()) {
      case DHHC_FRAME_HEADER_DATA_TYPE_DHP_RAW:
        s = 0; /// size is not a fixed number
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP:
      case DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD:
        s = 0; /// size is not a fixed number
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE:
      case DHHC_FRAME_HEADER_DATA_TYPE_FCE_RAW:
        B2INFO("Error: FCE type not yet supported ");
        s = 0; /// size is not a fixed number
//         error_flag = true;
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_COMMODE:
        s = data_commode_frame->getFixedSize();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_GHOST:
        s = data_ghost_frame->getFixedSize();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_DHH_START:
        s = data_dhh_start_frame->getFixedSize();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_DHH_END:
        s = data_dhh_end_frame->getFixedSize();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START:
        s = data_dhhc_start_frame->getFixedSize();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END:
        s = data_dhhc_end_frame->getFixedSize();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI:
        s = 0; /// size is not a fixed number
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG:
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

PXDUnpackerModule::PXDUnpackerModule() :
  Module(),
  m_storeRawHits(),
  m_storeROIs(),
  m_storeRawAdc(),
  m_storeRawPedestal()
{
  //Set module properties
  setDescription("Unpack Raw PXD Hits from ONSEN data stream");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("HeaderEndianSwap", m_headerEndianSwap, "Swap the endianess of the ONSEN header", true);
  addParam("IgnoreDATCON", m_ignoreDATCON, "Ignore missing DATCON ROIs", false);
  addParam("DoNotStore", m_doNotStore, "only unpack and check, but do not store", false);

}

void PXDUnpackerModule::initialize()
{
  //Register output collections
  m_storeRawHits.registerAsPersistent();
  m_storeRawAdc.registerAsPersistent();
  m_storeRawPedestal.registerAsPersistent();
  m_storeROIs.registerAsPersistent();
  /// actually, later we do not want to store ROIs and Pedestals into output file ...  aside from debugging

  B2INFO("HeaderEndianSwap: " << m_headerEndianSwap);
  B2INFO("Ignore(missing)DATCON: " << m_ignoreDATCON);

  ignore_datcon_flag = m_ignoreDATCON;

  m_unpackedEventsCount = 0;
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) m_errorCounter[i] = 0;
}

void PXDUnpackerModule::terminate()
{
  const string error_name[ONSEN_MAX_TYPE_ERR] = {
    "FTSW/DHHC mismatch", "DHHC/DHH mismatch", "-unused-", "ONSEN Trigger is not first frame",
    "DHHC_END missing", "DHH_START missing", "DHHC Framecount mismatch", "DATA outside of DHH",
    "DHHC_START is not second frame", "-unused-", "Fixed size frame wrong size", "DHH CRC Error:",
    "Unknown DHHC type", "Merger CRC Error", "Event Header Full Packet Size Error", "Event Header Magic Error",
    "Event Header Frame Count Error", "Event header Frame Size Error", "HLTROI Magic Error", "Merger HLT/DATCON TrigNr Mismatch",
    "DHP Size too small", "DHP-DHH DHHID mismatch", "DHP-DHH Port mismatch", "DHP Pix w/o row",
    "DHH START/END ID mismatch", "DHH ID mismatch of START and this frame", "DHH_START w/o prev END", "Nr PXD data ==0",
    "Missing Datcon", "NO DHHC data for Trigger", "DHH active mismatch", "DHP active mismatch"
  };

  int flag = 0;
  string errstr = "Statistic ( ;";
  errstr += to_string(m_unpackedEventsCount) + ";";
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) { errstr += to_string(m_errorCounter[i]) + ";"; flag |= m_errorCounter[i];}
  if (flag != 0) {
    B2ERROR("PXD Unpacker --> Error Statistics (counted once per event!) in Events: " << m_unpackedEventsCount);
    B2ERROR(errstr + " )");
    for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      if (m_errorCounter[i]) {
        B2ERROR(error_name[i] << ": " << m_errorCounter[i]);
        //printf("%s %d\n",error_name[i].c_str(), m_errorCounter[i]);
      }
    }
  } else {
    B2INFO("PXD Unpacker --> No Error found in Events: " << m_unpackedEventsCount);
  }
}

void PXDUnpackerModule::event()
{
  StoreArray<RawPXD> storeRaws;
  StoreArray<RawFTSW> storeFTSW;
  StoreObjPtr<EventMetaData> evtPtr;/// what will happen if it does not exist???

  int nRaws = storeRaws.getEntries();
  if (verbose) {
    B2INFO("PXD Unpacker --> RawPXD Objects in event: " << nRaws);
  };

  m_errorMask = 0;
  ftsw_evt_nr = 0;
  ftsw_evt_mask = 0;
  for (auto & it : storeFTSW) {
    ftsw_evt_nr = it.GetEveNo(0);
    ftsw_evt_mask = 0x7FFF;
    B2INFO("PXD Unpacker --> FTSW Event Number: $" << hex << ftsw_evt_nr);
    break;
  }


  m_meta_event_nr = evtPtr->getEvent();
  m_meta_run_nr = evtPtr->getRun();
  m_meta_subrun_nr = evtPtr->getSubrun();
  m_meta_experiment = evtPtr->getExperiment();
  /// evtPtr->getTime()


  int nsr = 0;
  for (auto & it : storeRaws) {
    if (verbose) {
      B2INFO("PXD Unpacker --> Unpack Objects: ");
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

void PXDUnpackerModule::endian_swap_frame(unsigned short* dataptr, int len)
{
  boost::spirit::endian::ubig16_t* p = (boost::spirit::endian::ubig16_t*)dataptr;

  /// swap endianess of all shorts in frame BUT not the CRC (2 shorts)
  for (int i = 0; i < len / 2 - 2; i++) {
    dataptr[i] = p[i];// Endian Swap! (it doesnt matter if you swap from little to big or vice versa)
  }
}

void PXDUnpackerModule::unpack_event(RawPXD& px)
{
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD Unpacker --> invalid packet size (32bit words) " << hex << px.size());
    m_errorMask |= ONSEN_ERR_FLAG_PACKET_SIZE;
    return;
  }
  unsigned int data[px.size()];
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  memcpy(data, (unsigned int*)px.data(), fullsize);

  if (fullsize < 8) {
    B2ERROR("Data is to small to hold a valid Header! Will not unpack anything. Size:" << fullsize);
    m_errorMask |= ONSEN_ERR_FLAG_PACKET_SIZE;
    return;
  }

  if (data[0] != 0xCAFEBABE && data[0] != 0xBEBAFECA) {
    B2ERROR("Magic invalid: Will not unpack anything. Header corrupted! " << hex << data[0]);
    m_errorMask |= ONSEN_ERR_FLAG_MAGIC;
    return;
  }

  if (m_headerEndianSwap) Frames_in_event = ((boost::spirit::endian::ubig32_t*)data)[1]; else Frames_in_event = ((boost::spirit::endian::ulittle32_t*)data)[1];
  if (Frames_in_event < 0 || Frames_in_event > 256) {
    B2ERROR("Number of Frames invalid: Will not unpack anything. Header corrupted! Frames in event: " << Frames_in_event);
    m_errorMask |= ONSEN_ERR_FLAG_FRAME_NR;
    return;
  }

  /// NEW format
  if (verbose) {
    B2INFO("PXD Unpacker --> data[0]: <-- Magic " << hex << data[0]);
    B2INFO("PXD Unpacker --> data[1]: <-- #Frames " << hex << data[1]);
    if (data[1] >= 1 && fullsize < 12) B2INFO("PXD Unpacker --> data[2]: <-- Frame 1 len " << hex << data[2]);
    if (data[1] >= 2 && fullsize < 16) B2INFO("PXD Unpacker --> data[3]: <-- Frame 2 len " << hex << data[3]);
    if (data[1] >= 3 && fullsize < 20) B2INFO("PXD Unpacker --> data[4]: <-- Frame 3 len " << hex << data[4]);
    if (data[1] >= 4 && fullsize < 24) B2INFO("PXD Unpacker --> data[5]: <-- Frame 4 len " << hex << data[5]);
  };

  unsigned int* tableptr;
  tableptr = &data[2]; // skip header!!!

  unsigned int* dataptr;
  dataptr = &tableptr[Frames_in_event];
  datafullsize = fullsize - 2 * 4 - Frames_in_event * 4; // minus header, minus table

  int ll = 0; // Offset in dataptr in bytes
  for (int j = 0; j < Frames_in_event; j++) {
    int lo;/// len of frame in bytes
    if (m_headerEndianSwap) lo = ((boost::spirit::endian::ubig32_t*)tableptr)[j]; else lo = ((boost::spirit::endian::ulittle32_t*)tableptr)[j];
    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      m_errorMask |= ONSEN_ERR_FLAG_FRAME_SIZE;
      return;
    }
    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " << datafullsize << " of fullsize " << fullsize);
      m_errorMask |= ONSEN_ERR_FLAG_FRAME_SIZE;
      return;
    }
    if (lo & 0x3) {
      B2ERROR("SKIP Frame with Data with not MOD 4 length " << " ( $" << hex << lo << " ) ");
      ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
    } else {
      B2INFO("unpack DHH(C) frame: " << j << " with size " << lo << " at byte offset in dataptr " << ll);
      endian_swap_frame((unsigned short*)(ll + (char*)dataptr), lo);
      unpack_dhhc_frame(ll + (char*)dataptr, lo, j, Frames_in_event);
      ll += lo; /// no rounding needed
    }
  }

}

void PXDUnpackerModule::unpack_dhp_raw(void* data, unsigned int frame_len, unsigned int dhh_ID, unsigned dhh_DHPport, VxdID vxd_id)
{
//   unsigned int nr_words = frame_len / 2; // frame_len in bytes (excl. CRC)!!!
  unsigned short* dhp_pix = (unsigned short*)data;
  // ADC/ADC and ADC/PEDESTAL can only be distinguised by length of frame

  if (frame_len != 0x10008 && frame_len != 0x20008) {
    B2ERROR("Frame size unsupported for RAW pedestal frame! $" << hex << frame_len << " bytes");
    return;
  }
  unsigned int dhp_header_type  = 0;
//   unsigned int dhp_reserved     = 0;
  unsigned int dhp_dhh_id       = 0;
  unsigned int dhp_dhp_id       = 0;

  dhp_header_type  = (dhp_pix[2] >> 13) & 0x07;
//   dhp_reserved     = (dhp_pix[2] >> 8) & 0x1F;
  dhp_dhh_id       = (dhp_pix[2] >> 2) & 0x3F;
  dhp_dhp_id       =  dhp_pix[2] & 0x03;

  if (dhh_ID != dhp_dhh_id) {
    B2ERROR("DHH ID in DHH and DHP header differ $" << hex << dhh_ID << " != $" << dhp_dhh_id);
    m_errorMask |= ONSEN_ERR_FLAG_DHH_DHP_DHHID;
  }
  if (dhh_DHPport != dhp_dhp_id) {
    B2ERROR("DHP ID (Chip/Port) in DHH and DHP header differ $" << hex << dhh_DHPport << " != $" << dhp_dhp_id);
    m_errorMask |= ONSEN_ERR_FLAG_DHH_DHP_PORT;
  }

  if (dhp_header_type != DHP_FRAME_HEADER_DATA_TYPE_RAW) {
    B2ERROR("Header type invalid for this kind of DHH frame: $" << hex << dhp_header_type);
    return;
  }

  if (frame_len == 0x10008) { // 64k
    B2INFO("Pedestal Data - (ADC:ADC) - not implemented... !");
    m_storeRawAdc.appendNew(vxd_id, data, false);
  } else if (frame_len == 0x20008) { // 128k
    B2INFO("Pedestal Data - (ADC:Pedestal) - not implemented... !");
    m_storeRawAdc.appendNew(vxd_id, data, true);
    m_storeRawPedestal.appendNew(vxd_id, data);
  } else {
    // checked already above
  }
};


void PXDUnpackerModule::unpack_dhp(void* data, unsigned int frame_len, unsigned int dhh_first_readout_frame_id_lo, unsigned int dhh_ID, unsigned dhh_DHPport, unsigned dhh_reformat, unsigned short toffset, VxdID vxd_id)
{
  unsigned int nr_words = frame_len / 2; // frame_len in bytes (excl. CRC)!!!
  bool printflag = false;
  unsigned short* dhp_pix = (unsigned short*)data;

  unsigned int dhp_readout_frame_lo = 0;
  unsigned int dhp_header_type  = 0;
  unsigned int dhp_reserved     = 0;
  unsigned int dhp_dhh_id       = 0;
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
  dhp_header_type  = (dhp_pix[2] >> 13) & 0x07;
  dhp_reserved     = (dhp_pix[2] >> 8) & 0x1F;
  dhp_dhh_id       = (dhp_pix[2] >> 2) & 0x3F;
  dhp_dhp_id       =  dhp_pix[2] & 0x03;

  if (printflag) {
    B2INFO("DHP type     | " << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
    B2INFO("DHP reserved | " << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
    B2INFO("DHP DHH ID   | " << hex << dhp_dhh_id << " ( " << dec << dhp_dhh_id << " ) ");
    B2INFO("DHP DHP ID   | " << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  }

  if (dhh_ID != dhp_dhh_id) {
    B2ERROR("DHH ID in DHH and DHP header differ $" << hex << dhh_ID << " != $" << dhp_dhh_id);
    m_errorMask |= ONSEN_ERR_FLAG_DHH_DHP_DHHID;
  }
  if (dhh_DHPport != dhp_dhp_id) {
    B2ERROR("DHP ID (Chip/Port) in DHH and DHP header differ $" << hex << dhh_DHPport << " != $" << dhp_dhp_id);
    m_errorMask |= ONSEN_ERR_FLAG_DHH_DHP_PORT;
  }

  if (dhp_header_type != DHP_FRAME_HEADER_DATA_TYPE_ZSD) {
    B2ERROR("Header type invalid for this kind of DHH frame: $" << hex << dhp_header_type);
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
      if (((dhp_pix[i] >> 15) & 0x1) == 0) {
        rowflag = true;
        dhp_row = (dhp_pix[i] >> 5) & 0xFFE;
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
          dhp_row = (dhp_row & 0xFFE) | ((dhp_pix[i] >> 14) & 0x001);
          dhp_col = ((dhp_pix[i] >> 8) & 0x3F);
          ///  remapping flag
          if (dhh_reformat == 0) dhp_col ^= 0x3C ; /// 0->60 61 62 63 4->56 57 58 59 ...
          dhp_col += dhp_offset;
          dhp_adc = dhp_pix[i] & 0xFF;
          if (printflag)
            B2INFO("SetPix: Row " << hex << dhp_row << " Col " << hex << dhp_col << " ADC " << hex << dhp_adc
                   << " CM " << hex << dhp_cm);

          /*if (verbose) {
            B2INFO("raw    |   " << hex << d[i]);
            B2INFO("row " << hex << ((d[i] >> 20) & 0xFFF) << "(" << ((d[i] >> 20) & 0xFFF) << ")" << " col " << "(" << hex << ((d[i] >> 8) & 0xFFF) << ((d[i] >> 8) & 0xFFF)
                   << " adc " << "(" << hex << (d[i] & 0xFF) << (d[i] & 0xFF) << ")");
            B2INFO("dhh_ID " << dhh_ID);
            B2INFO("start-Frame-Nr " << dec << dhh_first_readout_frame_id_lo);
            B2INFO("toffset " << toffset);
          };*/

          if (!m_doNotStore) m_storeRawHits.appendNew(vxd_id, dhp_row, dhp_col, dhp_adc,
                                                        toffset, (dhp_readout_frame_lo - dhh_first_readout_frame_id_lo) & 0x3F, dhp_cm
                                                       );
        }
      }
    }
  }

  if (printflag) {
    B2INFO("(DHH) DHH_ID " << hex << dhh_ID << " (DHH) DHP ID  " << hex << dhh_DHPport << " (DHP) DHH_ID " << hex << dhp_dhh_id << " (DHP) DHP ID " << hex << dhp_dhp_id);
    /*for (int i = 0; i < raw_nr_words ; i++) {
      B2INFO("RAW      |   " << hex << p_pix[i]);
      printf("raw %08X  |  ", p_pix[i]);
      B2INFO("row " << hex << ((p_pix[i] >> 20) & 0xFFF) << dec << " ( " << ((p_pix[i] >> 20) & 0xFFF) << " ) " << " col " << hex << ((p_pix[i] >> 8) & 0xFFF)
             << " ( " << dec << ((p_pix[i] >> 8) & 0xFFF) << " ) " << " adc " << hex << (p_pix[i] & 0xFF) << " ( " << (p_pix[i] & 0xFF) << " ) "
            );
    }*/
  }
};

int PXDUnpackerModule::nr5bits(int i) const
{
  /// too lazy to count the bits myself, thus using a small lookup table
  const int lut[32] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5
  };
  return lut[i & 0x1F];
}

void PXDUnpackerModule::unpack_dhhc_frame(void* data, const int len, const int Frame_Number, const int Frames_in_event)
{
  /// The following STATIC variables are used to save some state or cound some things
  /// while depacking the frames. they are in most cases (re)set on the first frame or ONSEN trg frame
  /// Most could put in as a class member, but they are only needed within this function
  static unsigned int eventNrOfOnsenTrgFrame = 0;
  static int countedWordsInEvent;// count the size of all frames in words
  static int nr_of_dhh_start_frame = 0;
  static int nr_of_dhh_end_frame = 0;
  //  static int nr_of_frames_dhhc = 0;/// tbd if and where this number will still show up in te DHHC format TODO
  static int nr_of_frames_counted = 0;/// eould not be needed if nr_of_frames_dhhc is not within the data structure
  static int nr_active_dhh = 0;
  static int mask_active_dhh = 0;
  //  static int nr_active_dhp = 0;// unused
  static int mask_active_dhp = 0;
  static int found_mask_active_dhp = 0;
  static unsigned int dhh_first_readout_frame_id_lo = 0;
  static unsigned int dhh_first_offset = 0;
  static unsigned int currentDHHID = 0xFFFFFFFF;
  static unsigned int currentVxdId = 0;
  static bool isFakedData_event = false;


  dhhc_frame_header_word0* hw = (dhhc_frame_header_word0*)data;
//   error_flag = false;

  dhhc_frames dhhc;
  dhhc.set(data, hw->getFrameType(), len);
  int s;
  s = dhhc.getFixedSize();
  if (len != s && s != 0) {
    B2ERROR("Fixed frame type size does not match specs: expect " << len << " != " << s << " (in data) ");
    m_errorMask |= ONSEN_ERR_FLAG_FIX_SIZE;
  }

  unsigned int eventNrOfThisFrame = dhhc.getEventNrLo();
  int type = dhhc.getFrameType();

  if (Frame_Number == 0) { /// We reset the counters on the first event
    nr_of_dhh_start_frame = 0;
    nr_of_dhh_end_frame = 0;
    currentDHHID = 0xFFFFFFFF;
    currentVxdId = 0;

    isFakedData_event = false;
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START) {
      B2ERROR("This looks like this is the old Desy 2013/14 testbeam format. Please use the pxdUnpackerDesy1314 module.");
    }
  }

  if (Frame_Number == 1) {
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START) {
      isFakedData_event = dhhc.data_dhhc_start_frame->isFakedData();
    }
  }



  if ((eventNrOfThisFrame & ftsw_evt_mask) != (ftsw_evt_nr & ftsw_evt_mask)) {
    if (isFakedData_event) {
      /// If ist a start, check if its a fake event, if its not a start, the fake flag is already (re)set. As long as the data structure is not messed up completly.
      /// no need to explicitly check the DHHC_END or HLT/ROI
      /// We have a fake and should set the trigger nr by hand to prevent further errors
      eventNrOfThisFrame = ftsw_evt_nr & ftsw_evt_mask; // masking might be a problem as we cannore recover all bits
    } else {
      B2ERROR("Event Numbers do not match for this frame $" << hex << eventNrOfThisFrame << "!=$" << ftsw_evt_nr << "(FTSW) mask $" << ftsw_evt_mask);
      m_errorMask |= ONSEN_ERR_FLAG_FTSW_DHHC_MM;
    }
  }

  if ((eventNrOfThisFrame & 0xFFFF) != (m_meta_event_nr & 0xFFFF)) {
    B2ERROR("Event Numbers do not match for this frame $" << hex << eventNrOfThisFrame << "!=$" << m_meta_event_nr << "(MetaInfo) mask");
    m_errorMask |= ONSEN_ERR_FLAG_FTSW_DHHC_MM;
  }

  if (Frame_Number > 1 && Frame_Number < Frames_in_event - 1) {
    if (nr_of_dhh_start_frame != nr_of_dhh_end_frame + 1)
      if (type != DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI && type != DHHC_FRAME_HEADER_DATA_TYPE_DHH_START) {
        B2ERROR("Data Frame outside a DHH START/END");
        m_errorMask |= ONSEN_ERR_FLAG_DATA_OUTSIDE;
      }
  }

  switch (type) {
    case DHHC_FRAME_HEADER_DATA_TYPE_DHP_RAW: {
      nr_of_frames_counted++;

      if (verbose) dhhc.data_direct_readout_frame_raw->print();
      if (currentDHHID != dhhc.data_direct_readout_frame_raw->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << currentDHHID << " != $" << dhhc.data_direct_readout_frame_raw->getDHHId());
        m_errorMask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      m_errorMask |= dhhc.check_crc();
      found_mask_active_dhp |= 1 << dhhc.data_direct_readout_frame->getDHPPort();

//       stat_raw++;

      unpack_dhp_raw(data, len - 4,
                     dhhc.data_direct_readout_frame->getDHHId(),
                     dhhc.data_direct_readout_frame->getDHPPort(),
                     currentVxdId);

      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP:
    case DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD: {
      nr_of_frames_counted++;

      if (verbose)dhhc.data_direct_readout_frame->print();

      //m_errorMask |= dhhc.data_direct_readout_frame->check_error();

      if (currentDHHID != dhhc.data_direct_readout_frame_raw->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << currentDHHID << " != $" << dhhc.data_direct_readout_frame_raw->getDHHId());
        m_errorMask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      m_errorMask |= dhhc.check_crc();
      found_mask_active_dhp |= 1 << dhhc.data_direct_readout_frame->getDHPPort();
//       stat_zsd++;

      unpack_dhp(data, len - 4,
                 dhh_first_readout_frame_id_lo,
                 dhhc.data_direct_readout_frame->getDHHId(),
                 dhhc.data_direct_readout_frame->getDHPPort(),
                 dhhc.data_direct_readout_frame->getDataReformattedFlag(),
                 dhh_first_offset, currentVxdId);
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE:
    case DHHC_FRAME_HEADER_DATA_TYPE_FCE_RAW: {
      nr_of_frames_counted++;

      if (verbose) hw->print();
      if (currentDHHID != dhhc.data_direct_readout_frame_raw->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << currentDHHID << " != $" << dhhc.data_direct_readout_frame_raw->getDHHId());
        m_errorMask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      m_errorMask |= dhhc.check_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_COMMODE: {
      nr_of_frames_counted++;

      if (verbose) hw->print();
      if (currentDHHID != dhhc.data_commode_frame->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << currentDHHID << " != $" << dhhc.data_commode_frame->getDHHId());
        m_errorMask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      m_errorMask |= dhhc.check_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START: {
      if (isFakedData_event != dhhc.data_dhhc_start_frame->isFakedData()) {
        B2ERROR("DHHC START is but no Fake event OR Fake Event but DHH END is not.");
      }
      if (dhhc.data_dhhc_start_frame->isFakedData()) {
        B2WARNING("Faked DHHC START Data -> trigger without Data!");
        m_errorMask |= ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhhc.data_dhhc_start_frame->print();
      }

//      eventNrOfOnsenTrgFrame = eventNrOfThisFrame;
      currentDHHID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      //nr_of_frames_dhhc = dhhc.data_dhhc_start_frame->get_dhhc_nr_frames();
      nr_of_frames_counted = 1;
      m_errorMask |= dhhc.check_crc();
//       stat_start++;

//   m_meta_event_nr=evtPtr->getEvent();
//   m_meta_run_nr=evtPtr->getRun();
//   n_meta_subrun_nr=evtPtr->getSubrun();
//   m_meta_experiment=evtPtr->getExperiment();

      mask_active_dhh = dhhc.data_dhhc_start_frame->get_active_dhh_mask();
      nr_active_dhh = nr5bits(mask_active_dhh);
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHH_START: {
      nr_of_frames_counted++;
      if (verbose)dhhc.data_dhh_start_frame->print();
      dhh_first_readout_frame_id_lo = dhhc.data_dhh_start_frame->getStartFrameNr();
      dhh_first_offset = dhhc.data_dhh_start_frame->getTriggerOffsetRow();
      currentDHHID = dhhc.data_dhh_start_frame->getDHHId();
      m_errorMask |= dhhc.check_crc();

      if (nr_of_dhh_start_frame != nr_of_dhh_end_frame) {
        B2ERROR("DHHC_FRAME_HEADER_DATA_TYPE_DHH_START without DHHC_FRAME_HEADER_DATA_TYPE_DHH_END");
        m_errorMask |= ONSEN_ERR_FLAG_DHH_START_WO_END;
      }
      nr_of_dhh_start_frame++;

      found_mask_active_dhp = 0;
      mask_active_dhp = dhhc.data_dhh_start_frame->getActiveDHPMask();
      ///      nr_active_dhp = nr5bits(mask_active_dhp);// unused

      // calculate the VXDID for DHH and save them for DHP unpacking
      {
        /// refering to BelleII Note Nr 0010, the numbers run from ... to
        ///   unsigned int layer, ladder, sensor;
        ///   layer= vxdid.getLayerNumber();/// 1 ... 2
        ///   ladder= vxdid.getLadderNumber();/// 1 ... 8 and 1 ... 12
        ///   sensor= vxdid.getSensorNumber();/// 1 ... 2
        ///   dhh_id = ((layer-1)<<5) | ((ladder)<<1) | (sensor-1);
        unsigned short sensor, ladder, layer;
        sensor = (currentDHHID & 0x1) + 1;
        ladder = (currentDHHID & 0x1E) >> 1; // no +1
        layer = ((currentDHHID & 0x20) >> 5) + 1;
        currentVxdId = VxdID(layer, ladder, sensor);
      }
      //currentVxdId = currentDHHID;

      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_GHOST:
      nr_of_frames_counted++;
      if (verbose)dhhc.data_ghost_frame->print();
      if (currentDHHID != dhhc.data_ghost_frame->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << currentDHHID << " != $" << dhhc.data_ghost_frame->getDHHId());
        m_errorMask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      /// Attention: Firmware might be changed such, that ghostframe come for all DHPs, not only active ones...
      found_mask_active_dhp |= 1 << dhhc.data_ghost_frame->getDHPPort();
      m_errorMask |= dhhc.check_crc();
//       stat_ghost++;

      break;
    case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END: {
      if (dhhc.data_dhhc_end_frame->isFakedData() != isFakedData_event) {
        B2ERROR("DHHC END is but no Fake event OR Fake Event but DHH END is not.");
      }
      nr_of_frames_counted++;
      currentDHHID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      if (isFakedData_event) {
        B2WARNING("Faked DHHC END Data -> trigger without Data!");
        m_errorMask |= ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhhc.data_dhhc_end_frame->print();
      }
//       stat_end++;

      if (!isFakedData_event) {
//         if (nr_of_frames_counted != nr_of_frames_dhhc) { /// Nr Frames has been removed .. tbd
//           if (!m_ignore_headernrframes) B2ERROR("Number of DHHC Frames in Header(??? Tail?) " << nr_of_frames_dhhc << " != " << nr_of_frames_counted << " Counted");
//           m_errorMask |= ONSEN_ERR_FLAG_DHHC_FRAMECOUNT;
//         }
      }
      if (!isFakedData_event) {
        int w;
        w = dhhc.data_dhhc_end_frame->get_words() * 2;
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
      m_errorMask |= dhhc.check_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHH_END: {
      nr_of_frames_counted++;
      if (verbose) dhhc.data_dhh_end_frame->print();
      if (currentDHHID != dhhc.data_dhh_end_frame->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << currentDHHID << " != $" << dhhc.data_dhh_end_frame->getDHHId());
        m_errorMask |= ONSEN_ERR_FLAG_DHH_START_END_ID;
      }
      currentDHHID = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      m_errorMask |= dhhc.check_crc();
      if (found_mask_active_dhp != mask_active_dhp) {
        B2ERROR("DHH_END: DHP active mask $" << hex << mask_active_dhp << " != $" << hex << found_mask_active_dhp << " mask of found dhp/ghost frames");
        m_errorMask |= ONSEN_ERR_FLAG_DHP_ACTIVE;
      }
      nr_of_dhh_end_frame++;
      if (nr_of_dhh_start_frame != nr_of_dhh_end_frame) {
        B2ERROR("DHHC_FRAME_HEADER_DATA_TYPE_DHH_END without DHHC_FRAME_HEADER_DATA_TYPE_DHH_START");
        m_errorMask |= ONSEN_ERR_FLAG_DHH_START;
      }
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI:
      nr_of_frames_counted += 0; /// DO NOT COUNT!!!!
      if (verbose) dhhc.data_onsen_roi_frame->print();
      m_errorMask |= dhhc.data_onsen_roi_frame->check_error(ignore_datcon_flag);
      m_errorMask |= dhhc.data_onsen_roi_frame->check_inner_crc(len - 4); /// CRC is without the DHHC header
      m_errorMask |= dhhc.check_crc();
      if (!m_doNotStore) dhhc.data_onsen_roi_frame->save(m_storeROIs, len, (unsigned int*) data);
      break;
    case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG:
      nr_of_frames_counted += 0; /// DO NOT COUNT!!!!
      eventNrOfOnsenTrgFrame = eventNrOfThisFrame;
//   m_meta_event_nr=evtPtr->getEvent();
//   m_meta_run_nr=evtPtr->getRun();
//   n_meta_subrun_nr=evtPtr->getSubrun();
//   m_meta_experiment=evtPtr->getExperiment();
      if (verbose) dhhc.data_onsen_trigger_frame->print();
      m_errorMask |= dhhc.data_onsen_trigger_frame->check_error();
      m_errorMask |= dhhc.check_crc();
      if (Frame_Number != 0) {
        B2ERROR("ONSEN TRG Frame must be the first one.");
      }
      break;
    default:
      B2ERROR("UNKNOWN DHHC frame type");
      m_errorMask |= ONSEN_ERR_FLAG_DHHC_UNKNOWN;
//      type_error++;
      if (verbose) hw->print();
//       error_flag = true;
      break;
  }

  if (eventNrOfThisFrame != eventNrOfOnsenTrgFrame) {
    B2ERROR("Frame TrigNr != ONSEN Trig Nr $" << hex << eventNrOfThisFrame << " != $" << eventNrOfOnsenTrgFrame);
    m_errorMask |= ONSEN_ERR_FLAG_DHHC_DHH_MM;
//     evtnr_error++;
//     error_flag = true;
  }

  if (Frame_Number == 0) {
    /// Check that DHHC Start is first Frame
    if (type != DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG) {
      B2ERROR("First frame is not a ONSEN Trigger frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= ONSEN_ERR_FLAG_ONSEN_TRG_FIRST;
    }
  } else { // (Frame_Number != 0 &&
    /// Check that there is no other DHHC Start
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG) {
      B2ERROR("More than one ONSEN Trigger frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= ONSEN_ERR_FLAG_ONSEN_TRG_FIRST;
    }
  }

  if (Frame_Number == 1) {
    /// Check that DHHC Start is first Frame
    if (type != DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START) {
      B2ERROR("Second frame is not a DHHC start of subevent frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= ONSEN_ERR_FLAG_DHHC_START_SECOND;
    }
  } else { // (Frame_Number != 0 &&
    /// Check that there is no other DHHC Start
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START) {
      B2ERROR("More than one DHHC start of subevent frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= ONSEN_ERR_FLAG_DHHC_START_SECOND;
    }
  }

  if (Frame_Number == Frames_in_event - 1) {
    /// Check that DHHC End is last Frame
    if (type != DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END) {
      B2ERROR("Last frame is not a DHHC end of subevent frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= ONSEN_ERR_FLAG_DHHC_END;
    }

    /// As we now have processed the whole event, we can do some more consistency checks!
    if (nr_of_dhh_start_frame != nr_of_dhh_end_frame || nr_of_dhh_start_frame != nr_active_dhh) {
      B2ERROR("The number of DHH Start/End does not match the number of active DHH in DHHC Header! Header: " << nr_active_dhh << " Start: " << nr_of_dhh_start_frame << " End: " << nr_of_dhh_end_frame << " Mask: $" << hex << mask_active_dhh << " in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= ONSEN_ERR_FLAG_DHH_ACTIVE;
    }

  } else { //  (Frame_Number != Frames_in_event - 1 &&
    /// Check that there is no other DHHC End
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END) {
      B2ERROR("More than one DHHC end of subevent frame in frame in Event Nr " << eventNrOfThisFrame);
      m_errorMask |= ONSEN_ERR_FLAG_DHHC_END;
    }
  }

  /// Check that (if there is at least one active DHH) the second Frame is DHH Start, actually this is redundant if the other checks work
  if (Frame_Number == 2 && nr_active_dhh != 0 && type != DHHC_FRAME_HEADER_DATA_TYPE_DHH_START) {
    B2ERROR("Third frame is not a DHH start frame in Event Nr " << eventNrOfThisFrame);
    m_errorMask |= ONSEN_ERR_FLAG_DHH_START;
  }

  countedWordsInEvent += len;

}



