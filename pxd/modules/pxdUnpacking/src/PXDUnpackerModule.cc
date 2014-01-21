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
//#include <rawdata/dataobjects/RawPXD.h>
#include <rawdata/dataobjects/RawFTSW.h>

// for htonl
#include <arpa/inet.h>

#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

#define DHP_FRAME_HEADER_DATA_TYPE_RAW  0x0
#define DHP_FRAME_HEADER_DATA_TYPE_ZSD  0x5

// DHHC envelope
#define DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START  0xB
#define DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END    0xC
// Onsen processed data
#define DHHC_FRAME_HEADER_DATA_TYPE_DHP_ONS     0xD
#define DHHC_FRAME_HEADER_DATA_TYPE_FCE_ONS     0x9
#define DHHC_FRAME_HEADER_DATA_TYPE_HLTROI      0xF

// DHH like above, but format has changed
#define DHHC_FRAME_HEADER_DATA_TYPE_DHP_RAW     0x0
#define DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD     0x5
#define DHHC_FRAME_HEADER_DATA_TYPE_FCE_RAW     0x1
#define DHHC_FRAME_HEADER_DATA_TYPE_COMMODE     0x6
#define DHHC_FRAME_HEADER_DATA_TYPE_GHOST       0x2
#define DHHC_FRAME_HEADER_DATA_TYPE_DHH_START   0x3
#define DHHC_FRAME_HEADER_DATA_TYPE_DHH_END     0x4

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhh_crc_32_type;


unsigned int type_error = 0, crc_error = 0, magic_error = 0, zerodata_error = 0;
unsigned int queue_error = 0, fnr_error = 0, wie_error = 0, end_error = 0, evt_skip_error = 0, start_error = 0, evtnr_error = 0;
// unsigned long long evt_counter = 0;
unsigned int stat_start = 0, stat_end = 0, stat_ghost = 0, stat_raw = 0, stat_zsd = 0;
unsigned int dhp_size_error = 0, dhp_pixel_error = 0, dhp_warning = 0;
bool verbose = true;
bool error_flag = false;
bool ignore_datcon_flag = true;

///*********************************************************************************
///***************************** DHHC Code starts here *****************************
///*********************************************************************************

char* dhhc_type_name[16] = {
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


class dhhc_frame_header_word0 {
public:
  unsigned short data;
  dhhc_frame_header_word0(unsigned int error_flag = 0, unsigned int data_typ = 0, unsigned int dependent = 0) {
    data = ((error_flag & 0x1) << 15) | ((data_typ & 0xF) << 11) | (dependent & 0x3FF);
  };
  unsigned int get_type(void) {
    return (data >> 11) & 0xF;
  };
  unsigned int get_err(void) {
    return (data >> 15) & 0x1;
  };
  unsigned int get_misc(void) {
    return data & 0x3FF;
  };
  void print(void) {
    if (verbose)
      B2INFO("DHHC FRAME TYP " << hex << get_type() << " -> " << dhhc_type_name[get_type()] << " ERR " << get_err() << " data " << data);
  };
};

class dhhc_start_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned short trigger_nr_hi;
  unsigned short time_tag_lo_and_type;
  unsigned short time_tag_mid;
  unsigned short time_tag_hi;
  unsigned short nr_frames_in_event;
  unsigned short crc32lo;/// Changed to 2*16 because of automatic compiler alignment
  unsigned short crc32hi;

  unsigned short get_evtnr_lo(void) {
    return trigger_nr_lo;
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
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
        B2INFO("DHHC Start Frame CRC " << hex << c << " == " << hex << crc32);
    } else {
      crc_error++;
      B2ERROR("DHHC Start Frame CRC FAIL " << hex << c << " != " << hex << crc32);
      error_flag = true;
    }
    return c;
  };
  bool is_fake(void) {
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
  inline static unsigned int size(void) {
    return 18;// bytes
  };
  void print(void) {
    word0.print();
    if (verbose) {
      unsigned int crc32 = (crc32hi << 16) | crc32lo;
      B2INFO("DHHC Start Frame TNRLO $" << hex << trigger_nr_lo << " TNRHI $" << hex << trigger_nr_hi << " TTLO $" << hex << time_tag_lo_and_type
             << " TTMID $" << hex << time_tag_mid << " TTHI $" << hex << time_tag_hi << " Frames in Event " << dec << nr_frames_in_event
             << " CRC $" << hex << crc32 << " (calc) $" << calc_crc());
    }
  };
  inline unsigned int get_active_dhh_mask(void) {return word0.get_misc() & 0x1F;};
  inline unsigned int get_dhhc_id(void) {return (word0.get_misc() >> 5) & 0xF;};
  inline unsigned int get_dhhc_nr_frames(void) {return nr_frames_in_event;};
};

