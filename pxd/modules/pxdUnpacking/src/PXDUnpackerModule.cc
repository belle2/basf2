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

// // for htonl
#include <arpa/inet.h>

#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

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
#define DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI      0xF
#define DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG      0xE
// Free IDs are 0x7 0x8 0xA

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDUnpacker)

/// If you chnage this list, change the NAMEs in the terminate function, too
#define ONSEN_ERR_FLAG_FTSW_DHHC_MM 0x00000001ul
#define ONSEN_ERR_FLAG_DHHC_DHH_MM  0x00000002ul
//#define ONSEN_ERR_FLAG_DHHC_DHP_MM  0x00000004ul // unsused
#define ONSEN_ERR_FLAG_DHHC_START 0x00000008ul
#define ONSEN_ERR_FLAG_DHHC_END   0x00000010ul
#define ONSEN_ERR_FLAG_DHH_START  0x00000020ul
#define ONSEN_ERR_FLAG_DHHC_FRAMECOUNT 0x00000040ul
#define ONSEN_ERR_FLAG_DATA_OUTSIDE 0x00000080ul
#define ONSEN_ERR_FLAG_DHHC_START2  0x00000100ul
#define ONSEN_ERR_FLAG_DHHC_END2  0x00000200ul
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
    B2INFO("DHHC FRAME TYP " << hex << getFrameType() << " -> " << dhhc_type_name[getFrameType()] << " ERR " << getErrorFlag() << " data " << data);
  };
};

struct dhhc_start_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trigger_nr_lo;
  const unsigned short trigger_nr_hi;
  const unsigned short time_tag_lo_and_type;
  const unsigned short time_tag_mid;
  const unsigned short time_tag_hi;
  const unsigned short nr_frames_in_event;
  const unsigned short crc32lo;/// Changed to 2*16 because of automatic compiler alignment
  const unsigned short crc32hi;
  /// fixed length, only for reading

  unsigned short getEventNrLo(void) const {
    return trigger_nr_lo;
  };
  bool isFakedData(void) const {
    unsigned int crc32 = (crc32hi << 16) | crc32lo;
    if (word0.data != 0x5800) return false;
    if (trigger_nr_lo != 0) return false;
    if (trigger_nr_hi != 0) return false;
    if (time_tag_lo_and_type != 0) return false;
    if (time_tag_mid != 0) return false;
    if (time_tag_hi != 0) return false;
    if (nr_frames_in_event != 0) return false;
    if (crc32 != 0x2DA167EF) return false;
    return true;
  };
  inline unsigned int getFixedSize(void) const {
    return 18;// bytes
  };
  void print(void) const {
    word0.print();
    unsigned int crc32 = (crc32hi << 16) | crc32lo;
    B2INFO("DHHC Start Frame TNRLO $" << hex << trigger_nr_lo << " TNRHI $" << hex << trigger_nr_hi << " TTLO $" << hex << time_tag_lo_and_type
           << " TTMID $" << hex << time_tag_mid << " TTHI $" << hex << time_tag_hi << " Frames in Event " << dec << nr_frames_in_event
           << " CRC $" << hex << crc32);
  };
  inline unsigned int get_active_dhh_mask(void) const {return word0.getMisc() & 0x1F;};
  inline unsigned int get_dhhc_id(void) const {return (word0.getMisc() >> 5) & 0xF;};
  inline unsigned int get_dhhc_nr_frames(void) const {return nr_frames_in_event;};
};

