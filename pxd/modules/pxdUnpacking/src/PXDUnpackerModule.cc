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
#include <pxd/dataobjects/RawPXD.h>

// for htonl
#include <arpa/inet.h>

#include <boost/foreach.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

#define DHP_FRAME_HEADER_DATA_TYPE_RAW  0x0
#define DHP_FRAME_HEADER_DATA_TYPE_ZSD  0x5

// DHH frames
#define DHH_FRAME_HEADER_DATA_TYPE_DHP_RAW  0x0
#define DHH_FRAME_HEADER_DATA_TYPE_DHP_ZSD  0x5
#define DHH_FRAME_HEADER_DATA_TYPE_DCE_RAW  0x1
#define DHH_FRAME_HEADER_DATA_TYPE_COMMODE  0x6
#define DHH_FRAME_HEADER_DATA_TYPE_EVT_FRM  0x3
#define DHH_FRAME_HEADER_DATA_TYPE_GHOST    0x2
#define DHH_FRAME_HEADER_DATA_TYPE_END_FRM  0x4
// Onsen processed data
#define DHH_FRAME_HEADER_DATA_TYPE_HLTROI   0x7

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
unsigned long long evt_counter = 0;
unsigned int stat_start = 0, stat_end = 0, stat_ghost = 0, stat_raw = 0, stat_zsd = 0;
unsigned int dhp_size_error = 0, dhp_pixel_error = 0, dhp_warning = 0;
bool verbose = true;
bool error_flag = false;

char* dhh_type_name[8] = {
  (char*)"DHP_RAW",
  (char*)"DCE_RAW",
  (char*)"GHOST  ",
  (char*)"EVT_FRM",
  (char*)"END_FRM",
  (char*)"DHP_ZSD",
  (char*)"COMMODE",
  (char*)"HLTROI ",
};

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
  int get_type(void) {
    return (data >> 11) & 0xF;
  };
  int get_err(void) {
    return (data >> 15) & 0x1;
  };
  void print(void) {
    if (verbose)
      B2INFO(" DHH FRAME TYP " << hex << get_type() << " -> " << dhhc_type_name[get_type()] << " ERR " << get_err() << " data " << data);
  };
};

class dhh_frame_header_word0 {
public:
  unsigned short data;
  dhh_frame_header_word0(unsigned int error_flag = 0, unsigned int data_typ = 0, unsigned int frame_nr = 0, unsigned int dhh_id = 0, unsigned int chip_id_ref = 0) {
    data = ((error_flag & 0x1) << 15) | ((data_typ & 0x7) << 12) | ((frame_nr & 0xF) << 8) | ((dhh_id & 0x3F) << 2) | (chip_id_ref & 0x3); //data mit variablen aufgefüllt
  };
  int get_chipid(void) {
    return (data) & 0x3;
  };
  int get_dhhid(void) {
    return (data >> 2) & 0x3F;
  };
  int get_fnr(void) {
    return (data >> 8) & 0xF;
  };
  int get_type(void) {
    return (data >> 12) & 0x7;
  };
  int get_err(void) {
    return (data >> 15) & 0x1;
  };
  void set_framenr(unsigned int fn) {
    data &= 0xF0FF;
    data |= ((fn & 0xF) << 8);
  };
  void print(void) {
    if (verbose)
      B2INFO(" DHH FRAME TYP " << hex << get_type() << " -> " << dhh_type_name[get_type()] << " FNR " << get_fnr()
             << " DDHID " << get_dhhid() << " CHIP " << get_chipid() << " ERR " << get_err() << " data " << data);
  };
};

class dhp_frame {
  dhh_frame_header_word0 word0;
  unsigned short frame_id;
};

class dhh_event_frame {
public:
  dhh_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned short trigger_nr_hi;
  unsigned short time_tag_lo;
  unsigned short time_tag_hi;
  unsigned short sfnr_offset;
  unsigned int crc32;
  dhh_event_frame(unsigned int time_tag = 0, unsigned int trigger_nr = 0, unsigned int frame_nr = 0, unsigned int dhh_id = 0, unsigned int chip_id_ref = 0): word0(0, DHH_FRAME_HEADER_DATA_TYPE_EVT_FRM, frame_nr, dhh_id, chip_id_ref) {
    trigger_nr_lo = trigger_nr & 0xFFFF;
    trigger_nr_hi = trigger_nr >> 16;
    time_tag_lo = time_tag & 0xFFFF;
    time_tag_hi = time_tag >> 16;
    sfnr_offset = 0;
    set_crc(calc_crc());
  };
  unsigned short get_evtnr(void) {
    return trigger_nr_lo;
  };
  unsigned short get_sfnr(void) {
    return (sfnr_offset >> 10) & 0x3F;
  };
  unsigned short get_toffset(void) {
    return sfnr_offset & 0x3FF;
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[size() * 4];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 1) * 4; k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 1) * 4);
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHH Event Frame CRC " << hex << c << " == " << hex << crc32);
    } else {
      crc_error++;
      B2ERROR(" DHH Event Frame CRC FAIL " << hex << c << " != " << hex << crc32);
      error_flag = true;
    }
    return c;
  };
  void set_crc(unsigned int c) {
    crc32 = c;
  };
  bool is_fake(void) {
    if (word0.data != 0x3000) return false;
    if (trigger_nr_lo != 0) return false;
    if (trigger_nr_hi != 0) return false;
    if (time_tag_lo != 0) return false;
    if (time_tag_hi != 0) return false;
    if (sfnr_offset != 0) return false;
    //if(crc32!=0x87654321) return false;
    return true;
  };
  inline static unsigned int size(void) {
    return 4;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO(" DHH Event Frame TNRLO " << hex << trigger_nr_lo << " TNRHI " << hex << trigger_nr_hi << " TTLO " << hex << time_tag_lo
             << " TTHI " << hex << time_tag_hi << " SFNR " << hex << ((sfnr_offset >> 10) & 0x3F) << " OFF " << hex << (sfnr_offset & 0x3FF)
             << " CRC " << hex << crc32 << " (calc)" << calc_crc());
  };
};

class dhh_commode_frame {
public:
  dhh_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned short data[96];
  unsigned int crc32;

