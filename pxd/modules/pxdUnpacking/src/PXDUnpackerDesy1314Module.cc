/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdUnpacking/PXDUnpackerDesy1314Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <rawdata/dataobjects/RawFTSW.h>

// // for htonl
#include <arpa/inet.h>

#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

#define DHP_FRAME_HEADER_DATA_TYPE_RAW  0x0
#define DHP_FRAME_HEADER_DATA_TYPE_ZSD  0x5

// DHC envelope
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_START  0xB
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_END    0xC
// Onsen processed data
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_ONS     0xD
#define DHC_FRAME_HEADER_DATA_TYPE_FCE_ONS     0x9
#define DHC_FRAME_HEADER_DATA_TYPE_HLTROI      0xF

// DHE like above, but format has changed
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW     0x0
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD     0x5
#define DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW     0x1
#define DHC_FRAME_HEADER_DATA_TYPE_COMMODE     0x6
#define DHC_FRAME_HEADER_DATA_TYPE_GHOST       0x2
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_START   0x3
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_END     0x4

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDUnpackerDesy1314)

unsigned int error_mask = 0;

#define ONSEN_ERR_FLAG_FTSW_DHC_MM 0x00000001ul
#define ONSEN_ERR_FLAG_DHC_DHE_MM  0x00000002ul
//#define ONSEN_ERR_FLAG_DHC_DHP_MM  0x00000004ul // unsused
#define ONSEN_ERR_FLAG_DHC_START 0x00000008ul
#define ONSEN_ERR_FLAG_DHC_END   0x00000010ul
#define ONSEN_ERR_FLAG_DHE_START  0x00000020ul
#define ONSEN_ERR_FLAG_DHC_FRAMECOUNT 0x00000040ul
#define ONSEN_ERR_FLAG_DATA_OUTSIDE 0x00000080ul
#define ONSEN_ERR_FLAG_DHC_START2  0x00000100ul
#define ONSEN_ERR_FLAG_DHC_END2  0x00000200ul
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

string error_name[ONSEN_MAX_TYPE_ERR] = {
  "FTSW/DHC mismatch", "DHC/DHE mismatch", "DHC/DHP mismatch", "DHC_START missing",
  "DHC_END missing", "DHE_START missing", "DHC Framecount mismatch", "DATA outside of DHE",
  "Second DHC_START", "Second DHC_END", "Fixed size frame wrong size", "DHE CRC Error:",
  "Unknown DHC type", "Merger CRC Error", "Event Header Full Packet Size Error", "Event Header Magic Error",
  "Event Header Frame Count Error", "Event header Frame Size Error", "HLTROI Magic Error", "Merger HLT/DATCON TrigNr Mismatch",
  "DHP Size too small", "DHP-DHE DHEID mismatch", "DHP-DHE Port mismatch", "DHP Pix w/o row",
  "DHE START/END ID mismatch", "DHE ID mismatch of START and this frame", "DHE_START w/o prev END", "Nr PXD data !=1",
  "Missing Datcon", "NO DHC data for Trigger", "DHE active mismatch", "DHP active mismatch"
};
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhe_crc_32_type;


unsigned int type_error = 0, crc_error = 0, magic_error = 0, zerodata_error = 0;
unsigned int queue_error = 0, fnr_error = 0, wie_error = 0, end_error = 0, evt_skip_error = 0, start_error = 0, evtnr_error = 0;
// unsigned long long evt_counter = 0;
unsigned int stat_start = 0, stat_end = 0, stat_ghost = 0, stat_raw = 0, stat_zsd = 0;
unsigned int dhp_size_error = 0, dhp_pixel_error = 0, dhp_warning = 0;
bool verbose = true;
bool error_flag = false;
bool ignore_datcon_flag = true;

///*********************************************************************************
///***************************** DHC Code starts here *****************************
///*********************************************************************************

char* dhc_type_name[16] = {
  (char*)"DHP_RAW",
  (char*)"FCE_RAW",
  (char*)"GHOST  ",
  (char*)"H_START",
  (char*)"H_END  ",
  (char*)"DHP_ZSD",
  (char*)"COMMODE",
  (char*)"undef  ",
  (char*)"undef  ",
  (char*)"FCE_ONS",
  (char*)"undef  ",
  (char*)"C_START",
  (char*)"C_END  ",
  (char*)"DHP_ONS",
  (char*)"undef  ",
  (char*)"HLTROI "
};


class dhc_frame_header_word0 {
public:
  unsigned short data;
  /// fixed length
  unsigned int get_type(void)
  {
    return (data >> 11) & 0xF;
  };
  unsigned int get_err(void)
  {
    return (data >> 15) & 0x1;
  };
  unsigned int get_misc(void)
  {
    return data & 0x3FF;
  };
  void print(void)
  {
    if (verbose)
      B2INFO("DHC FRAME TYP " << hex << get_type() << " -> " << dhc_type_name[get_type()] << " ERR " << get_err() << " data " << data);
  };
};

class dhc_start_frame {
public:
  dhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned short trigger_nr_hi;
  unsigned short time_tag_lo_and_type;
  unsigned short time_tag_mid;
  unsigned short time_tag_hi;
  unsigned short nr_frames_in_event;
  unsigned short crc32lo;/// Changed to 2*16 because of automatic compiler alignment
  unsigned short crc32hi;
  /// fixed length