struct dhhc_dhh_start_frame {
  const dhhc_frame_header_word0 word0;
  const unsigned short trigger_nr_lo;
  const unsigned short dhh_time_tag_lo;
  const unsigned short dhh_time_tag_hi;
  const unsigned short sfnr_offset;
  const unsigned short crc32lo;/// Changed to 2*16bit because of automatic compiler alignment
  const unsigned short crc32hi;
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
    return 14;// 7 words
  };
  void print(void) const {
    word0.print();
    unsigned int crc32 = (crc32hi << 16) | crc32lo;
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
    return (2 + 96 / 2) * 4;
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
    unsigned int error_mask = 0;
    if ((magic1 & 0xFFFF) != 0xCAFE) {
      B2ERROR("DHHC HLT/ROI Magic 1 error $" << hex << magic1);
      error_mask |= ONSEN_ERR_FLAG_HLTROI_MAGIC;
    }
    if ((magic2 & 0xFFFF) != 0xCAFE) {
      B2ERROR("DHHC HLT/ROI Magic 2 error $" << hex << magic2);
      error_mask |= ONSEN_ERR_FLAG_HLTROI_MAGIC;
    }
    if (magic2 == 0x0000CAFE && trignr2 == 0x00000000) {
      if (!ignore_datcon_flag) B2WARNING("DHHC HLT/ROI Frame: No DATCON data " << hex << trignr1 << "!=$" << trignr2);
      error_mask |= ONSEN_ERR_FLAG_NO_DATCON;
    } else {
      if (trignr1 != trignr2) {
        B2ERROR("DHHC HLT/ROI Frame Trigger Nr Mismatch $" << hex << trignr1 << "!=$" << trignr2);
        error_mask |= ONSEN_ERR_FLAG_MERGER_TRIGNR;
      }
    }
    return error_mask;
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
    c = htonl(bocrc.checksum());

    unsigned int crc32;
    crc32 = *(unsigned int*)(crcbuffer + length - 8); /// -4

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
  bool pad;

  dhhc_frames(void) {
    data = 0;
    datasize = 0;
    type = -1;
    length = 0;
    pad = false;
  };
  int getFrameType(void) {
    return type;
  };
  void set(void* d, unsigned int t) {
    data = d;
    type = t;
    length = 0;
    pad = false;
  };
  void set(void* d, unsigned int t, unsigned int l, bool p) {
    data = d;
    type = t;
    length = l;
    pad = p;
  };
  void set(void* d) {
    data = d;
    type = ((dhhc_frame_header_word0*)data)->getFrameType();
    length = 0;
    pad = false;
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
        crcbuffer[k] = d[k + 1];
        crcbuffer[k + 1] = d[k];
      }
      bocrc.process_bytes(crcbuffer, length - 4);
    }
    unsigned int c;
    c = htonl(bocrc.checksum());

    unsigned int crc32;
    if (pad) {
      crc32 = (*(unsigned short*)(d + length - 4))  | ((*(unsigned short*)(d + length - 2)) << 16);
    } else {
      crc32 = *(unsigned int*)(d + length - 4);
    }

    if (c == crc32) {
//       if (verbose)
      //         B2INFO("DHH Data Frame CRC: " << hex << c << "==" << crc32);
//         B2INFO("DHHC Data Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
//                << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " pad " << pad << " len $" << length);
    } else {
//       crc_error++;
//       if (verbose) {
      B2ERROR("DHHC Data Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
              << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " pad " << pad << " len $" << length);
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

  void write_pedestal(void) {
    B2INFO("Write Pedestal Data - not implemented... !");
  };

};

///******************************************************************
///*********************** Main unpacker code ***********************
///******************************************************************

PXDUnpackerModule::PXDUnpackerModule() :
  Module(),
  m_storeRawHits(),
  m_storeROIs()
{
  //Set module properties
  setDescription("Unpack Raw PXD Hits from ONSEN data stream");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("HeaderEndianSwap", m_headerEndianSwap, "Swap the endianess of the ONSEN header", true);
  addParam("IgnoreDATCON", m_ignoreDATCON, "Ignore missing DATCON ROIs", false);
  addParam("IgnoreDHHCFrameNr", m_ignore_headernrframes, "Ignore Wrong Nr Frames in DHHC Start", false);
  addParam("IgnoreDHPMask", m_ignore_dhpmask, "Ignore missing DHP from DHH Start mask", false);
  addParam("IgnoreDHPPortDiffer", m_ignore_dhpportdiffer, "Ignore if DHP port differ in DHH and DHP header", false);
  addParam("DoNotStore", m_doNotStore, "only unpack and check, but do not store", false);

}

void PXDUnpackerModule::initialize()
{
  B2ERROR("For unpacking data from Desy test 2013/14, please move to PXDUnpackerDesy1314Module !!!");
  B2ERROR("PXDUnpackerModule will only unpack data recorded with latest DHHC and ONSEN firmware.");
//   exit(0);

  //Register output collections
  m_storeRawHits.registerAsPersistent();
  m_storeROIs.registerAsPersistent();
  /// actually, later we do not want to store it into output file ...  aside from debugging
  B2INFO("HeaderEndianSwap: " << m_headerEndianSwap);
  B2INFO("Ignore(missing)DATCON: " << m_ignoreDATCON);

  ignore_datcon_flag = m_ignoreDATCON;

  unpacked_events = 0;
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) error_counter[i] = 0;
}