  dhh_commode_frame(unsigned int trigger_nr = 0, unsigned int frame_nr = 0, unsigned int dhh_id = 0, unsigned int chip_id_ref = 0):
    word0(0, DHH_FRAME_HEADER_DATA_TYPE_COMMODE, frame_nr, dhh_id, chip_id_ref) {
    trigger_nr_lo = trigger_nr & 0xFFFF;
    for (int i = 0; i < 96; i++) data[i] = i;
    set_crc(calc_crc());
  };
  unsigned int calc_crc(void) {
//     unsigned int* d;
    unsigned int c = 0;
//     d = (unsigned int*)this;

    if (verbose)
      B2INFO(" DHH Common Frame CRC not implemented here... ");
//     for (unsigned int i = 0; i < size(); i++) if (verbose) B2INFO(" d " << hex << d[i]);
    return c;
  };
  void set_crc(unsigned int c) {
    crc32 = c;
  };
  inline static unsigned int size(void) {
    return 2 + 96 / 2;
  };
};

class dhh_direct_readout_frame {
public:
  dhh_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  dhp_frame data;
  unsigned int crc32;
  dhh_direct_readout_frame(unsigned int type, unsigned int trigger_nr = 0, unsigned int frame_nr = 0, unsigned int dhh_id = 0, unsigned int chip_id_ref = 0):
    word0(0, type, frame_nr, dhh_id, chip_id_ref) {
    trigger_nr_lo = trigger_nr & 0xFFFF;
    set_crc(calc_crc());
  };
  unsigned int calc_crc(void) {

    unsigned int c = 0;

    if (verbose)
      B2INFO(" CRC for data frames not implemented here... ");
    return c;
  };
  void set_crc(unsigned int c) {
    crc32 = c;
  };
  inline static unsigned int size(void) {
    return 0;//2;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO(" DHH Direct Readout (Raw|ZSD) Frame TNRLO " << hex << trigger_nr_lo << " CRC " << calc_crc());
  };
};

class dhh_direct_readout_frame_raw : public dhh_direct_readout_frame {
public:
  dhh_direct_readout_frame_raw(unsigned int trigger_nr = 0, unsigned int frame_nr = 0, unsigned int dhh_id = 0, unsigned int chip_id_ref = 0):
    dhh_direct_readout_frame(DHH_FRAME_HEADER_DATA_TYPE_DHP_RAW, trigger_nr, frame_nr, dhh_id, chip_id_ref) {
  };
  void print(void) {
    dhh_direct_readout_frame::print();
  };
};

class dhh_direct_readout_frame_zsd : public dhh_direct_readout_frame {
public:
  dhh_direct_readout_frame_zsd(unsigned int trigger_nr = 0, unsigned int frame_nr = 0, unsigned int dhh_id = 0, unsigned int chip_id_ref = 0):
    dhh_direct_readout_frame(DHH_FRAME_HEADER_DATA_TYPE_DHP_ZSD, trigger_nr, frame_nr, dhh_id, chip_id_ref) {
  };
};

class dhh_onsen_frame {
  dhh_frame_header_word0 word0;/// mainly empty
  unsigned short trignr0;// not used
  unsigned int magic1;
  unsigned int trignr1;
  unsigned int magic2;
  unsigned int trignr2;
  /// plus n* ROIs (64 bit)
  /// plus checksum 32bit
  unsigned int length;/// not part
public:
  dhh_onsen_frame(void) {};
  void set_length(unsigned int l) {length = l;};
  void print(void) {
    if (magic1 == 0x56781234) {
      B2WARNING("DAVID ROI Framer Error");
      memmove(&magic1, &trignr1, length - 4);
      length -= 4;
    }
    word0.print();
    if (verbose)
      B2INFO("DHH HLT/ROI Frame " << hex << trignr1 << " ," << trignr2);
    if ((magic1 & 0xFFFF) != 0xCAFE) B2ERROR("DHH HLT/ROI Magic 1 error $" << hex << magic1);
    if ((magic2 & 0xFFFF) != 0xCAFE) B2ERROR("DHH HLT/ROI Magic 2 error $" << hex << magic2);
    if (magic2 == 0x0000CAFE && trignr2 == 0x00000000) {
      B2WARNING("DHH HLT/ROI Frame: No DATCON data " << hex << trignr1 << "!=$" << trignr2);
    } else {
      if (trignr1 != trignr2) B2ERROR("DHH HLT/ROI Frame Trigger Nr Mismatch $" << hex << trignr1 << "!=$" << trignr2);
    }
  };
  inline static unsigned int size(void) {
    return 0;//2;
  };
};

class dhh_ghost_frame {
public:
  dhh_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int crc32;

  dhh_ghost_frame(unsigned int trigger_nr = 0, unsigned int frame_nr = 0, unsigned int dhh_id = 0, unsigned int chip_id_ref = 0):
    word0(1, DHH_FRAME_HEADER_DATA_TYPE_GHOST, frame_nr, dhh_id, chip_id_ref) {
    trigger_nr_lo = trigger_nr & 0xFFFF;
    set_crc(calc_crc());
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[size() * 4];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 1) * 4; k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 1) * 4);
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHH Ghost Frame CRC " << hex << c << " == " << crc32);
    } else {
      crc_error++;
      B2ERROR(" DHH Ghost Frame CRC FAIL " << hex << c << " != " << crc32);
      error_flag = true;
    }
    return c;
  };
  void set_crc(unsigned int c) {
    crc32 = c;
  };
  inline static unsigned int size(void) {
    return 2;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO(" DHH Ghost Frame TNRLO " << hex << trigger_nr_lo << " CRC "  << crc32 << " (calc) "  << calc_crc());
  };
};

class dhh_end_event_frame {
public:
  dhh_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int wordsinevent;
  unsigned int errorinfo;
  unsigned int crc32;

  dhh_end_event_frame(unsigned int wie = 0, unsigned int eo = 0, unsigned int trigger_nr = 0, unsigned int frame_nr = 0, unsigned int dhh_id = 0, unsigned int chip_id_ref = 0):
    word0(1, DHH_FRAME_HEADER_DATA_TYPE_GHOST, frame_nr, dhh_id, chip_id_ref) {
    trigger_nr_lo = trigger_nr & 0xFFFF;
    wordsinevent = wie;
    errorinfo = eo;
    set_crc(calc_crc());
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[size() * 4];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 1) * 4; k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 1) * 4);
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHH End Frame CRC " << hex << c << "==" << crc32);
    } else {
      crc_error++;
      B2ERROR(" DHH End Frame CRC " << hex << c << "!=" << crc32);
      error_flag = true;
    }
    return c;
  };
  void set_crc(unsigned int c) {
    crc32 = c;
  };
  void set_words(unsigned int wie) {
    wordsinevent = wie;
  };
  unsigned int get_words(void) {
    return wordsinevent;
  }
  void set_framenr(unsigned int fn) {
    word0.set_framenr(fn);
  };
  inline static unsigned int size(void) {
    return 4;
  };
  bool is_fake(void) {
    if (word0.data != 0xC100) return false;
    if (trigger_nr_lo != 0) return false;
    if (wordsinevent != 0) return false;
    if (errorinfo != 0) return false;
    //if(crc32!=0x87654321) return false;
    return true;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO("DHH End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
             << " CRC " << hex << crc32 << " (calc) " << calc_crc());
  };
};