class dhhc_dhh_start_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned short dhh_time_tag_lo;
  unsigned short dhh_time_tag_hi;
  unsigned short sfnr_offset;
  unsigned short crc32lo;/// Changed to 2*16 because of automatic compiler alignment
  unsigned short crc32hi;

//    dhhc_dhh_start_frame(unsigned int time_tag = 0, unsigned int trigger_nr = 0, unsigned int depend = 0): word0(0, DHH_FRAME_HEADER_DATA_TYPE_EVT_FRM, depend) {
//    };
  inline unsigned short get_evtnr_lo(void) {
    return trigger_nr_lo;
  };
  inline unsigned short get_sfnr(void) {// last DHP fraem before trigger
    return (sfnr_offset >> 10) & 0x3F;
  };
  inline unsigned short get_toffset(void) {// and trigger row offset
    return sfnr_offset & 0x3FF;
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
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
        B2INFO("DHHC DHH Start Frame CRC " << hex << c << " == " << hex << crc32);
    } else {
      crc_error++;
      B2ERROR("DHHC DHH Start CRC FAIL " << hex << c << " != " << hex << crc32);
      error_flag = true;
    }
    return c;
  };
  inline static unsigned int size(void) {
    return 14;// 7 words
  };
  void print(void) {
    word0.print();
    if (verbose) {
      unsigned int crc32 = (crc32hi << 16) | crc32lo;
      B2INFO("DHHC Event Frame TNRLO $" << hex << trigger_nr_lo  << " DTTLO $" << hex << dhh_time_tag_lo << " DTTHI $" << hex << dhh_time_tag_hi
             << " DHHID $" << hex << get_dhh_id()
             << " DHPMASK $" << hex << get_active_dhp_mask()
             << " SFNR $" << hex << get_sfnr()
             << " OFF $" << hex << get_toffset()
             << " CRC " << hex << crc32 << " (calc)" << calc_crc());
    }
  };
  inline unsigned int get_active_dhp_mask(void) {return word0.get_misc() & 0xF;};
  inline unsigned int get_dhh_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
};

class dhhc_commode_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned short data[96];
  unsigned int crc32;

  inline unsigned short get_evtnr_lo(void) {
    return trigger_nr_lo;
  };
  inline static unsigned int size(void) {
    return (2 + 96 / 2) * 4;
  };
  inline unsigned int get_dhh_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
};

class dhhc_direct_readout_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  /// an unbelievable amount of words may follow
  /// and finally a 32 bit checksum

  inline static unsigned int size(void) {
    return 0;// size can vary
  };
  inline unsigned short get_evtnr_lo(void) {
    return trigger_nr_lo;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO("DHHC Direct Readout (Raw|ZSD|ONS) Frame TNRLO $" << hex << trigger_nr_lo << " DHH ID $" << get_dhh_id() << " DHP port $" << get_dhp_port());
  };
  inline unsigned short get_dhh_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
  inline unsigned short get_dhp_port(void) {return (word0.get_misc()) & 0x3;};
  inline bool get_reformat_flag(void) {return (word0.get_misc() >> 3) & 0x1;};
};