void PXDUnpackerModule::terminate()
{
  const string error_name[ONSEN_MAX_TYPE_ERR] = {
    "FTSW/DHHC mismatch", "DHHC/DHH mismatch", "DHHC/DHP mismatch", "DHHC_START missing",
    "DHHC_END missing", "DHH_START missing", "DHHC Framecount mismatch", "DATA outside of DHH",
    "Second DHHC_START", "Second DHHC_END", "Fixed size frame wrong size", "DHH CRC Error:",
    "Unknown DHHC type", "Merger CRC Error", "Event Header Full Packet Size Error", "Event Header Magic Error",
    "Event Header Frame Count Error", "Event header Frame Size Error", "HLTROI Magic Error", "Merger HLT/DATCON TrigNr Mismatch",
    "DHP Size too small", "DHP-DHH DHHID mismatch", "DHP-DHH Port mismatch", "DHP Pix w/o row",
    "DHH START/END ID mismatch", "DHH ID mismatch of START and this frame", "DHH_START w/o prev END", "Nr PXD data !=1",
    "Missing Datcon", "NO DHHC data for Trigger", "DHH active mismatch", "DHP active mismatch"
  };

  int flag = 0;
  string errstr = "Statistic ( ;";
  errstr += to_string(unpacked_events) + ";";
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) { errstr += to_string(error_counter[i]) + ";"; flag |= error_counter[i];}
  if (flag != 0) {
    B2ERROR("PXD Unpacker --> Error Statistics (counted once per event!) in Events: " << unpacked_events);
    B2ERROR(errstr + " )");
    for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      if (error_counter[i]) {
        B2ERROR(error_name[i] << ": " << error_counter[i]);
        //printf("%s %d\n",error_name[i].c_str(), error_counter[i]);
      }
    }
  } else {
    B2INFO("PXD Unpacker --> No Error found in Events: " << unpacked_events);
  }
}

void PXDUnpackerModule::event()
{
  StoreArray<RawPXD> storeRaws;
  StoreArray<RawFTSW> storeFTSW;

  int nRaws = storeRaws.getEntries();
  if (verbose) {
    B2INFO("PXD Unpacker --> RawPXD Objects in event: " << nRaws);
  };

  error_mask = 0;
  ftsw_evt_nr = 0;
  ftsw_evt_mask = 0;
  for (auto & it : storeFTSW) {
    ftsw_evt_nr = it.GetEveNo(0);
    ftsw_evt_mask = 0x7FFF;
    B2INFO("PXD Unpacker --> FTSW Event Number: $" << hex << ftsw_evt_nr);
    break;
  }

  int nsr = 0;
  for (auto & it : storeRaws) {
    if (verbose) {
      B2INFO("PXD Unpacker --> Unpack Objects: ");
    };
    unpack_event(it);
    nsr++;
  }

  if (nsr != 1) error_mask |= ONSEN_ERR_FLAG_NO_PXD;

  unpacked_events++;
  for (unsigned int i = 0, j = 1; i < ONSEN_MAX_TYPE_ERR; i++) {
    if (error_mask & j) error_counter[i]++;
    j += j;
  }
}

void PXDUnpackerModule::endian_swap_frame(unsigned short* dataptr, int len)
{
  /// swap endianess of all shorts in frame BUT not the CRC (2 shorts)
  for (int i = 0; i < len / 2 - 2; i++) {
    dataptr[i] = htons(dataptr[i]);
  }
}