class dhh_frames {
  void* data;
  unsigned int datasize;
  int type;
  int length;
  bool pad;
public:
  dhh_frames(void) {
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
    type = ((dhh_frame_header_word0*)data)->get_type();
    length = 0;
    pad = false;
  };
  unsigned int get_evtnr(void) {
    return ((unsigned short*)data)[1];
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[65536 * 16]; /// 1MB
    d = (unsigned char*)data;

    if (length > 65536 * 16) {
      B2WARNING(" DHH Data Frame CRC FAIL bacause of too large packet (>1MB)!");
    } else {
      for (int k = 0; k < length - 4; k += 2) {
        crcbuffer[k] = d[k + 1];
        crcbuffer[k + 1] = d[k];
      }
      bocrc.process_bytes(crcbuffer, length - 4);
    }
//     unsigned char* d;
//     dhh_crc_32_type bocrc;
//     d = (unsigned char*)data;
//     for (int k = 0; k < length - 4; k++) {
//       bocrc.process_byte(((const unsigned char*)d)[(k ^ 1)]);
//     }
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
//         B2INFO(" DHH Data Frame CRC: " << hex << c << "==" << crc32);
        B2INFO(" DHH Data Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
               << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " pad " << pad << " len $" << length);
    } else {
      crc_error++;
      if (verbose) {
        B2ERROR(" DHH Data Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
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
      case DHH_FRAME_HEADER_DATA_TYPE_DHP_RAW:
        s = ((dhh_direct_readout_frame_raw*)data)->size();
        break;
      case DHH_FRAME_HEADER_DATA_TYPE_DHP_ZSD:
        s = ((dhh_direct_readout_frame_zsd*)data)->size();
        break;
      case DHH_FRAME_HEADER_DATA_TYPE_DCE_RAW:
        B2INFO(" Error: DCE type no supported ");
        s = 0;
        error_flag = true;
        break;
      case DHH_FRAME_HEADER_DATA_TYPE_COMMODE:
        s = ((dhh_commode_frame*)data)->size();
        break;
      case DHH_FRAME_HEADER_DATA_TYPE_EVT_FRM:
        s = ((dhh_event_frame*)data)->size();
        break;
      case DHH_FRAME_HEADER_DATA_TYPE_GHOST:
        s = ((dhh_ghost_frame*)data)->size();
        break;
      case DHH_FRAME_HEADER_DATA_TYPE_END_FRM:
        s = ((dhh_end_event_frame*)data)->size();
        break;
      case DHH_FRAME_HEADER_DATA_TYPE_HLTROI:
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
    B2INFO(" Write Pedestal Data - not implemented... !");
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
  unsigned short nr_words_in_frame;
  unsigned int crc32;

//    dhhc_start_frame(unsigned int time_tag = 0, unsigned int trigger_nr = 0, unsigned int depend = 0): word0(0, DHH_FRAME_HEADER_DATA_TYPE_EVT_FRM, depend) {
//    };
  unsigned short get_evtnr(void) {
    return trigger_nr_lo;
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[size() * 4];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 1) * 4; k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 1) * 4);
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHHC Start Frame CRC " << hex << c << " == " << hex << crc32);
    } else {
      crc_error++;
      B2ERROR(" DHHC Start Frame CRC FAIL " << hex << c << " != " << hex << crc32);
      error_flag = true;
    }
    return c;
  };
  bool is_fake(void) {
    /*      if (word0.data != 0x3000) return false;
          if (trigger_nr_lo != 0) return false;
          if (trigger_nr_hi != 0) return false;
          if (time_tag_lo != 0) return false;
          if (time_tag_hi != 0) return false;
          if (sfnr_offset != 0) return false;
          //if(crc32!=0x87654321) return false;
          return true;
          */
    return false;
  };
  inline static unsigned int size(void) {
    return 4;// 9 words???
  };
  void print(void) {
    word0.print();
    /*      if (verbose)
             B2INFO(" DHHC Start Frame TNRLO " << hex << trigger_nr_lo << " TNRHI " << hex << trigger_nr_hi << " TTLO " << hex << time_tag_lo
             << " TTHI " << hex << time_tag_hi << " SFNR " << hex << ((sfnr_offset >> 10) & 0x3F) << " OFF " << hex << (sfnr_offset & 0x3FF)
             << " CRC " << hex << crc32 << " (calc)" << calc_crc());*/
  };
};

class dhhc_dhh_start_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  //unsigned short trigger_nr_hi;
  unsigned short dhh_time_tag_lo;
  unsigned short dhh_time_tag_hi;
  unsigned short sfnr_offset;
  unsigned int crc32;

//    dhhc_dhh_start_frame(unsigned int time_tag = 0, unsigned int trigger_nr = 0, unsigned int depend = 0): word0(0, DHH_FRAME_HEADER_DATA_TYPE_EVT_FRM, depend) {
//    };
  unsigned short get_evtnr(void) {
    return trigger_nr_lo;
  };
  unsigned short get_sfnr(void) {// last DHP fraem before trigger
    return (sfnr_offset >> 10) & 0x3F;
  };
  unsigned short get_toffset(void) {// and trigger row offset
    return sfnr_offset & 0x3FF;
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[size() * 4];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 1) * 4; k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 1) * 4);
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHHC DHH Start Frame CRC " << hex << c << " == " << hex << crc32);
    } else {
      crc_error++;
      B2ERROR(" DHHC DHH Start CRC FAIL " << hex << c << " != " << hex << crc32);
      error_flag = true;
    }
    return c;
  };
  bool is_fake(void) {
    /*if (word0.data != 0x3000) return false;
    if (trigger_nr_lo != 0) return false;
    if (trigger_nr_hi != 0) return false;
    if (time_tag_lo != 0) return false;
    if (time_tag_hi != 0) return false;
    if (sfnr_offset != 0) return false;
    //if(crc32!=0x87654321) return false;
    return true;*/
    return false;
  };
  inline static unsigned int size(void) {
    return 4;// 7 words
  };
  void print(void) {
    word0.print();
    /*      if (verbose)
             B2INFO(" DHHC Event Frame TNRLO " << hex << trigger_nr_lo << " TNRHI " << hex << trigger_nr_hi << " TTLO " << hex << time_tag_lo
             << " TTHI " << hex << time_tag_hi << " SFNR " << hex << ((sfnr_offset >> 10) & 0x3F) << " OFF " << hex << (sfnr_offset & 0x3FF)
             << " CRC " << hex << crc32 << " (calc)" << calc_crc());*/
  };
};