class dhhc_direct_readout_frame_raw : public dhhc_direct_readout_frame {
public:
};

class dhhc_direct_readout_frame_zsd : public dhhc_direct_readout_frame {
public:
};

class dhhc_onsen_frame {
  dhhc_frame_header_word0 word0;/// mainly empty
  unsigned short trignr0;// not used
  unsigned int magic1;
  unsigned int trignr1;
  unsigned int magic2;
  unsigned int trignr2;
  /// plus n* ROIs (64 bit)
  /// plus checksum 32bit
  /// unsigned int length;/// not part
public:
  inline unsigned short get_trig_nr0(void) {
    return trignr0;
  };
  inline unsigned short get_trig_nr1(void) {
    return trignr1;
  };
  inline unsigned short get_trig_nr2(void) {
    return trignr2;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO("DHHC HLT/ROI Frame " << hex << trignr1 << " ," << trignr2);
    if ((magic1 & 0xFFFF) != 0xCAFE) B2ERROR("DHHC HLT/ROI Magic 1 error $" << hex << magic1);
    if ((magic2 & 0xFFFF) != 0xCAFE) B2ERROR("DHHC HLT/ROI Magic 2 error $" << hex << magic2);
    if (magic2 == 0x0000CAFE && trignr2 == 0x00000000) {
      if (!ignore_datcon_flag) B2WARNING("DHHC HLT/ROI Frame: No DATCON data " << hex << trignr1 << "!=$" << trignr2);
    } else {
      if (trignr1 != trignr2) B2ERROR("DHHC HLT/ROI Frame Trigger Nr Mismatch $" << hex << trignr1 << "!=$" << trignr2);
    }
  };
//  inline static unsigned int size(void) {
//    return 0;// siez can vary
//  };

  unsigned int calc_crc(unsigned int length) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[65536 * 2]; /// 128kB
    d = (unsigned char*) &magic1;/// without the DHHC header as its only an inner checksum!!!

    if (length > 65536 * 2) {
      B2WARNING("DHHC ONSEN HLT/ROI Frame CRC FAIL bacause of too large packet (>128kB)!");
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
        B2INFO("DHHC ONSEN HLT/ROI Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
               << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " len $" << length);
    } else {
      crc_error++;
      if (verbose) {
        B2ERROR("DHHC ONSEN HLT/ROI Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
                << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " len $" << length);
        /// others would be interessting but possible subjects to access outside of buffer
        /// << " " << * (unsigned int*)(d + length - 2) << " " << * (unsigned int*)(d + length + 0) << " " << * (unsigned int*)(d + length + 2));
        if (length <= 64) {
          for (unsigned int i = 0; i < length / 4; i++) {
            B2ERROR("== " << i << "  $" << hex << ((unsigned int*)d)[i]);
          }
        }
      };
      error_flag = true;
    }
    return c;
  };
};

class dhhc_ghost_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int crc32;

  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
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
        B2INFO("DHHC Ghost Frame CRC " << hex << c << " == " << crc32);
    } else {
      crc_error++;
      B2ERROR("DHHC Ghost Frame CRC FAIL " << hex << c << " != " << crc32);
      error_flag = true;
    }
    return c;
  };
  inline static unsigned int size(void) {
    return 8;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO("DHHC Ghost Frame TNRLO " << hex << trigger_nr_lo << " DHH ID $" << get_dhh_id() << " DHP port $" << get_dhp_port() << " CRC $"  << crc32 << " (calc) "  << calc_crc());
  };
  inline unsigned short get_dhh_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
  inline unsigned short get_dhp_port(void) {return (word0.get_misc()) & 0x3;};
};