void PXDUnpackerModule::unpack_event(RawPXD& px)
{
  int last_wie = 0;
  static unsigned int last_evtnr = 0;
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD Unpacker --> invalid packet size (32bit words) " << hex << px.size());
    error_mask |= ONSEN_ERR_FLAG_PACKET_SIZE;
    return;
  }
  unsigned int data[px.size()];
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  memcpy(data, (unsigned int*)px.data(), fullsize);

  if (fullsize < 8) {
    B2ERROR("Data is to small to hold a valid Header! Will not unpack anything. Size:" << fullsize);
    error_mask |= ONSEN_ERR_FLAG_PACKET_SIZE;
    return;
  }

  if (data[0] != 0xCAFEBABE && data[0] != 0xBEBAFECA) {
    B2ERROR("Magic invalid: Will not unpack anything. Header corrupted! " << hex << data[0]);
    error_mask |= ONSEN_ERR_FLAG_MAGIC;
    return;
  }

  if (m_headerEndianSwap) Frames_in_event = ntohl(data[1]); else Frames_in_event = data[1];
  if (Frames_in_event < 0 || Frames_in_event > 256) {
    B2ERROR("Number of Frames invalid: Will not unpack anything. Header corrupted! Frames in event: " << Frames_in_event);
    error_mask |= ONSEN_ERR_FLAG_FRAME_NR;
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
    bool pad;
    if (m_headerEndianSwap) lo = ntohl(tableptr[j]); else lo = tableptr[j];
    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      error_mask |= ONSEN_ERR_FLAG_FRAME_SIZE;
      return;
    }
    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " << datafullsize << " of fullsize " << fullsize);
      error_mask |= ONSEN_ERR_FLAG_FRAME_SIZE;
      return;
    }
    if (lo & 0x3) {
      pad = true;
      if (verbose)
        B2INFO("Data with not MOD 4 length " << " ( " << lo << " ) ");
    } else {
      pad = false;
    }

    B2INFO("unpack DHH(C) frame: " << j << " with size " << lo << " at byte offset in dataptr " << ll);
    endian_swap_frame((unsigned short*)(ll + (char*)dataptr), lo);
    unpack_dhhc_frame(ll + (char*)dataptr, lo, pad, last_wie, last_evtnr, j, Frames_in_event);
    ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
  }

}