  unsigned short get_evtnr_lo(void)
  {
    return trigger_nr_lo;
  };
  unsigned int calc_crc(void)
  {
    unsigned char* d;
    dhe_crc_32_type bocrc;
    char crcbuffer[size()];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 4); k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 4));
    unsigned int c;
    c = htonl(bocrc.checksum());
    unsigned int crc32 = (crc32hi << 16) | crc32lo;
    if (c == crc32) {
      if (verbose)
        B2INFO("DHC Start Frame CRC " << hex << c << " == " << hex << crc32);
    } else {
      crc_error++;
      B2ERROR("DHC Start Frame CRC FAIL " << hex << c << " != " << hex << crc32);
      error_flag = true;
    }
    return c;
  };
  bool is_fake(void)
  {
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
  inline static unsigned int size(void)
  {
    return 18;// bytes
  };
  void print(void)
  {
    word0.print();
    if (verbose) {
      unsigned int crc32 = (crc32hi << 16) | crc32lo;
      B2INFO("DHC Start Frame TNRLO $" << hex << trigger_nr_lo << " TNRHI $" << hex << trigger_nr_hi << " TTLO $" << hex <<
             time_tag_lo_and_type
             << " TTMID $" << hex << time_tag_mid << " TTHI $" << hex << time_tag_hi << " Frames in Event " << dec << nr_frames_in_event
             << " CRC $" << hex << crc32 << " (calc) $" << calc_crc());
    }
  };
  inline unsigned int get_active_dhe_mask(void) {return word0.get_misc() & 0x1F;};
  inline unsigned int get_dhc_id(void) {return (word0.get_misc() >> 5) & 0xF;};
  inline unsigned int get_dhc_nr_frames(void) {return nr_frames_in_event;};
};

class dhc_dhe_start_frame {
public:
  dhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned short dhe_time_tag_lo;
  unsigned short dhe_time_tag_hi;
  unsigned short sfnr_offset;
  unsigned short crc32lo;/// Changed to 2*16 because of automatic compiler alignment
  unsigned short crc32hi;
  /// fixed length

  inline unsigned short get_evtnr_lo(void)
  {
    return trigger_nr_lo;
  };
  inline unsigned short get_sfnr(void)  // last DHP fraem before trigger
  {
    return (sfnr_offset >> 10) & 0x3F;
  };
  inline unsigned short get_toffset(void)  // and trigger row offset
  {
    return sfnr_offset & 0x3FF;
  };
  unsigned int calc_crc(void)
  {
    unsigned char* d;
    dhe_crc_32_type bocrc;
    char crcbuffer[size()];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 4); k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 4));
    unsigned int c;
    c = htonl(bocrc.checksum());
    unsigned int crc32 = (crc32hi << 16) | crc32lo;
    if (c == crc32) {
      if (verbose)
        B2INFO("DHC DHE Start Frame CRC " << hex << c << " == " << hex << crc32);
    } else {
      crc_error++;
      B2ERROR("DHC DHE Start CRC FAIL " << hex << c << " != " << hex << crc32);
      error_flag = true;
    }
    return c;
  };
  inline static unsigned int size(void)
  {
    return 14;// 7 words
  };
  void print(void)
  {
    word0.print();
    if (verbose) {
      unsigned int crc32 = (crc32hi << 16) | crc32lo;
      B2INFO("DHC Event Frame TNRLO $" << hex << trigger_nr_lo  << " DTTLO $" << hex << dhe_time_tag_lo << " DTTHI $" << hex <<
             dhe_time_tag_hi
             << " DHEID $" << hex << get_dhe_id()
             << " DHPMASK $" << hex << get_active_dhp_mask()
             << " SFNR $" << hex << get_sfnr()
             << " OFF $" << hex << get_toffset()
             << " CRC " << hex << crc32 << " (calc)" << calc_crc());
    }
  };
  inline unsigned int get_active_dhp_mask(void) {return word0.get_misc() & 0xF;};
  inline unsigned int get_dhe_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
};

class dhc_commode_frame {
public:
  dhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned short data[96];
  unsigned int crc32;
  /// fixed length

  inline unsigned short get_evtnr_lo(void)
  {
    return trigger_nr_lo;
  };
  inline static unsigned int size(void)
  {
    return (2 + 96 / 2) * 4;
  };
  inline unsigned int get_dhe_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
};

class dhc_direct_readout_frame {
public:
  dhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  /// an unbelievable amount of words may follow
  /// and finally a 32 bit checksum

  inline static unsigned int size(void)
  {
    return 0;// size can vary
  };
  inline unsigned short get_evtnr_lo(void)
  {
    return trigger_nr_lo;
  };
  void print(void)
  {
    word0.print();
    if (verbose)
      B2INFO("DHC Direct Readout (Raw|ZSD|ONS) Frame TNRLO $" << hex << trigger_nr_lo << " DHE ID $" << get_dhe_id() << " DHP port $" <<
             get_dhp_port());
  };
  inline unsigned short get_dhe_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
  inline unsigned short get_dhp_port(void) {return (word0.get_misc()) & 0x3;};
  inline bool get_reformat_flag(void) {return (word0.get_misc() >> 3) & 0x1;};
};

class dhc_direct_readout_frame_raw : public dhc_direct_readout_frame {
public:
};

class dhc_direct_readout_frame_zsd : public dhc_direct_readout_frame {
public:
};

class dhc_onsen_frame {
  dhc_frame_header_word0 word0;/// mainly empty
  unsigned short trignr0;// not used
  unsigned int magic1;
  unsigned int trignr1;
  unsigned int magic2;
  unsigned int trignr2;
  /// plus n* ROIs (64 bit)
  /// plus checksum 32bit
public:
  inline unsigned short get_trig_nr0(void)
  {
    return trignr0;
  };
  inline unsigned short get_trig_nr1(void)
  {
    return trignr1;
  };
  inline unsigned short get_trig_nr2(void)
  {
    return trignr2;
  };
  void print(void)
  {
    word0.print();
    if (verbose)
      B2INFO("DHC HLT/ROI Frame " << hex << trignr1 << " ," << trignr2);
    if ((magic1 & 0xFFFF) != 0xCAFE) {
      B2ERROR("DHC HLT/ROI Magic 1 error $" << hex << magic1);
      error_mask |= ONSEN_ERR_FLAG_HLTROI_MAGIC;
    }
    if ((magic2 & 0xFFFF) != 0xCAFE) {
      B2ERROR("DHC HLT/ROI Magic 2 error $" << hex << magic2);
      error_mask |= ONSEN_ERR_FLAG_HLTROI_MAGIC;
    }
    if (magic2 == 0x0000CAFE && trignr2 == 0x00000000) {
      if (!ignore_datcon_flag) B2WARNING("DHC HLT/ROI Frame: No DATCON data " << hex << trignr1 << "!=$" << trignr2);
      error_mask |= ONSEN_ERR_FLAG_NO_DATCON;
    } else {
      if (trignr1 != trignr2) {
        B2ERROR("DHC HLT/ROI Frame Trigger Nr Mismatch $" << hex << trignr1 << "!=$" << trignr2);
        error_mask |= ONSEN_ERR_FLAG_MERGER_TRIGNR;
      }
    }
  };
//  inline static unsigned int size(void) {
//    return 0;// siez can vary
//  };