class dhhc_end_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int wordsinevent;
  unsigned int errorinfo;
  unsigned int crc32;

  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
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
        B2INFO("DHHC End Frame CRC " << hex << c << "==" << crc32);
    } else {
      crc_error++;
      B2ERROR("DHHC End Frame CRC " << hex << c << "!=" << crc32);
      error_flag = true;
    }
    return c;
  };
  unsigned int get_words(void) {
    return wordsinevent;
  }
  inline static unsigned int size(void) {
    return 16;
  };
  bool is_fake(void) {
    if (word0.data != 0x6000) return false;
    if (trigger_nr_lo != 0) return false;
    if (wordsinevent != 0) return false;
    if (errorinfo != 0) return false;
    if (crc32 != 0xF7BCA507) return false;
    return true;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO("DHHC End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
             << " CRC " << hex << crc32 << " (calc) " << calc_crc());
  };
  inline unsigned int get_dhhc_id(void) {return (word0.get_misc() >> 5) & 0xF;};
};

class dhhc_dhh_end_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int wordsinevent;
  unsigned int errorinfo;
  unsigned int crc32;

  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
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
        B2INFO("DHHC DHH End Frame CRC " << hex << c << "==" << crc32);
    } else {
      crc_error++;
      B2ERROR("DHHC DHH End Frame CRC " << hex << c << "!=" << crc32);
      error_flag = true;
    }
    return c;
  };
  unsigned int get_words(void) {
    return wordsinevent;
  }
  inline static unsigned int size(void) {
    return 16;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO("DHHC DHH End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
             << " CRC " << hex << crc32 << " (calc) " << calc_crc());
  };
  inline unsigned int get_dhh_id(void) {return (word0.get_misc() >> 4) & 0x3F;};
};

class dhhc_frames {
  void* data;
  unsigned int datasize;
  int type;
  int length;
  bool pad;
public:
  dhhc_frames(void) {
    data = 0;
    datasize = 0;
    type = -1;
    length = 0;
    pad = false;
  };
  int get_type(void) {
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
    type = ((dhhc_frame_header_word0*)data)->get_type();
    length = 0;
    pad = false;
  };
  unsigned int get_evtnr_lo(void) {
    return ((unsigned short*)data)[1];
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[65536 * 16]; /// 1MB
    d = (unsigned char*)data;

    if (length > 65536 * 16) {
      B2WARNING("DHHC Data Frame CRC FAIL bacause of too large packet (>1MB)!");
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
        //         B2INFO("DHH Data Frame CRC: " << hex << c << "==" << crc32);
        B2INFO("DHHC Data Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
               << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " pad " << pad << " len $" << length);
    } else {
      crc_error++;
      if (verbose) {
        B2ERROR("DHHC Data Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
                << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " pad " << pad << " len $" << length);
        /// others would be interessting but possible subjects to access outside of buffer
        /// << " " << * (unsigned int*)(d + length - 2) << " " << * (unsigned int*)(d + length + 0) << " " << * (unsigned int*)(d + length + 2));
        if (length <= 32) {
          for (int i = 0; i < length / 4; i++) {
            B2ERROR("== " << i << "  $" << hex << ((unsigned int*)d)[i]);
          }
        }
      };
      error_flag = true;
    }
    return c;
  };