class dhhc_commode_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned short data[96];
  unsigned int crc32;

//    dhhc_commode_frame(unsigned int trigger_nr = 0, unsigned int dhhc_id = 0, unsigned int depend = 0):
//    word0(0, DHH_FRAME_HEADER_DATA_TYPE_COMMODE, depend) {
//    };
  unsigned int calc_crc(void) {
    //     unsigned int* d;
    unsigned int c = 0;
    //     d = (unsigned int*)this;

    if (verbose)
      B2INFO(" DHHC Common Frame CRC not implemented here... ");
    //     for (unsigned int i = 0; i < size(); i++) if (verbose) B2INFO(" d " << hex << d[i]);
    return c;
  };
  inline static unsigned int size(void) {
    return 2 + 96 / 2;
  };
};

class dhhc_direct_readout_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  dhp_frame data;
  unsigned int crc32;
//    dhhc_direct_readout_frame(unsigned int type, unsigned int trigger_nr = 0, unsigned int depend = 0):
//    word0(0, type, depend) {
//    };
  unsigned int calc_crc(void) {

    unsigned int c = 0;

    if (verbose)
      B2INFO(" CRC for data frames not implemented here... ");
    return c;
  };
  inline static unsigned int size(void) {
    return 0;//2;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO(" DHHC Direct Readout (Raw|ZSD) Frame TNRLO " << hex << trigger_nr_lo << " CRC " << calc_crc());
  };
};

class dhhc_direct_readout_frame_raw : public dhhc_direct_readout_frame {
public:
//    dhhc_direct_readout_frame_raw(unsigned int trigger_nr = 0, unsigned int depend = 0):
//    dhhc_direct_readout_frame(DHH_FRAME_HEADER_DATA_TYPE_DHP_RAW, trigger_nr, depend) {
//    };
//    void print(void) {
//       dhhc_direct_readout_frame::print();
//    };
};

class dhhc_direct_readout_frame_zsd : public dhhc_direct_readout_frame {
public:
//    dhhc_direct_readout_frame_zsd(unsigned int trigger_nr = 0, unsigned int depend = 0):
//    dhhc_direct_readout_frame(DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD, trigger_nr, depend) {
//    };
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
  unsigned int length;/// not part
public:
//    dhhc_onsen_frame(void) {};
  void set_length(unsigned int l) {length = l;};
  void print(void) {
    if (magic1 == 0x56781234) {
      B2WARNING("DAVID ROI Framer Error");
      memmove(&magic1, &trignr1, length - 4);
      length -= 4;
    }
    word0.print();
    if (verbose)
      B2INFO("DHHC HLT/ROI Frame " << hex << trignr1 << " ," << trignr2);
    if ((magic1 & 0xFFFF) != 0xCAFE) B2ERROR("DHHC HLT/ROI Magic 1 error $" << hex << magic1);
    if ((magic2 & 0xFFFF) != 0xCAFE) B2ERROR("DHHC HLT/ROI Magic 2 error $" << hex << magic2);
    if (magic2 == 0x0000CAFE && trignr2 == 0x00000000) {
      B2WARNING("DHHC HLT/ROI Frame: No DATCON data " << hex << trignr1 << "!=$" << trignr2);
    } else {
      if (trignr1 != trignr2) B2ERROR("DHHC HLT/ROI Frame Trigger Nr Mismatch $" << hex << trignr1 << "!=$" << trignr2);
    }
  };
  inline static unsigned int size(void) {
    return 0;//2;
  };
};

class dhhc_ghost_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int crc32;

//    dhhc_ghost_frame(unsigned int trigger_nr = 0, unsigned int depend = 0):
//    word0(1, DHH_FRAME_HEADER_DATA_TYPE_GHOST, depend) {
//    };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[size() * 4];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 1) * 4; k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 1) * 4);
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHHC Ghost Frame CRC " << hex << c << " == " << crc32);
    } else {
      crc_error++;
      B2ERROR(" DHHC Ghost Frame CRC FAIL " << hex << c << " != " << crc32);
      error_flag = true;
    }
    return c;
  };
  inline static unsigned int size(void) {
    return 2;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO(" DHHC Ghost Frame TNRLO " << hex << trigger_nr_lo << " CRC "  << crc32 << " (calc) "  << calc_crc());
  };
};

class dhhc_end_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int wordsinevent;
  unsigned int errorinfo;
  unsigned int crc32;

//    dhhc_end_frame(unsigned int wie = 0, unsigned int eo = 0, unsigned int trigger_nr = 0, unsigned int depend = 0):
//    word0(0, DHHC_FRAME_HEADER_DATA_TYPE_DHH_END, depend) {
//    };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[size() * 4];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 1) * 4; k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 1) * 4);
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHHC End Frame CRC " << hex << c << "==" << crc32);
    } else {
      crc_error++;
      B2ERROR(" DHHC End Frame CRC " << hex << c << "!=" << crc32);
      error_flag = true;
    }
    return c;
  };
  unsigned int get_words(void) {
    return wordsinevent;
  }
  inline static unsigned int size(void) {
    return 4;
  };
  bool is_fake(void) {
    /*      if (word0.data != 0xC100) return false;
          if (trigger_nr_lo != 0) return false;
          if (wordsinevent != 0) return false;
          if (errorinfo != 0) return false;
          //if(crc32!=0x87654321) return false;
          return true;*/
    return false;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO("DHHC End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
             << " CRC " << hex << crc32 << " (calc) " << calc_crc());
  };
};

class dhhc_dhh_end_frame {
public:
  dhhc_frame_header_word0 word0;
  unsigned short trigger_nr_lo;
  unsigned int wordsinevent;
  unsigned int errorinfo;
  unsigned int crc32;

//    dhhc_dhh_end_frame(unsigned int wie = 0, unsigned int eo = 0, unsigned int trigger_nr = 0, unsigned int depend = 0):
//    word0(0, DHHC_FRAME_HEADER_DATA_TYPE_DHH_END, depend) {
//    };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[size() * 4];
    d = (unsigned char*)this;