  unsigned int calc_crc(unsigned int length)
  {
    if (length < 8) {
      B2ERROR("DHC ONSEN HLT/ROI Frame too small!!!");
      error_mask |= ONSEN_ERR_FLAG_MERGER_CRC;
      return 0;
    }
    unsigned char* d;
    dhe_crc_32_type bocrc;
    char crcbuffer[65536 * 2]; /// 128kB
    d = (unsigned char*) &magic1;/// without the DHC header as its only an inner checksum!!!

    if (length > 65536 * 2) {
      B2WARNING("DHC ONSEN HLT/ROI Frame CRC FAIL bacause of too large packet (>128kB)!");
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
      if (verbose)
        B2INFO("DHC ONSEN HLT/ROI Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
               << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " len $" << length);
    } else {
      crc_error++;
      error_mask |= ONSEN_ERR_FLAG_MERGER_CRC;
      if (verbose) {
        B2ERROR("DHC ONSEN HLT/ROI Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
                << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " len $" << length);
        /// others would be interessting but possible subjects to access outside of buffer
        /// << " " << * (unsigned int*)(d + length - 2) << " " << * (unsigned int*)(d + length + 0) << " " << * (unsigned int*)(d + length + 2));
        //if (length <= 64) {
        //  for (unsigned int i = 0; i < length / 4; i++) {
        //    B2ERROR("== " << i << "  $" << hex << ((unsigned int*)d)[i]);
        //  }
        //}
      };
      error_flag = true;
    }
    return c;
  };
  void save(StoreArray<PXDRawROIs>& sa, unsigned int length, unsigned int* data)
  {
    if (length < 4 + 4 + 4 * 4) {
      B2ERROR("DHC ONSEN HLT/ROI Frame too small to hold any ROIs, did not save anything!");
      return;
    }
    unsigned int l;
    l = (length - 4 - 4 - 4 * 4) / 8;
    // for(unsigned int i=0; i<l*2; i++) data[5+i]=((data[5+i]>>16)&0xFFFF)| ((data[5+i]&0xFFFF)<<16);// dont do it here ... CRC will fail
    //sa.appendNew(l, magic1, trignr1, magic2, trignr2, &data[5]);
    sa.appendNew(l, &data[5]);// NEW format
  }

};

class dhc_ghost_frame {
public:
  dhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int crc32;
  /// fixed length

  unsigned int calc_crc(void)
  {
    unsigned char* d;
    dhe_crc_32_type bocrc;
    char crcbuffer[size()];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 4); k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 4));
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO("DHC Ghost Frame CRC " << hex << c << " == " << crc32);
    } else {
      crc_error++;
      B2ERROR("DHC Ghost Frame CRC FAIL " << hex << c << " != " << crc32);
      error_flag = true;
    }
    return c;
  };
  inline static unsigned int size(void)
  {
    return 8;
  };
  void print(void)
  {
    word0.print();
    if (verbose)
      B2INFO("DHC Ghost Frame TNRLO " << hex << trigger_nr_lo << " DHE ID $" << get_dhe_id() << " DHP port $" << get_dhp_port() <<
             " CRC $"  << crc32 << " (calc) "  << calc_crc());
  };
  inline unsigned short get_dhe_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
  inline unsigned short get_dhp_port(void) {return (word0.get_misc()) & 0x3;};
};

class dhc_end_frame {
public:
  dhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int wordsinevent;
  unsigned int errorinfo;
  unsigned int crc32;
  /// fixed length

  unsigned int calc_crc(void)
  {
    unsigned char* d;
    dhe_crc_32_type bocrc;
    char crcbuffer[size()];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 4); k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 4));
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO("DHC End Frame CRC " << hex << c << "==" << crc32);
    } else {
      crc_error++;
      B2ERROR("DHC End Frame CRC " << hex << c << "!=" << crc32);
      error_flag = true;
    }
    return c;
  };
  unsigned int get_words(void)
  {
    return wordsinevent;
  }
  inline static unsigned int size(void)
  {
    return 16;
  };
  bool is_fake(void)
  {
    if (word0.data != 0x6000) return false;
    if (trigger_nr_lo != 0) return false;
    if (wordsinevent != 0) return false;
    if (errorinfo != 0) return false;
    if (crc32 != 0xF7BCA507) return false;
    return true;
  };
  void print(void)
  {
    word0.print();
    if (verbose)
      B2INFO("DHC End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
             << " CRC " << hex << crc32 << " (calc) " << calc_crc());
  };
  inline unsigned int get_dhc_id(void) {return (word0.get_misc() >> 5) & 0xF;};
};

class dhc_dhe_end_frame {
public:
  dhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int wordsinevent;
  unsigned int errorinfo;
  unsigned int crc32;
  /// fixed length