  unsigned int size(void) {
    unsigned int s = 0;
    switch (get_type()) {
      case DHHC_FRAME_HEADER_DATA_TYPE_DHP_RAW:
        s = ((dhhc_direct_readout_frame_raw*)data)->size();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_DHP_ONS:
      case DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD:
        s = ((dhhc_direct_readout_frame_zsd*)data)->size();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_FCE_RAW:
        B2INFO("Error: FCE type no supported ");
        s = 0;
        error_flag = true;
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_COMMODE:
        s = ((dhhc_commode_frame*)data)->size();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_GHOST:
        s = ((dhhc_ghost_frame*)data)->size();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_DHH_START:
        s = ((dhhc_dhh_start_frame*)data)->size();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_DHH_END:
        s = ((dhhc_dhh_end_frame*)data)->size();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START:
        s = ((dhhc_start_frame*)data)->size();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END:
        s = ((dhhc_end_frame*)data)->size();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_HLTROI:
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

  void write_pedestal(void) {
    B2INFO("Write Pedestal Data - not implemented... !");
  };

};

///******************************************************************
///*********************** Main inpacker code ***********************
///******************************************************************

PXDUnpackerModule::PXDUnpackerModule() :
  Module(),
  m_storeRawHits()
{
  //Set module properties
  setDescription("Unpack Raw PXD Hits");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("HeaderEndianSwap", m_headerEndianSwap, "Swap the endianess of the ONSEN header", true);
  addParam("DHHCmode", m_DHHCmode, "Run in DHHC mode", true);
  addParam("IgnoreDATCON", m_ignoreDATCON, "Ignore missing  DATCON", true);
}

void PXDUnpackerModule::initialize()
{
  //Register output collections
  m_storeRawHits.registerAsPersistent();
  /// actually, later we do not want o store it into output file ...  aside from debugging
  B2INFO("HeaderEndianSwap: " << m_headerEndianSwap);
  B2INFO("DHHCmode: " << m_DHHCmode);
  B2INFO("Ignore(missing)DATCON: " << m_ignoreDATCON);
  ignore_datcon_flag = m_ignoreDATCON;

}

void PXDUnpackerModule::event()
{
  StoreArray<RawPXD> storeRaws;
  StoreArray<RawFTSW> storeFTSW;

  int nRaws = storeRaws.getEntries();
  if (verbose) {
    B2INFO("PXD Unpacker --> RawPXD Objects in event: " << nRaws);
  };

  ftsw_evt_nr = 0;
  ftsw_evt_mask = 0;
  for (auto & it : storeFTSW) {
    ftsw_evt_nr = it.GetEveNo(0);
    ftsw_evt_mask = 0x7FFF;
    B2INFO("PXD Unpacker --> FTSW Event Number: $" << hex << ftsw_evt_nr);
    break;
  }
  for (auto & it : storeRaws) {
    if (verbose) {
      B2INFO("PXD Unpacker --> Unpack Objects: ");
    };
    unpack_event(it);
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
    return;
  }
  unsigned int data[px.size()];
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  memcpy(data, (unsigned int*)px.data(), fullsize);

  /// NEW format
  if (verbose) {
    B2INFO("PXD Unpacker --> data[0]: <-- Magic " << hex << data[0]);
    B2INFO("PXD Unpacker --> data[1]: <-- #Frames " << hex << data[1]);
    if (data[1] >= 1) B2INFO("PXD Unpacker --> data[2]: <-- Frame 1 len " << hex << data[2]);
    if (data[1] >= 2) B2INFO("PXD Unpacker --> data[3]: <-- Frame 2 len " << hex << data[3]);
    if (data[1] >= 3) B2INFO("PXD Unpacker --> data[4]: <-- Frame 3 len " << hex << data[4]);
    if (data[1] >= 4) B2INFO("PXD Unpacker --> data[5]: <-- Frame 4 len " << hex << data[5]);
  };

  unsigned int* tableptr;
  tableptr = &data[2]; // skip header!!!
  if (m_headerEndianSwap) Frames_in_event = ntohl(data[1]); else Frames_in_event = data[1];

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
      exit(0);
    }
    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " << datafullsize << " of fullsize " << fullsize);
//       exit(0);
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
    if (m_DHHCmode) {
      unpack_dhhc_frame(ll + (char*)dataptr, lo, pad, last_wie, last_evtnr, j, Frames_in_event);
    } else {
      B2ERROR("old plain DHH code has been removed!!!");
    }
    ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
  }
//   if (!last_end) B2ERROR("Error: Last Frame is not an END FRAME!");

}