    for (unsigned int k = 0; k < (size() - 1) * 4; k += 2) {
      crcbuffer[k] = d[k + 1];
      crcbuffer[k + 1] = d[k];
    }
    bocrc.process_bytes(crcbuffer, (size() - 1) * 4);
    unsigned int c;
    c = htonl(bocrc.checksum());
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHHC DHH End Frame CRC " << hex << c << "==" << crc32);
    } else {
      crc_error++;
      B2ERROR(" DHHC DHH End Frame CRC " << hex << c << "!=" << crc32);
      error_flag = true;
    }
    return c;
  };
  unsigned int get_words(void) {
    return wordsinevent;
  }
  inline static unsigned int size(void) {
    return 4;
  };
  bool is_fake(void) {
    /*      if (word0.data != 0xC100) return false;
          if (trigger_nr_lo != 0) return false;
          if (wordsinevent != 0) return false;
          if (errorinfo != 0) return false;
          //if(crc32!=0x87654321) return false;
          return true;*/
    return false;
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO("DHHC DHH End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
             << " CRC " << hex << crc32 << " (calc) " << calc_crc());
  };
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
  unsigned int get_evtnr(void) {
    return ((unsigned short*)data)[1];
  };
  unsigned int calc_crc(void) {
    unsigned char* d;
    dhh_crc_32_type bocrc;
    char crcbuffer[65536 * 16]; /// 1MB
    d = (unsigned char*)data;

    if (length > 65536 * 16) {
      B2WARNING(" DHHC Data Frame CRC FAIL bacause of too large packet (>1MB)!");
    } else {
      for (int k = 0; k < length - 4; k += 2) {
        crcbuffer[k] = d[k + 1];
        crcbuffer[k + 1] = d[k];
      }
      bocrc.process_bytes(crcbuffer, length - 4);
    }
    //     unsigned char* d;
    //     dhh_crc_32_type bocrc;
    //     d = (unsigned char*)data;
    //     for (int k = 0; k < length - 4; k++) {
    //       bocrc.process_byte(((const unsigned char*)d)[(k ^ 1)]);
    //     }
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
        //         B2INFO(" DHH Data Frame CRC: " << hex << c << "==" << crc32);
        B2INFO(" DHHC Data Frame CRC OK: " << hex << c << "==" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
               << * (unsigned int*)(d + length - 6) << " " << * (unsigned int*)(d + length - 4) << " pad " << pad << " len $" << length);
    } else {
      crc_error++;
      if (verbose) {
        B2ERROR(" DHHC Data Frame CRC FAIL: " << hex << c << "!=" << crc32 << " data "  << * (unsigned int*)(d + length - 8) << " "
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
      case DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD:
        s = ((dhhc_direct_readout_frame_zsd*)data)->size();
        break;
      case DHHC_FRAME_HEADER_DATA_TYPE_FCE_RAW:
        B2INFO(" Error: FCE type no supported ");
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
    B2INFO(" Write Pedestal Data - not implemented... !");
  };

};

PXDUnpackerModule::PXDUnpackerModule() :
  Module(),
  m_storeRawHits()
{
  //Set module properties
  setDescription("Unpack Raw PXD Hits");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("HeaderEndianSwap", m_headerEndianSwap, "Swap the endianess of the ONSEN header", false);
  addParam("DHHCmode", m_DHHCmode, "Run in DHHC mode", false);
}

void PXDUnpackerModule::initialize()
{
  //Register output collections
  m_storeRawHits.registerAsPersistent();
  /// actually, later we do not want o store it into output file ...  aside from debugging
  B2INFO("HeaderEndianSwap: " << m_headerEndianSwap);
  B2INFO("DHHCmode: " << m_DHHCmode);

}

void PXDUnpackerModule::event()
{
  StoreArray<RawPXD> storeRaws;

  int nRaws = storeRaws.getEntries();
  if (verbose) {
    B2INFO(" PXD Unpacker --> RawPXD Objects in event: " << nRaws);
  };

  for (auto & it : storeRaws) {
    if (verbose) {
      B2INFO(" PXD Unpacker --> Unpack Objects: ");
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
  int last_wie = 0, last_framenr = 0, last_start = 0, last_end = 1;
  static unsigned int last_evtnr = 0;
  int Frames_per_event;
  int fullsize;
  int datafullsize;

  unsigned int data[px.size()];
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  memcpy(data, (unsigned int*)px.data(), fullsize);

  /// NEW format
  if (verbose) {
    B2INFO(" PXD Unpacker --> data[0]: <-- Magic " << hex << data[0]);
    B2INFO(" PXD Unpacker --> data[1]: <-- #Frames " << hex << data[1]);
    if (data[1] >= 1) B2INFO(" PXD Unpacker --> data[2]: <-- Frame 1 len " << hex << data[2]);
    if (data[1] >= 2) B2INFO(" PXD Unpacker --> data[3]: <-- Frame 2 len " << hex << data[3]);
    if (data[1] >= 3) B2INFO(" PXD Unpacker --> data[4]: <-- Frame 3 len " << hex << data[4]);
    if (data[1] >= 4) B2INFO(" PXD Unpacker --> data[5]: <-- Frame 4 len " << hex << data[5]);
  };

  unsigned int* tableptr;
  tableptr = &data[2]; // skip header!!!
  if (m_headerEndianSwap) Frames_per_event = ntohl(data[1]); else Frames_per_event = data[1];

  unsigned int* dataptr;
  dataptr = &tableptr[Frames_per_event];
  datafullsize = fullsize - 2 * 4 - Frames_per_event * 4; // minus header, minus table

  int ll = 0; // Offset in dataptr in bytes
  for (int j = 0; j < Frames_per_event; j++) {
    int lo;/// len of frame in bytes
    bool pad;
    if (m_headerEndianSwap) lo = ntohl(tableptr[j]); else lo = tableptr[j];
    if (lo <= 0) {
      B2ERROR(" size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      exit(0);
    }
    if (ll + lo > datafullsize) {
      B2ERROR(" frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " << datafullsize << " of fullsize " << fullsize);
//       exit(0);
      return;
    }
    if (lo & 0x3) {
      pad = true;
      if (verbose)
        B2INFO(" Data with not MOD 4 length " << " ( " << lo << " ) ");
    } else {
      pad = false;
    }

    B2INFO(" unpack DHH frame: " << j << " with size " << lo << " at byte offset in dataptr " << ll);
    endian_swap_frame((unsigned short*)(ll + (char*)dataptr), lo);
    if (m_DHHCmode) {
      unpack_dhhc_frame(ll + (char*)dataptr, lo, pad, last_wie, last_start, last_end, last_evtnr);
    } else {
      unpack_frame(ll + (char*)dataptr, lo, pad, last_framenr, last_wie, last_start, last_end, last_evtnr);
    }
    ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
  }
  if (!last_end) B2ERROR("Error: Last Frame is not an END FRAME!");

}

void PXDUnpackerModule::unpack_dhp(void* data, unsigned int len2, unsigned int dhh_first_frame_id_lo, unsigned int dhh_ID, unsigned short toffset)
{
  unsigned int anzahl = len2 / 2; // len2 in bytes!!!
  bool commode = false;
  bool printflag = false;
  unsigned short* dhp_pix = (unsigned short*)data;

  int dhh_header_id_I = 0;
  int dhh_header_error = 0;
  int dhh_header_type = 0;
  int dhh_header_dhh = 0;

  int dhp_header_id_I = 0;
  int dhp_header_type  = 0;
  int dhp_reserved     = 0;
  int dhp_chipid       = 0;

  int dhp_id = 0, dhp_row = 0, dhp_col = 0, dhp_adc = 0, dhp_cm = 0;
  int dhp_offset = 0;
  bool rowflag = false;

  if (anzahl < 4) {
    dhp_size_error++;
    return;
    //return -1;
  }

  if (printflag)
    B2INFO(" HEADER --  " << hex << dhp_pix[0] << hex << dhp_pix[1] << hex << dhp_pix[2] << hex << dhp_pix[3] << " -- ");



  if (printflag)
    B2INFO("DHH Header     |  " << hex << dhp_pix[0] << " ( " << dec << dhp_pix[0] << " ) ");
  dhh_header_error = (dhp_pix[0] >> 10) & 0x3F;
  dhh_header_type  = (dhp_pix[0] >> 8) & 0x3;
  dhh_header_dhh   = (dhp_pix[0] >> 2) & 0x3F;
  dhp_id           = dhp_pix[0] & 0x3;

  //if (dhp_id != 0) {// onyl for testbeam with one dhp
  //  B2INFO(" DECODE ERROR ... DHP ID !=0  " << hex << dhp_pix[0]);
  //  printflag = true;
  //}
  if (dhp_id == 0) {
    dhp_offset = 0;
  }
  if (dhp_id == 1) {
    dhp_offset = 64;
  }
  if (dhp_id == 2) {
    dhp_offset = 128;
  }
  if (dhp_id == 3) {
    dhp_offset = 192;
  }
  if (printflag) {
    B2INFO(" error     |   " << hex << dhh_header_error << " ( " << hex << dhh_header_error << " ) ");
    B2INFO(" type     |   " << hex << dhh_header_type << " ( " << hex << dhh_header_type << " ) ");
    B2INFO(" DHH ID     |   " << hex << dhh_header_dhh << " ( " << hex << dhh_header_dhh << " ) ");
    B2INFO(" chip ID     |   " << hex << dhp_id << " ( " << hex << dhp_id << " ) ");
  }

  dhh_header_id_I  = dhp_pix[1] & 0xFFFF;
  if (printflag)
    B2INFO(" Trigger Nr     |   " << hex << dhh_header_id_I << " ( " << hex << dhh_header_id_I << " ) ");

  if (printflag)
    B2INFO(" DHP Header     |   " << hex << dhp_pix[2] << " ( " << hex << dhp_pix[2] << " ) ");
  dhp_header_type  = (dhp_pix[2] >> 13) & 0x7;
  dhp_reserved     = (dhp_pix[2] >> 8) & 0x1F;
  dhp_chipid       =  dhp_pix[2] & 0xFF;

  if (printflag) {
    B2INFO(" DHP type          |    " << hex << dhp_header_type << " ( " << dec << dhp_header_type << " ) ");
    B2INFO(" DHP rederved          |    " << hex << dhp_reserved << " ( " << dec << dhp_reserved << " ) ");
    B2INFO(" DHP chip ID          |    " << hex << dhp_chipid << " ( " << dec << dhp_chipid << " ) ");
  }


  int dhp_warning = 0;

  if (dhp_pix[2] == dhp_pix[3]) {
    B2INFO(" Warn: double $A000 detected ...could be error ");
    dhp_warning++;
  }

  dhp_header_id_I  = dhp_pix[3] & 0xFFFF;
  if (printflag)
    B2INFO(" DHP Frame Nr     |   " << hex << dhp_header_id_I << " ( " << hex << dhp_header_id_I << " ) ");

  if (dhp_pix[3] == dhp_pix[4]) {
    B2INFO(" Warn: Could be double FrameNr " << dhp_pix[3] << "=" << dhp_pix[4]);
    dhp_warning++;

  } else {

  }


  for (unsigned int i = 4; i < anzahl ; i++) {

    if (printflag)
      B2INFO(" -- " << hex << dhp_pix[i] << " --   " << dec << i);
    {
      if (((dhp_pix[i] >> 15) & 0x1) == 0) {
        rowflag = true;
        dhp_row = (dhp_pix[i] >> 5) & 0xFFE;
        dhp_cm  = dhp_pix[i] & 0x3F;
        if (printflag)
          B2INFO(" SetRow: " << hex << dhp_row << " CM " << hex << dhp_cm);
      } else {
        if (!rowflag) {
          B2ERROR(" Error: Pix without Row!!! skip dhp data ");
          dhp_pixel_error++;
          return;
          // return -2;
        } else {
          dhp_row = (dhp_row & 0xFFE) | ((dhp_pix[i] >> 14) & 0x001);
          dhp_col = ((dhp_pix[i] >> 8) & 0x3F) + dhp_offset;
          dhp_adc = dhp_pix[i] & 0xFF;
          if (printflag)
            B2INFO(" SetPix: Row " << hex << dhp_row << " Col " << hex << dhp_col << " ADC " << hex << dhp_adc
                   << " CM " << hex << dhp_cm);

          /*if (verbose) {
            B2INFO(" raw    |   " << hex << d[i]);
            B2INFO(" row " << hex << ((d[i] >> 20) & 0xFFF) << "(" << ((d[i] >> 20) & 0xFFF) << ")" << " col " << "(" << hex << ((d[i] >> 8) & 0xFFF) << ((d[i] >> 8) & 0xFFF)
                   << " adc " << "(" << hex << (d[i] & 0xFF) << (d[i] & 0xFF) << ")");
            B2INFO(" dhh_ID " << dhh_ID);
            B2INFO(" start-Frame-Nr " << dec << dhh_first_frame_id_lo);
            B2INFO(" toffset " << toffset);
          };*/

          if (commode) {/// TODO I Have the strong feeling that this is wrong, as CM is already substracted in DHH, isnt it??
            m_storeRawHits.appendNew(dhh_ID, dhp_row, dhp_col, dhp_adc,
                                     dhh_first_frame_id_lo, toffset, dhp_cm
                                    );
          } else {
            m_storeRawHits.appendNew(dhh_ID, dhp_row, dhp_col, dhp_adc,
                                     dhh_first_frame_id_lo, toffset, 0
                                    );
          }
        }
      }
    }
  }

  if (printflag) {
    B2INFO(" header ID    |    " << hex << dhh_header_id_I << " ( " << dhh_header_id_I << " ) ");
    B2INFO(" error    |    " << hex << dhh_header_error << " ( " << dhh_header_error << " ) ");
    B2INFO(" type    |    " << hex << dhh_header_type << " ( " << dhh_header_type << " ) ");
    B2INFO(" DHH_ID    |    " << hex << dhh_header_dhh << " ( " << dhh_header_dhh << " ) ");
    B2INFO(" chip ID    |    " << hex << dhp_id << " ( " << dhp_id << " ) ");
    /*for (int i = 0; i < raw_anzahl ; i++) {
      B2INFO(" RAW      |   " << hex << p_pix[i]);
      printf("raw %08X  |  ", p_pix[i]);
      B2INFO(" row " << hex << ((p_pix[i] >> 20) & 0xFFF) << dec << " ( " << ((p_pix[i] >> 20) & 0xFFF) << " ) " << " col " << hex << ((p_pix[i] >> 8) & 0xFFF)
             << " ( " << dec << ((p_pix[i] >> 8) & 0xFFF) << " ) " << " adc " << hex << (p_pix[i] & 0xFF) << " ( " << (p_pix[i] & 0xFF) << " ) "
            );
    }*/
  }

//  return dhp_header_id_I;
//    if (cid == -1) dhp_size_error++;
//    if (cid == -2) dhp_pixel_error++;
};

unsigned int dhh_first_frame_id_lo = 0;
unsigned int dhh_first_offset = 0;

void PXDUnpackerModule::unpack_frame(void* data, int len, bool pad, int& last_framenr, int& last_wie, int& last_start, int& last_end, unsigned int& last_evtnr)
{
  dhh_frame_header_word0* hw;
  error_flag = false;
  hw = (dhh_frame_header_word0*)data;
// printf("len %d", len);

  dhh_frames dhh;
  dhh.set(data, hw->get_type(), len, pad);
  int s;
  s = dhh.size() * 4;
  if (len != s && s != 0) {
    B2WARNING(" Size (real) " << len << " != " << s << " (in data) " << pad);
  }

  int le = 0, ls = last_start;

  int lfnr;
  unsigned int evtnr;

  lfnr = hw->get_fnr();
  evtnr = dhh.get_evtnr();

  switch (dhh.get_type()) {
    case DHH_FRAME_HEADER_DATA_TYPE_DHP_RAW:

      ((dhh_direct_readout_frame_raw*)data)->print();
      dhh.calc_crc();
      stat_raw++;
      dhh.write_pedestal();
      break;
    case DHH_FRAME_HEADER_DATA_TYPE_DHP_ZSD:

      hw->print();
//       B2WARNING(" Size (real) " << len << " != " << s << " (in data) " << pad);
      dhh.calc_crc();
      stat_zsd++;

      unpack_dhp(data, len - 4, dhh_first_frame_id_lo, hw->get_dhhid(), dhh_first_offset);

      break;
    case DHH_FRAME_HEADER_DATA_TYPE_DCE_RAW:

      hw->print();
      dhh.calc_crc();
      break;
    case DHH_FRAME_HEADER_DATA_TYPE_COMMODE:

      hw->print();
      dhh.calc_crc();
      break;
    case DHH_FRAME_HEADER_DATA_TYPE_EVT_FRM: {
      bool fake = ((dhh_event_frame*)data)->is_fake();
      if (fake) {
        B2WARNING("Faked DHH START Data -> trigger without Data!");
      } else {
        ((dhh_event_frame*)data)->print();
      }
      if (last_end == 0) {
        end_error++;
        if (verbose) {
          B2ERROR(" Error: Start without End ");
        };
        error_flag = true;
      }
      ls = 1;
      last_framenr = -1;
      last_wie = 0;

      if (!fake) {
        if ((evtnr & 0x7FFF) != ((last_evtnr + 1) & 0x7FFF)) {
          evt_skip_error++;
          if (verbose) {
            B2WARNING(" Event skipped: " << evtnr << " != " << last_evtnr << " + 1 ");
          };
          error_flag = true;
        }
      }
      last_evtnr = evtnr;
      dhh_first_frame_id_lo = ((dhh_event_frame*)data)->get_sfnr();
      dhh_first_offset = ((dhh_event_frame*)data)->get_toffset();
      if (!fake) dhh.calc_crc();
      stat_start++;
      break;
    };
    case DHH_FRAME_HEADER_DATA_TYPE_GHOST:
      ((dhh_ghost_frame*)data)->print();
      dhh.calc_crc();
      stat_ghost++;
      last_wie -= 2;
      break;
    case DHH_FRAME_HEADER_DATA_TYPE_END_FRM: {
      bool fake = ((dhh_end_event_frame*)data)->is_fake();
      if (fake) {
        B2WARNING("Faked DHH END Data -> trigger without Data!");
      } else {
        ((dhh_end_event_frame*)data)->print();
      }
      ls = 0;
      le = 1;
      stat_end++;
      if (last_start == 0) {
        B2ERROR(" Error: End without Start ");
        start_error++;
        error_flag = true;
      }
      if (!fake) {
        int w;
        w = ((dhh_end_event_frame*)data)->get_words() * 2;
        last_wie += 2;
        if (verbose) {
          B2INFO(" last_wie " << last_wie << " w " << w);
        };
        if (last_wie != w) {
          if (verbose) {
            B2INFO(" Error: WIE " << hex << last_wie << " vs END " << hex << w << " pad " << pad);
          };
          error_flag = true;
          wie_error++;
        } else {
          if (verbose)
            B2INFO(" EVT END: WIE " << hex << last_wie << " == END " << hex << w << " pad " << pad);
        }
        dhh.calc_crc();
      }
      break;
    };
    case DHH_FRAME_HEADER_DATA_TYPE_HLTROI:
      hw->print();
      ((dhh_onsen_frame*)data)->set_length(len - 4);
      ((dhh_onsen_frame*)data)->print();
      //dhh.calc_crc(); /// WILL FAIL anyway
      break;
    default:
      B2ERROR(" Error: no data ");
      type_error++;
      hw->print();
      error_flag = true;
      break;
  }

  if (evtnr != last_evtnr) {
    B2ERROR(" Error: Event Nr " << evtnr << " != " << last_evtnr);
    evtnr_error++;
    error_flag = true;
  }
  if (lfnr != ((last_framenr + 1) & 0xF)) {
    B2ERROR(" Error: Frame Nr " << lfnr << " != " << last_framenr << " + 1 ");
    fnr_error++;
    error_flag = true;
  }
  last_framenr = lfnr;

  last_wie += len;

  last_end = le;
  last_start = ls;
}

void PXDUnpackerModule::unpack_dhhc_frame(void* data, int len, bool pad, int& last_wie, int& last_start, int& last_end, unsigned int& last_evtnr)
{
  dhhc_frame_header_word0* hw;
  error_flag = false;
  hw = (dhhc_frame_header_word0*)data;
  // printf("len %d", len);

  dhhc_frames dhhc;
  dhhc.set(data, hw->get_type(), len, pad);
  int s;
  s = dhhc.size() * 4;
  if (len != s && s != 0) {
    B2WARNING(" Size (real) " << len << " != " << s << " (in data) " << pad);
  }

  int le = 0, ls = last_start;

  unsigned int evtnr;

  evtnr = dhhc.get_evtnr();
  int dhh_id = 0x0; //hw->get_dhhid()

  switch (dhhc.get_type()) {
    case DHHC_FRAME_HEADER_DATA_TYPE_DHP_RAW: {

      ((dhhc_direct_readout_frame_raw*)data)->print();
      dhhc.calc_crc();
      stat_raw++;
      dhhc.write_pedestal();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD: {

      hw->print();
      //       B2WARNING(" Size (real) " << len << " != " << s << " (in data) " << pad);
      dhhc.calc_crc();
      stat_zsd++;

      unpack_dhp(data, len - 4, dhh_first_frame_id_lo, dhh_id, dhh_first_offset);

      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_FCE_RAW: {

      hw->print();
      dhhc.calc_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_COMMODE: {

      hw->print();
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
      if (last_end == 0) {
        end_error++;
        if (verbose) {
          B2ERROR(" Error: Start without End ");
        };
        error_flag = true;
      }
      ls = 1;
      last_wie = 0;

      if (!fake) {
        if ((evtnr & 0x7FFF) != ((last_evtnr + 1) & 0x7FFF)) {
          evt_skip_error++;
          if (verbose) {
            B2WARNING(" Event skipped: " << evtnr << " != " << last_evtnr << " + 1 ");
          };
          error_flag = true;
        }
      }
      last_evtnr = evtnr;
//         dhh_first_frame_id_lo = ((dhhc_start_frame*)data)->get_sfnr();
//         dhh_first_offset = ((dhhc_start_frame*)data)->get_toffset();
      if (!fake) dhhc.calc_crc();
      stat_start++;
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHH_START: {
      bool fake = ((dhhc_dhh_start_frame*)data)->is_fake();
      if (fake) {
        B2WARNING("Faked DHH START Data -> trigger without Data!");
      } else {
        ((dhhc_dhh_start_frame*)data)->print();
      }
      dhh_first_frame_id_lo = ((dhhc_dhh_start_frame*)data)->get_sfnr();
      dhh_first_offset = ((dhhc_dhh_start_frame*)data)->get_toffset();
      if (!fake) dhhc.calc_crc();
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_GHOST:
      ((dhhc_ghost_frame*)data)->print();
      dhhc.calc_crc();
      stat_ghost++;
      last_wie -= 2;
      break;
    case DHHC_FRAME_HEADER_DATA_TYPE_DHHC_END: {
      bool fake = ((dhhc_end_frame*)data)->is_fake();
      if (fake) {
        B2WARNING("Faked DHHC END Data -> trigger without Data!");
      } else {
        ((dhhc_end_frame*)data)->print();
      }
      ls = 0;
      le = 1;
      stat_end++;
      if (last_start == 0) {
        B2ERROR(" Error: End without Start ");
        start_error++;
        error_flag = true;
      }
      if (!fake) {
        /*int w;
        w = ((dhhc_dhh_end_frame*)data)->get_words() * 2;
        last_wie += 2;
        if (verbose) {
           B2INFO(" last_wie " << last_wie << " w " << w);
        };
        if (last_wie != w) {
           if (verbose) {
              B2INFO(" Error: WIE " << hex << last_wie << " vs END " << hex << w << " pad " << pad);
           };
           error_flag = true;
           wie_error++;
        } else {
           if (verbose)
              B2INFO(" EVT END: WIE " << hex << last_wie << " == END " << hex << w << " pad " << pad);
        }*/
        dhhc.calc_crc();
      }
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_DHH_END: {
      bool fake = ((dhhc_dhh_end_frame*)data)->is_fake();
      if (fake) {
        B2WARNING("Faked DHH END Data -> trigger without Data!");
      } else {
        ((dhhc_dhh_end_frame*)data)->print();
      }
      if (!fake) {
        dhhc.calc_crc();
      }
      break;
    };
    case DHHC_FRAME_HEADER_DATA_TYPE_HLTROI:
      hw->print();
      ((dhhc_onsen_frame*)data)->set_length(len - 4);
      ((dhhc_onsen_frame*)data)->print();
      //dhh.calc_crc(); /// WILL FAIL anyway
      break;
    default:
      B2ERROR(" Error: no data ");
      type_error++;
      hw->print();
      error_flag = true;
      break;
  }

  if (evtnr != last_evtnr) {
    B2ERROR(" Error: Event Nr " << evtnr << " != " << last_evtnr);
    evtnr_error++;
    error_flag = true;
  }

  last_wie += len;

  last_end = le;
  last_start = ls;
}