  unsigned int calc_crc(void)
  {
    unsigned char* d;
    dhe_crc_32_type bocrc;
    char crcbuffer[size()];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 4); k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 4));
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO("DHC DHE End Frame CRC " << hex << c << "==" << crc32);
    } else {
      crc_error++;
      B2ERROR("DHC DHE End Frame CRC " << hex << c << "!=" << crc32);
      error_flag = true;
    }
    return c;
  };
  unsigned int get_words(void)
  {
    return wordsinevent;
  }
  inline static unsigned int size(void)
  {
    return 16;
  };
  void print(void)
  {
    word0.print();
    if (verbose)
      B2INFO("DHC DHE End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
             << " CRC " << hex << crc32 << " (calc) " << calc_crc());
  };
  inline unsigned int get_dhe_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
};

class dhc_frames {
  void* data;
  unsigned int datasize;
  int type;
  int length;
  bool pad;
public:
  dhc_frames(void)
  {
    data = 0;
    datasize = 0;
    type = -1;
    length = 0;
    pad = false;
  };
  int get_type(void)
  {
    return type;
  };
  void set(void* d, unsigned int t)
  {
    data = d;
    type = t;
    length = 0;
    pad = false;
  };
  void set(void* d, unsigned int t, unsigned int l, bool p)
  {
    data = d;
    type = t;
    length = l;
    pad = p;
  };
  void set(void* d)
  {
    data = d;
    type = ((dhc_frame_header_word0*)data)->get_type();
    length = 0;
    pad = false;
  };
  unsigned int get_evtnr_lo(void)
  {
    return ((unsigned short*)data)[1];
  };
  unsigned int calc_crc(void)
  {
    unsigned char* d;
    dhe_crc_32_type bocrc;
    char crcbuffer[65536 * 16]; /// 1MB
    d = (unsigned char*)data;

    if (length > 65536 * 16) {
      B2WARNING("DHC Data Frame CRC FAIL bacause of too large packet (>1MB)!");
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
      if (verbose)
        //         B2INFO("DHE Data Frame CRC: " << hex << c << "==" << crc32);
        B2INFO("DHC Data Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
               << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " pad " << pad << " len $" << length);
    } else {
      crc_error++;
      error_mask |= ONSEN_ERR_FLAG_DHE_CRC;
      if (verbose) {
        B2ERROR("DHC Data Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
                << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " pad " << pad << " len $" << length);
        /// others would be interessting but possible subjects to access outside of buffer
        /// << " " << * (unsigned int*)(d + length - 2) << " " << * (unsigned int*)(d + length + 0) << " " << * (unsigned int*)(d + length + 2));
        //if (length <= 32) {
        //  for (int i = 0; i < length / 4; i++) {
        //    B2ERROR("== " << i << "  $" << hex << ((unsigned int*)d)[i]);
        //  }
        //}
      };
      error_flag = true;
    }
    return c;
  };


  unsigned int size(void)
  {
    unsigned int s = 0;
    switch (get_type()) {
      case DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW:
        s = ((dhc_direct_readout_frame_raw*)data)->size();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_DHP_ONS:
      case DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD:
        s = ((dhc_direct_readout_frame_zsd*)data)->size();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW:
        B2INFO("Error: FCE type no supported ");
        s = 0;
        error_flag = true;
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_COMMODE:
        s = ((dhc_commode_frame*)data)->size();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_GHOST:
        s = ((dhc_ghost_frame*)data)->size();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_DHE_START:
        s = ((dhc_dhe_start_frame*)data)->size();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_DHE_END:
        s = ((dhc_dhe_end_frame*)data)->size();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_DHC_START:
        s = ((dhc_start_frame*)data)->size();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_DHC_END:
        s = ((dhc_end_frame*)data)->size();
        break;
      case DHC_FRAME_HEADER_DATA_TYPE_HLTROI:
        s = 0;
        break;
      default:
        B2ERROR("Error: not a valid data frame!");
        error_flag = true;
        s = 0;
        break;
    }
    datasize = s;
    return s;
  };

  void write_pedestal(void)
  {
    B2INFO("Write Pedestal Data - not implemented... !");
  };

};

///******************************************************************
///*********************** Main unpacker code ***********************
///******************************************************************

PXDUnpackerDesy1314Module::PXDUnpackerDesy1314Module() :
  Module(),
  m_storeRawHits(),
  m_storeROIs()
{
  //Set module properties
  setDescription("Unpack Raw PXD Hits from ONSEN data stream (Desy2013/14 test)");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("HeaderEndianSwap", m_headerEndianSwap, "Swap the endianess of the ONSEN header", true);
  addParam("DHCmode", m_DHCmode, "Run in DHC mode", true);
  addParam("IgnoreDATCON", m_ignoreDATCON, "Ignore missing DATCON ROIs", true);
  addParam("IgnoreDHCFrameNr", m_ignore_headernrframes, "Ignore Wrong Nr Frames in DHC Start", true);
  addParam("IgnoreDHPMask", m_ignore_dhpmask, "Ignore missing DHP from DHE Start mask", true);
  addParam("IgnoreDHPPortDiffer", m_ignore_dhpportdiffer, "Ignore if DHP port differ in DHE and DHP header", true);
  addParam("IgnoreEmptyDHPWrongSize", m_ignore_empty_dhp_wrong_size, "Ignore empty. wrong sized DHP packets", true);
  addParam("DoNotStore", m_doNotStore, "only unpack and check, but do not store", false);

}

void PXDUnpackerDesy1314Module::initialize()
{
  //Register output collections
  m_storeRawHits.registerInDataStore();
  m_storeROIs.registerInDataStore();
  /// actually, later we do not want o store it into output file ...  aside from debugging
  B2INFO("HeaderEndianSwap: " << m_headerEndianSwap);
  B2INFO("DHCmode: " << m_DHCmode);
  B2INFO("Ignore(missing)DATCON: " << m_ignoreDATCON);
  ignore_datcon_flag = m_ignoreDATCON;

  unpacked_events = 0;
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) error_counter[i] = 0;
}