void PXDUnpackerModule::unpack_dhp(void* data, unsigned int len2, unsigned int dhh_first_readout_frame_id_lo, unsigned int dhh_ID, unsigned dhh_DHPport, unsigned dhh_reformat, unsigned short toffset, VxdID vxd_id)
{
  unsigned int anzahl = len2 / 2; // len2 in bytes!!!
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

  if (anzahl < 4) {
    B2ERROR("DHP frame size error (too small) " << anzahl);
    error_mask |= ONSEN_ERR_FLAG_DHP_SIZE;
//     dhp_size_error++;
    return;
    //return -1;
  }

  if (printflag)
    B2INFO("HEADER --  " << hex << dhp_pix[0] << hex << dhp_pix[1] << hex << dhp_pix[2] << hex << dhp_pix[3] << " -- ");

  if (printflag)
    B2INFO("DHP Header     |   " << hex << dhp_pix[2] << " ( " << hex << dhp_pix[2] << " ) ");
  dhp_header_type  = (dhp_pix[2] >> 13) & 0x07;
  dhp_reserved     = (dhp_pix[2] >> 8) & 0x1F;
  dhp_dhh_id       = (dhp_pix[2] >> 2) & 0x3F;
  dhp_dhp_id       =  dhp_pix[2] & 0x03;

  if (printflag) {
    B2INFO("DHP type          |    " << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
    B2INFO("DHP reserved          |    " << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
    B2INFO("DHP DHH ID          |    " << hex << dhp_dhh_id << " ( " << dec << dhp_dhh_id << " ) ");
    B2INFO("DHP DHP ID          |    " << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  }

  if (dhh_ID != dhp_dhh_id) {
    B2ERROR("DHH ID in DHH and DHP header differ ($" << hex << dhh_ID << " != $" << dhp_dhh_id);
    error_mask |= ONSEN_ERR_FLAG_DHH_DHP_DHHID;
  }
  if (dhh_DHPport != dhp_dhp_id && !m_ignore_dhpportdiffer) {
    B2ERROR("DHP ID (Chip/Port) in DHH and DHP header differ ($" << hex << dhh_DHPport << " != $" << dhp_dhp_id);
    error_mask |= ONSEN_ERR_FLAG_DHH_DHP_PORT;
  }

  static int offtab[4] = {0, 64, 128, 192};
  dhp_offset = offtab[dhp_dhp_id];

  dhp_readout_frame_lo  = dhp_pix[3] & 0xFFFF;
  if (printflag)
    B2INFO("DHP Frame Nr     |   " << hex << dhp_readout_frame_lo << " ( " << hex << dhp_readout_frame_lo << " ) ");

  for (unsigned int i = 4; i < anzahl ; i++) {

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
          error_mask |= ONSEN_ERR_FLAG_DHP_PIX_WO_ROW;
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
    /*for (int i = 0; i < raw_anzahl ; i++) {
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

void PXDUnpackerModule::unpack_dhhc_frame(void* data, int len, bool pad, int& last_wie, unsigned int& last_evtnr, int Frame_Number, int Frames_in_event)
{
  static int nr_of_dhh_start_frame = 0; /// could put it as a class member, but is only needed within this function
  static int nr_of_dhh_end_frame = 0; /// could put it as a class member, but is only needed within this function
  static int nr_of_frames_dhhc = 0;
  static int nr_of_frames_counted = 0;
  static int nr_active_dhh = 0;
  static int mask_active_dhh = 0;
  ///  static int nr_active_dhp = 0;// unused
  static int mask_active_dhp = 0;
  static int found_mask_active_dhp = 0;
  static unsigned int dhh_first_readout_frame_id_lo = 0;
  static unsigned int dhh_first_offset = 0;
  static unsigned int current_dhh_id = 0xFFFFFFFF;
  static unsigned int currentVxdId = 0;
  static bool isFakedData_event = false;


  dhhc_frame_header_word0* hw = (dhhc_frame_header_word0*)data;
//   error_flag = false;

  dhhc_frames dhhc;
  dhhc.set(data, hw->getFrameType(), len, pad);
  int s;
  s = dhhc.getFixedSize();
  if (len != s && s != 0) {
    B2ERROR("Fixed frame type size does not match specs: expect " << len << " != " << s << " (in data) " << pad);
    error_mask |= ONSEN_ERR_FLAG_FIX_SIZE;
  }

  unsigned int evtnr = dhhc.getEventNrLo();
  int type = dhhc.getFrameType();

  if (Frame_Number == 0) { /// We reset the counters on the first event
    nr_of_dhh_start_frame = 0;
    nr_of_dhh_end_frame = 0;
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START) {
      isFakedData_event = dhhc.data_dhhc_start_frame->isFakedData();
    } else {
      isFakedData_event = false;
    }
  }


  if ((evtnr & ftsw_evt_mask) != (ftsw_evt_nr & ftsw_evt_mask)) {
    if (isFakedData_event) {
      /// If ist a start, check if its a fake event, if its not a start, the fake flag is already (re)set. As long as the data structure is not messed up completly.
      /// no need to explicitly check the DHHC_END or HLT/ROI
      /// We have afake and shoudl set the trigger nr by hand to prevent further errors
      evtnr = ftsw_evt_nr & ftsw_evt_mask; // masking might be a problem as we cannore recover all bits
    } else {
      B2ERROR("Event Numbers do not match for this frame $" << hex << evtnr << "!=$" << ftsw_evt_nr << "(FTSW) mask $" << ftsw_evt_mask);
      error_mask |= ONSEN_ERR_FLAG_FTSW_DHHC_MM;
    }
  }

  if (Frame_Number > 0 && Frame_Number < Frames_in_event - 1) {
    if (nr_of_dhh_start_frame != nr_of_dhh_end_frame + 1)
      if (type != DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI && type != DHHC_FRAME_HEADER_DATA_TYPE_DHH_START) {
        B2ERROR("Data Frame outside a DHH START/END");
        error_mask |= ONSEN_ERR_FLAG_DATA_OUTSIDE;
      }
  }

  switch (type) {
    case DHHC_FRAME_HEADER_DATA_TYPE_DHP_RAW: {
      nr_of_frames_counted++;

      if (verbose)dhhc.data_direct_readout_frame_raw->print();
      if (current_dhh_id != dhhc.data_direct_readout_frame_raw->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << dhhc.data_direct_readout_frame_raw->getDHHId());
        error_mask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      error_mask |= dhhc.check_crc();
      found_mask_active_dhp |= 1 << dhhc.data_direct_readout_frame->getDHPPort();

//       stat_raw++;
      dhhc.write_pedestal();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP:
    case DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD: {
      nr_of_frames_counted++;

      if (verbose)dhhc.data_direct_readout_frame->print();

      //error_mask |= dhhc.data_direct_readout_frame->check_error();

      if (current_dhh_id != dhhc.data_direct_readout_frame_raw->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << dhhc.data_direct_readout_frame_raw->getDHHId());
        error_mask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      error_mask |= dhhc.check_crc();
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
      if (current_dhh_id != dhhc.data_direct_readout_frame_raw->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << dhhc.data_direct_readout_frame_raw->getDHHId());
        error_mask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      error_mask |= dhhc.check_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_COMMODE: {
      nr_of_frames_counted++;

      if (verbose) hw->print();
      if (current_dhh_id != dhhc.data_commode_frame->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << dhhc.data_commode_frame->getDHHId());
        error_mask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      error_mask |= dhhc.check_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START: {
      if (isFakedData_event != dhhc.data_dhhc_start_frame->isFakedData()) {
        B2ERROR("DHHC START is but no Fake event OR Fake Event but DHH END is not.");
      }
      if (dhhc.data_dhhc_start_frame->isFakedData()) {
        B2WARNING("Faked DHHC START Data -> trigger without Data!");
        error_mask |= ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhhc.data_dhhc_start_frame->print();
      }

      last_evtnr = evtnr;
      current_dhh_id = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      nr_of_frames_dhhc = dhhc.data_dhhc_start_frame->get_dhhc_nr_frames();
      nr_of_frames_counted = 1;
      error_mask |= dhhc.check_crc();
//       stat_start++;

      mask_active_dhh = dhhc.data_dhhc_start_frame->get_active_dhh_mask();
      nr_active_dhh = nr5bits(mask_active_dhh);
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHH_START: {
      nr_of_frames_counted++;
      if (verbose)dhhc.data_dhh_start_frame->print();
      dhh_first_readout_frame_id_lo = dhhc.data_dhh_start_frame->getStartFrameNr();
      dhh_first_offset = dhhc.data_dhh_start_frame->getTriggerOffsetRow();
      current_dhh_id = dhhc.data_dhh_start_frame->getDHHId();
      error_mask |= dhhc.check_crc();

      if (nr_of_dhh_start_frame != nr_of_dhh_end_frame) {
        B2ERROR("DHHC_FRAME_HEADER_DATA_TYPE_DHH_START without DHHC_FRAME_HEADER_DATA_TYPE_DHH_END");
        error_mask |= ONSEN_ERR_FLAG_DHH_START_WO_END;
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
        sensor = (current_dhh_id & 0x1) + 1;
        ladder = (current_dhh_id & 0x1E) >> 1; // no +1
        layer = ((current_dhh_id & 0x20) >> 5) + 1;
        currentVxdId = VxdID(layer, ladder, sensor);
      }
      //currentVxdId = current_dhh_id;

      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_GHOST:
      nr_of_frames_counted++;
      if (verbose)dhhc.data_ghost_frame->print();
      if (current_dhh_id != dhhc.data_ghost_frame->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << dhhc.data_ghost_frame->getDHHId());
        error_mask |= ONSEN_ERR_FLAG_DHH_START_ID;
      }
      /// Attention: Firmware might be changed such, that ghostframe come for all DHPs, not only active ones...
      found_mask_active_dhp |= 1 << dhhc.data_ghost_frame->getDHPPort();
      error_mask |= dhhc.check_crc();
//       stat_ghost++;

      break;
    case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END: {
      if (dhhc.data_dhhc_end_frame->isFakedData() != isFakedData_event) {
        B2ERROR("DHHC END is but no Fake event OR Fake Event but DHH END is not.");
      }
      nr_of_frames_counted++;
      current_dhh_id = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      if (isFakedData_event) {
        B2WARNING("Faked DHHC END Data -> trigger without Data!");
        error_mask |= ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG;
      } else {
        if (verbose)dhhc.data_dhhc_end_frame->print();
      }
//       stat_end++;

      if (!isFakedData_event) {
        if (nr_of_frames_counted != nr_of_frames_dhhc) {
          if (!m_ignore_headernrframes) B2ERROR("Number of DHHC Frames in Header " << nr_of_frames_dhhc << " != " << nr_of_frames_counted << " Counted");
          error_mask |= ONSEN_ERR_FLAG_DHHC_FRAMECOUNT;
        }
      }
      if (!isFakedData_event) {
        int w;
        w = dhhc.data_dhhc_end_frame->get_words() * 2;
        last_wie += 2;
        if (verbose) {
          B2INFO("last_wie " << last_wie << " w " << w);
        };
        if (last_wie != w) {
          if (verbose) {
            B2INFO("Error: WIE " << hex << last_wie << " vs END " << hex << w << " pad " << pad);
          };
//           error_flag = true;
//           wie_error++;
        } else {
          if (verbose)
            B2INFO("EVT END: WIE " << hex << last_wie << " == END " << hex << w << " pad " << pad);
        }
      }
      error_mask |= dhhc.check_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHH_END: {
      nr_of_frames_counted++;
      if (verbose) dhhc.data_dhh_end_frame->print();
      if (current_dhh_id != dhhc.data_dhh_end_frame->getDHHId()) {
        B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << dhhc.data_dhh_end_frame->getDHHId());
        error_mask |= ONSEN_ERR_FLAG_DHH_START_END_ID;
      }
      current_dhh_id = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      error_mask |= dhhc.check_crc();
      if (found_mask_active_dhp != mask_active_dhp) {
        if (!m_ignore_dhpmask) B2ERROR("DHH_END: DHP active mask $" << hex << mask_active_dhp << " != $" << hex << found_mask_active_dhp << " mask of found dhp/ghost frames");
        error_mask |= ONSEN_ERR_FLAG_DHP_ACTIVE;
      }
      nr_of_dhh_end_frame++;
      if (nr_of_dhh_start_frame != nr_of_dhh_end_frame) {
        B2ERROR("DHHC_FRAME_HEADER_DATA_TYPE_DHH_END without DHHC_FRAME_HEADER_DATA_TYPE_DHH_START");
        error_mask |= ONSEN_ERR_FLAG_DHH_START;
      }
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI:
      //nr_of_frames_counted++;/// DO NOT COUNT!!!!
      //dhhc.data_onsen_roi_frame->set_length(len - 4);
      if (verbose) dhhc.data_onsen_roi_frame->print();
      error_mask |= dhhc.data_onsen_roi_frame->check_error(ignore_datcon_flag);
      error_mask |= dhhc.data_onsen_roi_frame->check_inner_crc(len - 4); /// CRC is without the DHHC header
      error_mask |= dhhc.check_crc();
      if (!m_doNotStore)dhhc.data_onsen_roi_frame->save(m_storeROIs, len, (unsigned int*) data);
      break;
    case DHHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG:
      if (verbose) dhhc.data_onsen_trigger_frame->print();
      error_mask |= dhhc.data_onsen_trigger_frame->check_error();
      error_mask |= dhhc.check_crc();
      break;
    default:
      B2ERROR("UNKNOWN DHHC frame type");
      error_mask |= ONSEN_ERR_FLAG_DHHC_UNKNOWN;
//      type_error++;
      if (verbose) hw->print();
//       error_flag = true;
      break;
  }

  if (evtnr != last_evtnr) {
    B2ERROR("Frame TrigNr != DHHC Trig Nr $" << hex << evtnr << " != $" << last_evtnr);
    error_mask |= ONSEN_ERR_FLAG_DHHC_DHH_MM;
//     evtnr_error++;
//     error_flag = true;
  }

  if (Frame_Number == 0) {
    /// Check that DHHC Start is first Frame
    if (type != DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START) {
      B2ERROR("First frame is not a DHHC start of subevent frame in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHHC_START;
    }
  } else { // (Frame_Number != 0 &&
    /// Check that there is no other DHHC Start
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START) {
      B2ERROR("More than one DHHC start of subevent frame in frame in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHHC_START2;
    }
  }

  if (Frame_Number == Frames_in_event - 1) {
    /// Check that DHHC End is last Frame
    if (type != DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END) {
      B2ERROR("Last frame is not a DHHC end of subevent frame in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHHC_END;
    }

    /// As we now have processed the whole event, we can do some more consistency checks!
    if (nr_of_dhh_start_frame != nr_of_dhh_end_frame || nr_of_dhh_start_frame != nr_active_dhh) {
      B2ERROR("The number of DHH Start/End does not match the number of active DHH in DHHC Header! Header: " << nr_active_dhh << " Start: " << nr_of_dhh_start_frame << " End: " << nr_of_dhh_end_frame << " Mask: $" << hex << mask_active_dhh << " in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHH_ACTIVE;
    }

  } else { //  (Frame_Number != Frames_in_event - 1 &&
    /// Check that there is no other DHHC End
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END) {
      B2ERROR("More than one DHHC end of subevent frame in frame in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHHC_END2;
    }
  }

  /// Check that (if there is at least one active DHH) the second Frame is DHH Start, actually this is redundant if the other checks work
  if (Frame_Number == 1 && nr_active_dhh != 0 && type != DHHC_FRAME_HEADER_DATA_TYPE_DHH_START) {
    B2ERROR("Second frame is not a DHH start frame in Event Nr " << evtnr);
    error_mask |= ONSEN_ERR_FLAG_DHH_START;
  }

  last_wie += len;

}