void PXDUnpackerModule::unpack_dhp(void* data, unsigned int len2, unsigned int dhh_first_readout_frame_id_lo, unsigned int dhh_ID, unsigned dhh_DHPport, unsigned dhh_reformat, unsigned short toffset)
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
  dhp_dhh_id       = (dhp_pix[2] >> 2) & 0x3F;
  dhp_dhp_id       =  dhp_pix[2] & 0x03;

  if (printflag) {
    B2INFO("DHP type          |    " << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
    B2INFO("DHP reserved          |    " << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
    B2INFO("DHP DHH ID          |    " << hex << dhp_dhh_id << " ( " << dec << dhp_dhh_id << " ) ");
    B2INFO("DHP DHP ID          |    " << hex << dhp_dhp_id << " ( " << dec << dhp_dhp_id << " ) ");
  }

  if (dhh_ID != dhp_dhh_id) B2ERROR("DHH ID in DHH and DHP header differ ($" << hex << dhh_ID << " != $" << dhp_dhh_id);
  if (dhh_DHPport != dhp_dhp_id) B2ERROR("DHP ID (Chip/Port) in DHH and DHP header differ ($" << hex << dhh_DHPport << " != $" << dhp_dhp_id);

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
          B2ERROR("Error: Pix without Row!!! skip dhp data ");
          dhp_pixel_error++;
          return;
          // return -2;
        } else {
          dhp_row = (dhp_row & 0xFFE) | ((dhp_pix[i] >> 14) & 0x001);
          dhp_col = ((dhp_pix[i] >> 8) & 0x3F) + dhp_offset;
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

          m_storeRawHits.appendNew(dhh_ID, dhp_row, dhp_col, dhp_adc,
                                   dhp_readout_frame_lo - dhh_first_readout_frame_id_lo, toffset, dhp_cm
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

int nr5bits(int i)
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
  static int nr_active_dhp = 0;
  static int mask_active_dhp = 0;
  static int found_mask_active_dhp = 0;

  static unsigned int dhh_first_readout_frame_id_lo = 0;
  static unsigned int dhh_first_offset = 0;
  static unsigned int current_dhh_id = 0xFFFF;


  dhhc_frame_header_word0* hw = (dhhc_frame_header_word0*)data;
  error_flag = false;

  if (Frame_Number == 0) { /// We reset the counters on the first event
    nr_of_dhh_start_frame = 0;
    nr_of_dhh_end_frame = 0;
  }

  dhhc_frames dhhc;
  dhhc.set(data, hw->get_type(), len, pad);
  int s;
  s = dhhc.size();
  if (len != s && s != 0) {
    B2ERROR("Fixed frame type size does not match specs: expect " << len << " != " << s << " (in data) " << pad);
  }

  unsigned int evtnr;

  evtnr = dhhc.get_evtnr_lo();
  if ((evtnr & ftsw_evt_mask) != (ftsw_evt_nr & ftsw_evt_mask)) B2ERROR("Event Numbers do not match for this frame $" << hex << evtnr << "!=$" << ftsw_evt_nr << "(FTSW) mask $" << ftsw_evt_mask);

  int type = dhhc.get_type();

  if (Frame_Number > 0 && Frame_Number < Frames_in_event - 1) {
    if (nr_of_dhh_start_frame != nr_of_dhh_end_frame + 1)
      if (type != DHHC_FRAME_HEADER_DATA_TYPE_HLTROI && type != DHHC_FRAME_HEADER_DATA_TYPE_DHH_START) {
        B2ERROR("Data Frame outside a DHH START/END");
      }
  }

  switch (type) {
    case DHHC_FRAME_HEADER_DATA_TYPE_DHP_RAW: {
      nr_of_frames_counted++;

      ((dhhc_direct_readout_frame_raw*)data)->print();
      if (current_dhh_id != ((dhhc_direct_readout_frame_raw*)data)->get_dhh_id()) B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << ((dhhc_direct_readout_frame_raw*)data)->get_dhh_id());
      dhhc.calc_crc();
      found_mask_active_dhp |= 1 << ((dhhc_direct_readout_frame*)data)->get_dhp_port();

      stat_raw++;
      dhhc.write_pedestal();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHP_ONS:
    case DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD: {
      nr_of_frames_counted++;

      ((dhhc_direct_readout_frame*)data)->print();
      if (current_dhh_id != ((dhhc_direct_readout_frame_raw*)data)->get_dhh_id()) B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << ((dhhc_direct_readout_frame_raw*)data)->get_dhh_id());
      dhhc.calc_crc();
      found_mask_active_dhp |= 1 << ((dhhc_direct_readout_frame*)data)->get_dhp_port();
      stat_zsd++;

      unpack_dhp(data, len - 4,
                 dhh_first_readout_frame_id_lo,
                 ((dhhc_direct_readout_frame*)data)->get_dhh_id(),
                 ((dhhc_direct_readout_frame*)data)->get_dhp_port(),
                 ((dhhc_direct_readout_frame*)data)->get_reformat_flag(),
                 dhh_first_offset);
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_FCE_RAW: {
      nr_of_frames_counted++;

      hw->print();
      if (current_dhh_id != ((dhhc_direct_readout_frame_raw*)data)->get_dhh_id()) B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << ((dhhc_direct_readout_frame_raw*)data)->get_dhh_id());
      dhhc.calc_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_COMMODE: {
      nr_of_frames_counted++;

      hw->print();
      if (current_dhh_id != ((dhhc_commode_frame*)data)->get_dhh_id()) B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << ((dhhc_commode_frame*)data)->get_dhh_id());
      dhhc.calc_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START: {
      bool fake = ((dhhc_start_frame*)data)->is_fake();
      if (fake) {
        B2WARNING("Faked DHHC START Data -> trigger without Data!");
      } else {
        ((dhhc_start_frame*)data)->print();
      }

      last_evtnr = evtnr;
      nr_of_frames_dhhc = ((dhhc_start_frame*)data)->get_dhhc_nr_frames();
      nr_of_frames_counted = 1;
      dhhc.calc_crc();
      stat_start++;

      mask_active_dhh = ((dhhc_start_frame*)data)->get_active_dhh_mask();
      nr_active_dhh = nr5bits(mask_active_dhh);
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHH_START: {
      nr_of_frames_counted++;
      ((dhhc_dhh_start_frame*)data)->print();
      dhh_first_readout_frame_id_lo = ((dhhc_dhh_start_frame*)data)->get_sfnr();
      dhh_first_offset = ((dhhc_dhh_start_frame*)data)->get_toffset();
      current_dhh_id = ((dhhc_dhh_start_frame*)data)->get_dhh_id();
      dhhc.calc_crc();

      if (nr_of_dhh_start_frame != nr_of_dhh_end_frame) B2ERROR("DHHC_FRAME_HEADER_DATA_TYPE_DHH_START without DHHC_FRAME_HEADER_DATA_TYPE_DHH_END");
      nr_of_dhh_start_frame++;

      found_mask_active_dhp = 0;
      mask_active_dhp = ((dhhc_dhh_start_frame*)data)->get_active_dhp_mask();
      nr_active_dhp = nr5bits(mask_active_dhp);
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_GHOST:
      nr_of_frames_counted++;
      ((dhhc_ghost_frame*)data)->print();
      if (current_dhh_id != ((dhhc_ghost_frame*)data)->get_dhh_id()) B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << ((dhhc_ghost_frame*)data)->get_dhh_id());
      /// Attention: Firmware might be changed such, that ghostframe come for all DHPs, not only active ones...
      found_mask_active_dhp |= 1 << ((dhhc_ghost_frame*)data)->get_dhp_port();
      dhhc.calc_crc();
      stat_ghost++;

      break;
    case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END: {
      bool fake = ((dhhc_end_frame*)data)->is_fake();
      nr_of_frames_counted++;
      if (fake) {
        B2WARNING("Faked DHHC END Data -> trigger without Data!");
      } else {
        ((dhhc_end_frame*)data)->print();
      }
      stat_end++;

      if (!fake) {
        if (nr_of_frames_counted != nr_of_frames_dhhc)  B2ERROR("Number of DHHC Frames in Header " << nr_of_frames_dhhc << " != " << nr_of_frames_counted << " Counted");
      }
      if (!fake) {
        int w;
        w = ((dhhc_end_frame*)data)->get_words() * 2;
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
      dhhc.calc_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHH_END: {
      nr_of_frames_counted++;
      ((dhhc_dhh_end_frame*)data)->print();
      if (current_dhh_id != ((dhhc_dhh_end_frame*)data)->get_dhh_id()) B2ERROR("DHH ID from DHH Start and this frame do not match $" << hex << current_dhh_id << " != $" << ((dhhc_dhh_end_frame*)data)->get_dhh_id());
      current_dhh_id = 0xFFFF;
      dhhc.calc_crc();
      if (found_mask_active_dhp != mask_active_dhp) B2ERROR("DHH_END: DHP active mask $" << hex << mask_active_dhp << " != $" << hex << found_mask_active_dhp << " mask of found dhp/ghost frames");
      nr_of_dhh_end_frame++;
      if (nr_of_dhh_start_frame != nr_of_dhh_end_frame) B2ERROR("DHHC_FRAME_HEADER_DATA_TYPE_DHH_END without DHHC_FRAME_HEADER_DATA_TYPE_DHH_START");
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_HLTROI:
      //nr_of_frames_counted++;/// DO NOT COUNT!!!!
      //((dhhc_onsen_frame*)data)->set_length(len - 4);
      ((dhhc_onsen_frame*)data)->print();
      ((dhhc_onsen_frame*)data)->calc_crc(len - 4); /// CRC is without the DHHC header
      dhhc.calc_crc();
      break;
    default:
      B2ERROR("UNKNOWN DHHC frame type");
      type_error++;
      hw->print();
      error_flag = true;
      break;
  }

  if (evtnr != last_evtnr) {
    B2ERROR("Error: Event Nr $" << hex << evtnr << " != $" << last_evtnr);
    evtnr_error++;
    error_flag = true;
  }

  if (Frame_Number == 0) {
    /// Check that DHHC Start is first Frame
    if (type != DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START) B2ERROR("First frame is not a DHHC start of subevent frame in Event Nr " << evtnr);
  } else { // (Frame_Number != 0 &&
    /// Check that there is no other DHHC Start
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_DHHC_START) B2ERROR("More than one DHHC start of subevent frame in frame in Event Nr " << evtnr)
    }

  if (Frame_Number == Frames_in_event - 1) {
    /// Check that DHHC End is last Frame
    if (type != DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END) B2ERROR("Last frame is not a DHHC end of subevent frame in Event Nr " << evtnr);

    /// As we now have processed the whole event, we can do some more consistency checks!
    if (nr_of_dhh_start_frame != nr_of_dhh_end_frame || nr_of_dhh_start_frame != nr_active_dhh)
      B2ERROR("The number of DHH Start/End does not match the number of active DHH in DHHC Header! Header: " << nr_active_dhh << " Start: " << nr_of_dhh_start_frame << " End: " << nr_of_dhh_end_frame << " Mask: $" << hex << mask_active_dhh << " in Event Nr " << evtnr);

  } else { //  (Frame_Number != Frames_in_event - 1 &&
    /// Check that there is no other DHHC End
    if (type == DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END) B2ERROR("More than one DHHC end of subevent frame in frame in Event Nr " << evtnr)
    }

  /// Check that (if there is at least one active DHH) the second Frame is DHH Start, actually this is redundant if the other checks work
  if (Frame_Number == 1 && nr_active_dhh != 0 && type != DHHC_FRAME_HEADER_DATA_TYPE_DHH_START) {
    B2ERROR("Second frame is not a DHH start frame in Event Nr " << evtnr)
  }

  last_wie += len;

}