void PXDUnpackerDesy1314Module::terminate()
{
  int flag = 0;
  string errstr = "Statistic ( ;";
  errstr += to_string(unpacked_events) + ";";
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) { errstr += to_string(error_counter[i]) + ";"; flag |= error_counter[i];}
  if (flag != 0) {
    B2ERROR("PXD UnpackerDesy1314 --> Error Statistics (counted once per event!) in Events: " << unpacked_events);
    B2ERROR(errstr + " )");
    for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      if (error_counter[i]) {
        B2ERROR(error_name[i] << ": " << error_counter[i]);
        //printf("%s %d\n",error_name[i].c_str(), error_counter[i]);
      }
    }
  } else {
    B2INFO("PXD UnpackerDesy1314 --> No Error found in Events: " << unpacked_events);
  }
}

void PXDUnpackerDesy1314Module::event()
{
  StoreArray<RawPXD> storeRaws;
  StoreArray<RawFTSW> storeFTSW;

  int nRaws = storeRaws.getEntries();
  if (verbose) {
    B2INFO("PXD UnpackerDesy1314 --> RawPXD Objects in event: " << nRaws);
  };

  error_mask = 0;
  ftsw_evt_nr = 0;
  ftsw_evt_mask = 0;
  for (auto& it : storeFTSW) {
    ftsw_evt_nr = it.GetEveNo(0);
    ftsw_evt_mask = 0x7FFF;
    B2INFO("PXD UnpackerDesy1314 --> FTSW Event Number: $" << hex << ftsw_evt_nr);
    break;
  }

  int nsr = 0;
  for (auto& it : storeRaws) {
    if (verbose) {
      B2INFO("PXD UnpackerDesy1314 --> Unpack Objects: ");
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

void PXDUnpackerDesy1314Module::endian_swap_frame(unsigned short* dataptr, int len)
{
  /// swap endianess of all shorts in frame BUT not the CRC (2 shorts)
  for (int i = 0; i < len / 2 - 2; i++) {
    dataptr[i] = htons(dataptr[i]);
  }
}

void PXDUnpackerDesy1314Module::unpack_event(RawPXD& px)
{
  int last_wie = 0;
  static unsigned int last_evtnr = 0;
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD UnpackerDesy1314 --> invalid packet size (32bit words) " << hex << px.size());
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

  if (m_headerEndianSwap) Frames_in_event = ntohl(data[1]);
  else Frames_in_event = data[1];
  if (Frames_in_event < 0 || Frames_in_event > 256) {
    B2ERROR("Number of Frames invalid: Will not unpack anything. Header corrupted! Frames in event: " << Frames_in_event);
    error_mask |= ONSEN_ERR_FLAG_FRAME_NR;
    return;
  }

  /// NEW format
  if (verbose) {
    B2INFO("PXD UnpackerDesy1314 --> data[0]: <-- Magic " << hex << data[0]);
    B2INFO("PXD UnpackerDesy1314 --> data[1]: <-- #Frames " << hex << data[1]);
    if (data[1] >= 1 && fullsize < 12) B2INFO("PXD UnpackerDesy1314 --> data[2]: <-- Frame 1 len " << hex << data[2]);
    if (data[1] >= 2 && fullsize < 16) B2INFO("PXD UnpackerDesy1314 --> data[3]: <-- Frame 2 len " << hex << data[3]);
    if (data[1] >= 3 && fullsize < 20) B2INFO("PXD UnpackerDesy1314 --> data[4]: <-- Frame 3 len " << hex << data[4]);
    if (data[1] >= 4 && fullsize < 24) B2INFO("PXD UnpackerDesy1314 --> data[5]: <-- Frame 4 len " << hex << data[5]);
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
    if (m_headerEndianSwap) lo = ntohl(tableptr[j]);
    else lo = tableptr[j];
    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      error_mask |= ONSEN_ERR_FLAG_FRAME_SIZE;
      return;
    }
    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " <<
              datafullsize << " of fullsize " << fullsize);
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

    B2INFO("unpack DHE(C) frame: " << j << " with size " << lo << " at byte offset in dataptr " << ll);
    endian_swap_frame((unsigned short*)(ll + (char*)dataptr), lo);
    if (m_DHCmode) {
      unpack_dhc_frame(ll + (char*)dataptr, lo, pad, last_wie, last_evtnr, j, Frames_in_event);
    } else {
      B2ERROR("old plain DHE code has been removed!!!");
    }
    ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
  }

}

void PXDUnpackerDesy1314Module::unpack_dhp(void* data, unsigned int len2, unsigned int dhe_first_readout_frame_id_lo,
                                           unsigned int dhe_ID, unsigned dhe_DHPport, unsigned dhe_reformat, unsigned short toffset, VxdID vxd_id)
{
  unsigned int anzahl = len2 / 2; // len2 in bytes!!!
  bool printflag = false;
  unsigned short* dhp_pix = (unsigned short*)data;

  unsigned int dhp_readout_frame_lo = 0;
  unsigned int dhp_header_type  = 0;
  unsigned int dhp_reserved     = 0;
  unsigned int dhp_dhe_id       = 0;
  unsigned int dhp_dhp_id       = 0;

  unsigned int dhp_row = 0, dhp_col = 0, dhp_adc = 0, dhp_cm = 0;
  unsigned int dhp_offset = 0;
  bool rowflag = false;

  if (anzahl < 4) {
    if (!m_ignore_empty_dhp_wrong_size) B2ERROR("DHP frame size error (too small) " << anzahl);
    error_mask |= ONSEN_ERR_FLAG_DHP_SIZE;
    dhp_size_error++;
    return;
    //return -1;
  }

  if (printflag)
    B2INFO("HEADER --  " << hex << dhp_pix[0] << hex << dhp_pix[1] << hex << dhp_pix[2] << hex << dhp_pix[3] << " -- ");

  if (printflag)
    B2INFO("DHP Header     |   " << hex << dhp_pix[2] << " ( " << hex << dhp_pix[2] << " ) ");
  dhp_header_type  = (dhp_pix[2] >> 13) & 0x07;
  dhp_reserved     = (dhp_pix[2] >> 8) & 0x1F;
  dhp_dhe_id       = (dhp_pix[2] >> 2) & 0x3F;
  dhp_dhp_id       =  dhp_pix[2] & 0x03;

  if (printflag) {
    B2INFO("DHP type          |    " << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
    B2INFO("DHP reserved          |    " << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
    B2INFO("DHP DHE ID          |    " << hex << dhp_dhe_id << " ( " << dec << dhp_dhe_id << " ) ");
    B2INFO("DHP DHP ID          |    " << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  }

  if (dhe_ID != dhp_dhe_id) {
    B2ERROR("DHE ID in DHE and DHP header differ ($" << hex << dhe_ID << " != $" << dhp_dhe_id);
    error_mask |= ONSEN_ERR_FLAG_DHE_DHP_DHEID;
  }
  if (dhe_DHPport != dhp_dhp_id && !m_ignore_dhpportdiffer) {
    B2ERROR("DHP ID (Chip/Port) in DHE and DHP header differ ($" << hex << dhe_DHPport << " != $" << dhp_dhp_id);
    error_mask |= ONSEN_ERR_FLAG_DHE_DHP_PORT;
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
          dhp_pixel_error++;
          return;
        } else {
          dhp_row = (dhp_row & 0xFFE) | ((dhp_pix[i] >> 14) & 0x001);
          dhp_col = ((dhp_pix[i] >> 8) & 0x3F);
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
    /*for (int i = 0; i < raw_anzahl ; i++) {
      B2INFO("RAW      |   " << hex << p_pix[i]);
      printf("raw %08X  |  ", p_pix[i]);
      B2INFO("row " << hex << ((p_pix[i] >> 20) & 0xFFF) << dec << " ( " << ((p_pix[i] >> 20) & 0xFFF) << " ) " << " col " << hex << ((p_pix[i] >> 8) & 0xFFF)
             << " ( " << dec << ((p_pix[i] >> 8) & 0xFFF) << " ) " << " adc " << hex << (p_pix[i] & 0xFF) << " ( " << (p_pix[i] & 0xFF) << " ) "
            );
    }*/
  }
};

int nr5bits(int i)
{
  /// too lazy to count the bits myself, thus using a small lookup table
  const int lut[32] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5
  };
  return lut[i & 0x1F];
}

void PXDUnpackerDesy1314Module::unpack_dhc_frame(void* data, int len, bool pad, int& last_wie, unsigned int& last_evtnr,
                                                 int Frame_Number, int Frames_in_event)
{
  static int nr_of_dhe_start_frame = 0; /// could put it as a class member, but is only needed within this function
  static int nr_of_dhe_end_frame = 0; /// could put it as a class member, but is only needed within this function
  static int nr_of_frames_dhc = 0;
  static int nr_of_frames_counted = 0;
  static int nr_active_dhe = 0;
  static int mask_active_dhe = 0;
  ///  static int nr_active_dhp = 0;// unused
  static int mask_active_dhp = 0;
  static int found_mask_active_dhp = 0;
  static unsigned int dhe_first_readout_frame_id_lo = 0;
  static unsigned int dhe_first_offset = 0;
  static unsigned int current_dhe_id = 0xFFFFFFFF;
  static unsigned int currentVxdId = 0;
  static bool is_fake_event = false;


  dhc_frame_header_word0* hw = (dhc_frame_header_word0*)data;
  error_flag = false;

  dhc_frames dhc;
  dhc.set(data, hw->get_type(), len, pad);
  int s;
  s = dhc.size();
  if (len != s && s != 0) {
    B2ERROR("Fixed frame type size does not match specs: expect " << len << " != " << s << " (in data) " << pad);
    error_mask |= ONSEN_ERR_FLAG_FIX_SIZE;
  }

  unsigned int evtnr = dhc.get_evtnr_lo();
  int type = dhc.get_type();

  if (Frame_Number == 0) { /// We reset the counters on the first event
    nr_of_dhe_start_frame = 0;
    nr_of_dhe_end_frame = 0;
    if (type == DHC_FRAME_HEADER_DATA_TYPE_DHC_START) {
      is_fake_event = ((dhc_start_frame*)data)->is_fake();
    } else {
      is_fake_event = false;
    }
  }


  if ((evtnr & ftsw_evt_mask) != (ftsw_evt_nr & ftsw_evt_mask)) {
    if (is_fake_event) {
      /// If ist a start, check if its a fake event, if its not a start, the fake flag is already (re)set. As long as the data structure is not messed up completly.
      /// no need to explicitly check the DHC_END or HLT/ROI
      /// We have afake and shoudl set the trigger nr by hand to prevent further errors
      evtnr = ftsw_evt_nr & ftsw_evt_mask; // masking might be a problem as we cannore recover all bits
    } else {
      B2ERROR("Event Numbers do not match for this frame $" << hex << evtnr << "!=$" << ftsw_evt_nr << "(FTSW) mask $" << ftsw_evt_mask);
      error_mask |= ONSEN_ERR_FLAG_FTSW_DHC_MM;
    }
  }

  if (Frame_Number > 0 && Frame_Number < Frames_in_event - 1) {
    if (nr_of_dhe_start_frame != nr_of_dhe_end_frame + 1)
      if (type != DHC_FRAME_HEADER_DATA_TYPE_HLTROI && type != DHC_FRAME_HEADER_DATA_TYPE_DHE_START) {
        B2ERROR("Data Frame outside a DHE START/END");
        error_mask |= ONSEN_ERR_FLAG_DATA_OUTSIDE;
      }
  }

  switch (type) {
    case DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW: {
      nr_of_frames_counted++;

      ((dhc_direct_readout_frame_raw*)data)->print();
      if (current_dhe_id != ((dhc_direct_readout_frame_raw*)data)->get_dhe_id()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << current_dhe_id << " != $" << ((
                  dhc_direct_readout_frame_raw*)data)->get_dhe_id());
        error_mask |= ONSEN_ERR_FLAG_DHE_START_ID;
      }
      dhc.calc_crc();
      found_mask_active_dhp |= 1 << ((dhc_direct_readout_frame*)data)->get_dhp_port();

      stat_raw++;
      dhc.write_pedestal();
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_DHP_ONS:
    case DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD: {
      nr_of_frames_counted++;

      ((dhc_direct_readout_frame*)data)->print();
      if (current_dhe_id != ((dhc_direct_readout_frame_raw*)data)->get_dhe_id()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << current_dhe_id << " != $" << ((
                  dhc_direct_readout_frame_raw*)data)->get_dhe_id());
        error_mask |= ONSEN_ERR_FLAG_DHE_START_ID;
      }
      if (m_ignore_empty_dhp_wrong_size && type == DHC_FRAME_HEADER_DATA_TYPE_DHP_ONS && len == 10) {
        // Bug from Davids code ... ignore but count as error -> in unpack... ignore crc error
        // error_mask |= ONSEN_ERR_FLAG_DHE_CRC;
      } else {
        dhc.calc_crc();
      }
      found_mask_active_dhp |= 1 << ((dhc_direct_readout_frame*)data)->get_dhp_port();
      stat_zsd++;

      unpack_dhp(data, len - 4,
                 dhe_first_readout_frame_id_lo,
                 ((dhc_direct_readout_frame*)data)->get_dhe_id(),
                 ((dhc_direct_readout_frame*)data)->get_dhp_port(),
                 ((dhc_direct_readout_frame*)data)->get_reformat_flag(),
                 dhe_first_offset, currentVxdId);
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW: {
      nr_of_frames_counted++;

      hw->print();
      if (current_dhe_id != ((dhc_direct_readout_frame_raw*)data)->get_dhe_id()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << current_dhe_id << " != $" << ((
                  dhc_direct_readout_frame_raw*)data)->get_dhe_id());
        error_mask |= ONSEN_ERR_FLAG_DHE_START_ID;
      }
      dhc.calc_crc();
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_COMMODE: {
      nr_of_frames_counted++;

      hw->print();
      if (current_dhe_id != ((dhc_commode_frame*)data)->get_dhe_id()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << current_dhe_id << " != $" << ((
                  dhc_commode_frame*)data)->get_dhe_id());
        error_mask |= ONSEN_ERR_FLAG_DHE_START_ID;
      }
      dhc.calc_crc();
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_DHC_START: {
      if (is_fake_event != ((dhc_start_frame*)data)->is_fake()) {
        B2ERROR("DHC START is but no Fake event OR Fake Event but DHE END is not.");
      }
      if (((dhc_start_frame*)data)->is_fake()) {
        B2WARNING("Faked DHC START Data -> trigger without Data!");
        error_mask |= ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG;
      } else {
        ((dhc_start_frame*)data)->print();
      }

      last_evtnr = evtnr;
      current_dhe_id = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      nr_of_frames_dhc = ((dhc_start_frame*)data)->get_dhc_nr_frames();
      nr_of_frames_counted = 1;
      dhc.calc_crc();
      stat_start++;

      mask_active_dhe = ((dhc_start_frame*)data)->get_active_dhe_mask();
      nr_active_dhe = nr5bits(mask_active_dhe);
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_DHE_START: {
      nr_of_frames_counted++;
      ((dhc_dhe_start_frame*)data)->print();
      dhe_first_readout_frame_id_lo = ((dhc_dhe_start_frame*)data)->get_sfnr();
      dhe_first_offset = ((dhc_dhe_start_frame*)data)->get_toffset();
      current_dhe_id = ((dhc_dhe_start_frame*)data)->get_dhe_id();
      dhc.calc_crc();

      if (nr_of_dhe_start_frame != nr_of_dhe_end_frame) {
        B2ERROR("DHC_FRAME_HEADER_DATA_TYPE_DHE_START without DHC_FRAME_HEADER_DATA_TYPE_DHE_END");
        error_mask |= ONSEN_ERR_FLAG_DHE_START_WO_END;
      }
      nr_of_dhe_start_frame++;

      found_mask_active_dhp = 0;
      mask_active_dhp = ((dhc_dhe_start_frame*)data)->get_active_dhp_mask();
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
        sensor = (current_dhe_id & 0x1) + 1;
        ladder = (current_dhe_id & 0x1E) >> 1; // no +1
        layer = ((current_dhe_id & 0x20) >> 5) + 1;
        currentVxdId = VxdID(layer, ladder, sensor);
      }
      //currentVxdId = current_dhe_id;

      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_GHOST:
      nr_of_frames_counted++;
      ((dhc_ghost_frame*)data)->print();
      if (current_dhe_id != ((dhc_ghost_frame*)data)->get_dhe_id()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << current_dhe_id << " != $" << ((
                  dhc_ghost_frame*)data)->get_dhe_id());
        error_mask |= ONSEN_ERR_FLAG_DHE_START_ID;
      }
      /// Attention: Firmware might be changed such, that ghostframe come for all DHPs, not only active ones...
      found_mask_active_dhp |= 1 << ((dhc_ghost_frame*)data)->get_dhp_port();
      dhc.calc_crc();
      stat_ghost++;

      break;
    case DHC_FRAME_HEADER_DATA_TYPE_DHC_END: {
      if (((dhc_end_frame*)data)->is_fake() != is_fake_event) {
        B2ERROR("DHC END is but no Fake event OR Fake Event but DHE END is not.");
      }
      nr_of_frames_counted++;
      current_dhe_id = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      if (is_fake_event) {
        B2WARNING("Faked DHC END Data -> trigger without Data!");
        error_mask |= ONSEN_ERR_FLAG_FAKE_NO_DATA_TRIG;
      } else {
        ((dhc_end_frame*)data)->print();
      }
      stat_end++;

      if (!is_fake_event) {
        if (nr_of_frames_counted != nr_of_frames_dhc) {
          if (!m_ignore_headernrframes) B2ERROR("Number of DHC Frames in Header " << nr_of_frames_dhc << " != " << nr_of_frames_counted <<
                                                  " Counted");
          error_mask |= ONSEN_ERR_FLAG_DHC_FRAMECOUNT;
        }
      }
      if (!is_fake_event) {
        int w;
        w = ((dhc_end_frame*)data)->get_words() * 2;
        last_wie += 2;
        if (verbose) {
          B2INFO("last_wie " << last_wie << " w " << w);
        };
        if (last_wie != w) {
          if (verbose) {
            B2INFO("Error: WIE " << hex << last_wie << " vs END " << hex << w << " pad " << pad);
          };
          error_flag = true;
          wie_error++;
        } else {
          if (verbose)
            B2INFO("EVT END: WIE " << hex << last_wie << " == END " << hex << w << " pad " << pad);
        }
      }
      dhc.calc_crc();
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_DHE_END: {
      nr_of_frames_counted++;
      ((dhc_dhe_end_frame*)data)->print();
      if (current_dhe_id != ((dhc_dhe_end_frame*)data)->get_dhe_id()) {
        B2ERROR("DHE ID from DHE Start and this frame do not match $" << hex << current_dhe_id << " != $" << ((
                  dhc_dhe_end_frame*)data)->get_dhe_id());
        error_mask |= ONSEN_ERR_FLAG_DHE_START_END_ID;
      }
      current_dhe_id = 0xFFFFFFFF;
      currentVxdId = 0; /// invalid
      dhc.calc_crc();
      if (found_mask_active_dhp != mask_active_dhp) {
        if (!m_ignore_dhpmask) B2ERROR("DHE_END: DHP active mask $" << hex << mask_active_dhp << " != $" << hex << found_mask_active_dhp <<
                                         " mask of found dhp/ghost frames");
        error_mask |= ONSEN_ERR_FLAG_DHP_ACTIVE;
      }
      nr_of_dhe_end_frame++;
      if (nr_of_dhe_start_frame != nr_of_dhe_end_frame) {
        B2ERROR("DHC_FRAME_HEADER_DATA_TYPE_DHE_END without DHC_FRAME_HEADER_DATA_TYPE_DHE_START");
        error_mask |= ONSEN_ERR_FLAG_DHE_START;
      }
      break;
    };
    case DHC_FRAME_HEADER_DATA_TYPE_HLTROI:
      //nr_of_frames_counted++;/// DO NOT COUNT!!!!
      //((dhc_onsen_frame*)data)->set_length(len - 4);
      ((dhc_onsen_frame*)data)->print();
      ((dhc_onsen_frame*)data)->calc_crc(len - 4); /// CRC is without the DHC header
      dhc.calc_crc();
      if (!m_doNotStore)((dhc_onsen_frame*)data)->save(m_storeROIs, len, (unsigned int*) data);
      break;
    default:
      B2ERROR("UNKNOWN DHC frame type");
      error_mask |= ONSEN_ERR_FLAG_DHC_UNKNOWN;
      type_error++;
      hw->print();
      error_flag = true;
      break;
  }

  if (evtnr != last_evtnr) {
    B2ERROR("Frame TrigNr != DHC Trig Nr $" << hex << evtnr << " != $" << last_evtnr);
    error_mask |= ONSEN_ERR_FLAG_DHC_DHE_MM;
    evtnr_error++;
    error_flag = true;
  }

  if (Frame_Number == 0) {
    /// Check that DHC Start is first Frame
    if (type != DHC_FRAME_HEADER_DATA_TYPE_DHC_START) {
      B2ERROR("First frame is not a DHC start of subevent frame in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHC_START;
    }
  } else { // (Frame_Number != 0 &&
    /// Check that there is no other DHC Start
    if (type == DHC_FRAME_HEADER_DATA_TYPE_DHC_START) {
      B2ERROR("More than one DHC start of subevent frame in frame in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHC_START2;
    }
  }

  if (Frame_Number == Frames_in_event - 1) {
    /// Check that DHC End is last Frame
    if (type != DHC_FRAME_HEADER_DATA_TYPE_DHC_END) {
      B2ERROR("Last frame is not a DHC end of subevent frame in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHC_END;
    }

    /// As we now have processed the whole event, we can do some more consistency checks!
    if (nr_of_dhe_start_frame != nr_of_dhe_end_frame || nr_of_dhe_start_frame != nr_active_dhe) {
      B2ERROR("The number of DHE Start/End does not match the number of active DHE in DHC Header! Header: " << nr_active_dhe << " Start: "
              << nr_of_dhe_start_frame << " End: " << nr_of_dhe_end_frame << " Mask: $" << hex << mask_active_dhe << " in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHE_ACTIVE;
    }

  } else { //  (Frame_Number != Frames_in_event - 1 &&
    /// Check that there is no other DHC End
    if (type == DHC_FRAME_HEADER_DATA_TYPE_DHC_END) {
      B2ERROR("More than one DHC end of subevent frame in frame in Event Nr " << evtnr);
      error_mask |= ONSEN_ERR_FLAG_DHC_END2;
    }
  }

  /// Check that (if there is at least one active DHE) the second Frame is DHE Start, actually this is redundant if the other checks work
  if (Frame_Number == 1 && nr_active_dhe != 0 && type != DHC_FRAME_HEADER_DATA_TYPE_DHE_START) {
    B2ERROR("Second frame is not a DHE start frame in Event Nr " << evtnr);
    error_mask |= ONSEN_ERR_FLAG_DHE_START;
  }

  last_wie += len;

}



