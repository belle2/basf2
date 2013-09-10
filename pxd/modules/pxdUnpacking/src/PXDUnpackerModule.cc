/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdUnpacking/PXDUnpackerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/dataobjects/RawPXD.h>
#include <boost/foreach.hpp>
#include <pxd/dataobjects/PXDRawHit.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

#define FIDS    32
#define ROWS    1024
#define COLUMNS 256
#define MAX_DATA  20

#define DHP_FRAME_HEADER_DATA_TYPE_RAW  0x0
#define DHP_FRAME_HEADER_DATA_TYPE_ZSD  0x5
#define DHH_FRAME_HEADER_DATA_TYPE_DHP_RAW  0x0
#define DHH_FRAME_HEADER_DATA_TYPE_DHP_ZSD  0x5
#define DHH_FRAME_HEADER_DATA_TYPE_DCE_RAW  0x1
#define DHH_FRAME_HEADER_DATA_TYPE_COMMODE  0x6
#define DHH_FRAME_HEADER_DATA_TYPE_EVT_FRM  0x3
#define DHH_FRAME_HEADER_DATA_TYPE_GHOST    0x2
#define DHH_FRAME_HEADER_DATA_TYPE_END_FRM  0x4
#define DHH_FRAME_HEADER_DATA_TYPE_UNUSED   0x7


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
bool verbose = false;
bool dcp_check_flag = true;
bool error_flag = false;

char* dhh_type_name[8] = {
  (char*)"DHP_RAW",
  (char*)"DCE_RAW",
  (char*)"GHOST  ",
  (char*)"EVT_FRM",
  (char*)"END_FRM",
  (char*)"DHP_ZSD",
  (char*)"COMMODE",
  (char*)"UNUSED ",
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
      B2INFO(" DHH FRAME TYP " << hex << get_type() << dec << dhh_type_name[get_type()] << " FNR " << hex << get_fnr()
             << " DDHID " << hex << get_dhhid() << " CHIP " << get_chipid() << " ERR " << get_err());
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
    unsigned int* d;
    dhh_crc_32_type bocrc;
    d = (unsigned int*)this;
    for (unsigned int k = 0; k < (size() - 1) * 4; k++) {
      bocrc.process_byte(((const unsigned char*)d)[(k ^ 1)]);
    }
    unsigned int c;
    c = bocrc.checksum();
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHH Event Frame CRC " << hex << c << " == " << hex << crc32);
    } else {
      crc_error++;
      B2INFO(" DHH Event Frame CRC " << hex << c << " != " << hex << crc32);
      error_flag = true;
    }
    return c;
  };
  void set_crc(unsigned int c) {
    crc32 = c;
  };
  inline static unsigned int size(void) {
    return 4;
  };
  void stream(FILE* fh) {
    fwrite(&word0, 2, 1, fh);
    fwrite(&trigger_nr_lo, 2, 1, fh);
    fwrite(&trigger_nr_hi, 2, 1, fh);
    fwrite(&time_tag_lo, 2, 1, fh);
    fwrite(&time_tag_hi, 2, 1, fh);
    fwrite(&sfnr_offset, 2, 1, fh);
    fwrite(&crc32, 4, 1, fh);
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO(" DHH Event Frame TNRLO " << hex << trigger_nr_lo << " TNRHI " << hex << trigger_nr_hi << " TTLO " << hex
             << " TTHI " << hex << time_tag_hi << " SFNR " << hex << ((sfnr_offset >> 10) & 0x3F) << " OFF " << hex << (sfnr_offset & 0x3FF)
             << " CRC " << hex << crc32 << calc_crc());
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
    unsigned int* d;
    unsigned int c = 0;
    d = (unsigned int*)this;

    if (verbose)
      B2INFO(" DHH Common Frame CRC:");
    for (unsigned int i = 0; i < size(); i++) if (verbose) B2INFO(" d " << hex << d[i]);
    return c;
  };
  void set_crc(unsigned int c) {
    crc32 = c;
  };
  inline static unsigned int size(void) {
    return 2 + 96 / 2;
  };
  void stream(FILE* fh) {
    fwrite(&word0, 2, 1, fh);
    fwrite(&trigger_nr_lo, 2, 1, fh);
    fwrite(data, 2, 96, fh);
    fwrite(&crc32, 4, 1, fh);
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
    return 2;
  };
  void stream(FILE* fh) {
    fwrite(&word0, 2, 1, fh);
    fwrite(&trigger_nr_lo, 2, 1, fh);
    ///
    fwrite(&crc32, 4, 1, fh);
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO(" DHH Direct Readout (Raw|ZSD) Frame TNRLO " << hex << trigger_nr_lo << " CRC " << hex << calc_crc());
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
    unsigned int* d;
    dhh_crc_32_type bocrc;
    d = (unsigned int*)this;
    for (unsigned int k = 0; k < (size() - 1) * 4; k++) {
      bocrc.process_byte(((const unsigned char*)d)[(k ^ 1)]);
    }
    unsigned int c;
    c = bocrc.checksum();

    return c;
  };
  void set_crc(unsigned int c) {
    crc32 = c;
  };
  inline static unsigned int size(void) {
    return 2;
  };
  void stream(FILE* fh) {
    fwrite(&word0, 2, 1, fh);
    fwrite(&trigger_nr_lo, 2, 1, fh);
    fwrite(&crc32, 4, 1, fh);
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO(" DHH Ghost Frame TNRLO " << hex << trigger_nr_lo << " CRC " << hex << crc32 << hex << calc_crc());
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
    unsigned int* d;
    dhh_crc_32_type bocrc;
    d = (unsigned int*)this;
    for (unsigned int k = 0; k < (size() - 1) * 4; k++) {
      bocrc.process_byte(((const unsigned char*)d)[(k ^ 1)]);
    }
    unsigned int c;
    c = bocrc.checksum();
    if (c == crc32) {
      if (verbose)
        B2INFO(" DHH End Frame CRC " << hex << c << " ==  " << hex << crc32);
    } else {
      crc_error++;
      B2INFO(" DHH End Frame CRC " << hex << c << " != " << hex << crc32);
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
  void stream(FILE* fh) {
    fwrite(&word0, 2, 1, fh);
    fwrite(&trigger_nr_lo, 2, 1, fh);
    fwrite(&wordsinevent, 4, 1, fh);
    fwrite(&errorinfo, 4, 1, fh);
    fwrite(&crc32, 4, 1, fh);
  };
  void print(void) {
    word0.print();
    if (verbose)
      B2INFO("DHH End Frame TNRLO " << hex << trigger_nr_lo << " WIEVT " << hex << wordsinevent << " ERR " << hex << errorinfo
             << " CRC " << hex << crc32 << hex << calc_crc());
  };
};

class dhh_frames {
  void* data;
  unsigned int datasize;
  int type;
  int length;
  int pad;
public:
  dhh_frames(void) {
    data = 0;
    datasize = 0;
    type = -1;
    length = 0;
    pad = 0;
  };
  int get_type(void) {
    return type;
  };
  void set(void* d, unsigned int t) {
    data = d;
    type = t;
    length = 0;
    pad = 0;
  };
  void set(void* d, unsigned int t, unsigned int l, unsigned int p) {
    data = d;
    type = t;
    length = l;
    pad = p;
  };
  void set(void* d) {
    data = d;
    type = ((dhh_frame_header_word0*)data)->get_type();
    length = 0;
    pad = 0;
  };
  unsigned int get_evtnr(void) {
    return ((unsigned short*)data)[1];
  };
  unsigned int calc_crc(void) {
    unsigned int* d;
    dhh_crc_32_type bocrc;
    d = (unsigned int*)this;
    for (unsigned int k = 0; k < (size() - 1) * 4; k++) {
      bocrc.process_byte(((const unsigned char*)d)[(k ^ 1)]);
    }
    unsigned int c;
    c = bocrc.checksum();

    unsigned int crc32;
    if (pad != 0) {
      crc32 = (0xFFFF0000 & *(unsigned int*)(d + length - 2))  | (0xFFFF & *(unsigned int*)(d + length - 4));
    } else {
      crc32 = *(unsigned int*)(d + length - 4);
    }

    if (c == crc32) {
      if (verbose)
        B2INFO(" DHH Data Frame CRC: " << hex << c << " == " << hex << crc32);
    } else {
      crc_error++;
      if (verbose) {
        B2INFO(" DHH Data Frame CRC: " << hex << c << hex << crc32 << hex << * (unsigned int*)(d + length - 8) << hex
               << * (unsigned int*)(d + length - 6) << hex << * (unsigned int*)(d + length - 4) << hex << * (unsigned int*)(d + length - 2) << hex
               << * (unsigned int*)(d + length + 0) << hex << * (unsigned int*)(d + length + 2));
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
        if (verbose)
          B2INFO(" Error: DCE type no supported ");
        s = 0;
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
      case DHH_FRAME_HEADER_DATA_TYPE_UNUSED:
        B2INFO(" Error: type undefined ");
        s = 0;
        error_flag = true;
        break;

      default:
        B2INFO(" Error: no data ");
        error_flag = true;
        s = 0;
        break;
    }
    datasize = s;
    return s;
  };

  void write_pedestal(void) {
    B2INFO(" Write Pedestal Date - done ");

  };

};



int PXDUnpackerModule::format_raw_from_dhp(void* dhp_in, int anzahl, void* raw_out, int& raw_anzahl, bool printflag = false, bool commode = false)
{
  unsigned int* p_pix = (unsigned int*)raw_out;
  unsigned short* dhp_pix = (unsigned short*)dhp_in;
  raw_anzahl = 0;

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

  if (anzahl < 4) return -1;

  if (printflag)
    B2INFO(" HEADER --  " << hex << dhp_pix[0] << hex << dhp_pix[1] << hex << dhp_pix[2] << hex << dhp_pix[3] << " -- ");



  if (printflag)
    B2INFO("DHH Header     |  " << hex << dhp_pix[0] << " ( " << dec << dhp_pix[0] << " ) ");
  dhh_header_error = (dhp_pix[0] >> 10) & 0x3F;
  dhh_header_type  = (dhp_pix[0] >> 8) & 0x3;
  dhh_header_dhh   = (dhp_pix[0] >> 2) & 0x3F;
  dhp_id           = dhp_pix[0] & 0x3;

  if (dhp_id != 0) {
    B2INFO(" DECODE ERROR ... DHP ID !=0  " << hex << dhp_pix[0]);
    printflag = true;
  }
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
    B2INFO(" Warn: Could be double FrameNr ");
    dhp_warning++;

  } else {

  }


  for (int i = 4; i < anzahl ; i++) {

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
          B2INFO(" Error: Pix without Row!!! skip dhp data ");
          return -2;
        } else {
          dhp_row = (dhp_row & 0xFFE) | ((dhp_pix[i] >> 14) & 0x001);
          dhp_col = ((dhp_pix[i] >> 8) & 0x3F) + dhp_offset;
          dhp_adc = dhp_pix[i] & 0xFF;
          if (printflag)
            B2INFO(" SetPix: Row " << hex << dhp_row << " Col " << hex << dhp_col << " ADC " << hex << dhp_adc
                   << " ADC+CM " << hex << (dhp_adc + dhp_cm));

          if (commode) {
            p_pix[raw_anzahl] = ((dhp_row & 0x7FF) << 21) | ((dhp_col & 0x7FF) << 10) | ((dhp_adc + dhp_cm) & 0x3FF);
          } else {
            p_pix[raw_anzahl] = ((dhp_row & 0xFFF) << 20) | ((dhp_col & 0xFFF) << 8) | ((dhp_adc & 0xFF));
          }
          raw_anzahl++;
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
    for (int i = 0; i < raw_anzahl ; i++) {
      B2INFO(" RAW      |   " << hex << p_pix[i]);
      printf("raw %08X  |  ", p_pix[i]);
      B2INFO(" row " << hex << ((p_pix[i] >> 20) & 0xFFF) << dec << " ( " << ((p_pix[i] >> 20) & 0xFFF) << " ) " << " col " << hex << ((p_pix[i] >> 8) & 0xFFF)
             << " ( " << dec << ((p_pix[i] >> 8) & 0xFFF) << " ) " << " adc " << hex << (p_pix[i] & 0xFF) << " ( " << (p_pix[i] & 0xFF) << " ) "
            );
    }
  }

  return dhp_header_id_I;
}

PXDUnpackerModule::PXDUnpackerModule() :
  Module()
{
  //Set module properties
  setDescription("Unpack Raw PXD Hits");
  setPropertyFlags(c_ParallelProcessingCertified);

  m_storeRAWPxdName = "RawPXDs";
  //addParam("Clusters", m_storeClustersName, "Cluster collection name",
//          string(""));

  m_storeRawHitsName = "RawHit";



}

void PXDUnpackerModule::initialize()
{
  //Register output collections
  StoreArray<PXDRawHit>::registerPersistent(m_storeRawHitsName);
  m_storeRawHitsName = "RawHit";
}
unsigned char tmpbuffer[1024 * 1024 * 16];

void PXDUnpackerModule::event()
{
  StoreArray<RawPXD> storeRaws(m_storeRAWPxdName);
  StoreArray<PXDRawHit> storeRawHits(m_storeRawHitsName);
  m_storeRawHitsName = "RawHit";
  if (!storeRawHits.isValid())
    storeRawHits.create();
  else
    storeRawHits.getPtr()->Clear();



  int nRaws = storeRaws.getEntries();
  if (verbose) {
    B2INFO(" PXD Unpacker --> RawPXD Objects in event: " << nRaws);
  };
  if (nRaws == 0)
    return;


  for (int i = 0; i < nRaws; i++) {
    if (verbose) {
      B2INFO(" PXD Unpacker --> Unpack Objects: " << i);
    };
    unpack_event(storeRaws[i]);

  }

}

void PXDUnpackerModule::unpack_event(RawPXD* px)
{
  int last_wie = 0, last_framenr = 0, last_start = 0, last_end = 0;
  unsigned int last_evtnr = 0;
  unsigned int Offset = 0;
  unsigned int len = 0, pad;
  unsigned int Byte_gesamt = 0;
  unsigned int Offset_Tabelle[256];

  unsigned int Byte_per_event;
  unsigned int Frames_per_event;

  int i;
  unsigned int* data;
  data = (unsigned int*)px->data();
  if (verbose) {
    B2INFO(" PXD Unpacker --> data[0]: <-- Magic " << hex << data[0]);
    B2INFO(" PXD Unpacker --> data[1]: <-- Length " << hex << data[1]);
    B2INFO(" PXD Unpacker --> data[2]: <-- Dummy1 " << hex << data[2]);
    B2INFO(" PXD Unpacker --> data[3]: <-- Dummy2 " << hex << data[3]);
  };
  Byte_per_event = 0;
  Frames_per_event = 0;

  Byte_per_event += 4;

  len = data[1];
  Byte_per_event += 4;

  Byte_per_event += 4;

  Byte_per_event += 4;
  i = 0;
  do {
    Offset = data[4 + i];
    Offset_Tabelle[i] = Offset;
    Byte_per_event += 4;
    Frames_per_event += 1;
    i++;
  } while (Offset != 0);

  unsigned int* buffer;
  buffer = &data[4 + i];
  Byte_per_event += len;
  Byte_gesamt += Byte_per_event;
  Byte_per_event = htonl(Byte_per_event);

  int ll = 0;
  for (int j = 0; j < i; j++) { //i= number of offsets
    int lo;
    if (j == 0)
      lo = Offset_Tabelle[j];
    else
      lo = Offset_Tabelle[j] - Offset_Tabelle[j - 1]; //lo = length of the j offset
    if (lo <= 0) {
      B2INFO(" break: " << j << Offset_Tabelle[j] << Offset_Tabelle[j - 1] << ll << len);
      exit(0);
    }
    if (verbose) {
      B2INFO(" swap " << j << ll << lo << len);
    };
    if (lo & 0x3) {
      pad = 0xFF;
      if (verbose)
        B2INFO(" Data with not MOD 4 length " << " ( " << lo << pad << " ) ");
    } else {
      pad = 0;
    }

    unpack_frame(ll + (char*)buffer, lo, pad, last_framenr, last_wie, last_start, last_end, last_evtnr);
    //printf("lo %d",lo);
    ll = Offset_Tabelle[j];      //ll becomes the value of the j offset
    if (Offset_Tabelle[j] == len) break;
    //when offset j==len break beacause frame ended
  }

}

void PXDUnpackerModule::fill_pixelmap(void* data, unsigned int len, unsigned int dhh_first_frame_id_lo, unsigned int dhh_ID, unsigned short toffset)
{
  // len in bytes!!!
  StoreArray<PXDRawHit> storeRawHits(m_storeRawHitsName);
  int lout = 0;
  bool commode = false;
  int cid;
  int fid;
  cid = format_raw_from_dhp(data, len / 2, tmpbuffer, lout, false, commode);
  fid = (cid - dhh_first_frame_id_lo) & (FIDS - 1);

  if (cid >= 0 && fid >= 0 && fid < FIDS) {
    unsigned int* d;
    d = (unsigned int*)tmpbuffer;


    for (int i = 0; i < lout ; i++) {
      if (verbose) {
        B2INFO(" raw    |   " << hex << d[i]);
        B2INFO(" row " << hex << ((d[i] >> 20) & 0xFFF) << "(" << ((d[i] >> 20) & 0xFFF) << ")" << " col " << "(" << hex << ((d[i] >> 8) & 0xFFF) << ((d[i] >> 8) & 0xFFF)
               << " adc " << "(" << hex << (d[i] & 0xFF) << (d[i] & 0xFF) << ")");
        B2INFO(" dhh_ID " << dhh_ID);
        B2INFO(" start-Frame-Nr " << dec << dhh_first_frame_id_lo);
        B2INFO(" toffset " << toffset);
      };
      storeRawHits.appendNew(PXDRawHit(
                               dhh_ID, (d[i] >> 20) & 0xFFF, (d[i] >> 8) & 0xFFF, d[i] & 0xFF,
                               dhh_first_frame_id_lo, toffset
                             ));
    }
  } else if (cid < 0) {
    if (cid == -1) dhp_size_error++;
    if (cid == -2) dhp_pixel_error++;
  }
};

unsigned int dhh_first_frame_id_lo = 0;
unsigned int dhh_first_offset = 0;

void PXDUnpackerModule::unpack_frame(void* data, int len, int pad, int& last_framenr, int& last_wie, int& last_start, int& last_end, unsigned int& last_evtnr)
{
  dhh_frame_header_word0* hw;
  error_flag = false;
  hw = (dhh_frame_header_word0*)data;
// printf("len %d", len);
  if (pad == 2) len -= pad;

  dhh_frames dhh;
  dhh.set(data, hw->get_type(), len, pad);
  int s;
  s = dhh.size() * 4;
  if (len != s) if (verbose)
      B2INFO(" Size (real) " << len << " != " << s << " (in data) " << pad);

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
      dhh.calc_crc();
      stat_zsd++;

      if (dcp_check_flag) fill_pixelmap(data, len - 4, dhh_first_frame_id_lo, hw->get_dhhid(), dhh_first_offset);

      break;
    case DHH_FRAME_HEADER_DATA_TYPE_DCE_RAW:

      hw->print();
      dhh.calc_crc();
      break;
    case DHH_FRAME_HEADER_DATA_TYPE_COMMODE:

      hw->print();
      dhh.calc_crc();
      break;
    case DHH_FRAME_HEADER_DATA_TYPE_EVT_FRM:

      ((dhh_event_frame*)data)->print();
      if (last_end == 0) {
        end_error++;
        if (verbose) {
          B2INFO(" Error: Start without End ");
        };
        error_flag = true;
      }
      ls = 1;
      last_framenr = -1;
      last_wie = 0;

      if ((evtnr & 0x7FFF) != ((last_evtnr + 1) & 0x7FFF)) {
        evt_skip_error++;
        if (verbose) {
          B2INFO(" Event skipped: " << evtnr << " != " << last_evtnr << " + 1 ");
        };
        error_flag = true;
      }
      last_evtnr = evtnr;
      dhh_first_frame_id_lo = ((dhh_event_frame*)data)->get_sfnr();
      dhh_first_offset = ((dhh_event_frame*)data)->get_toffset();
      stat_start++;
      break;
    case DHH_FRAME_HEADER_DATA_TYPE_GHOST:

      ((dhh_ghost_frame*)data)->print();
      dhh.calc_crc();
      stat_ghost++;
      last_wie -= 2;
      break;
    case DHH_FRAME_HEADER_DATA_TYPE_END_FRM:

      ((dhh_end_event_frame*)data)->print();
      ls = 0;
      le = 1;
      stat_end++;
      if (last_start == 0) {
        B2INFO(" Error: End without Start ");
        start_error++;
        error_flag = true;
      }
      int w;
      w = ((dhh_end_event_frame*)data)->get_words() * 2;
      last_wie += 2;
      if (verbose) {
        B2INFO(" last_wie " << last_wie << " w " << w);
      };
      if (last_wie != w) {
        if (verbose) {
          B2INFO(" Error: WIE " << hex << last_wie << " vs END " << hex << w << " pad " << hex << pad);
        };
        error_flag = true;
        wie_error++;
      } else {
        if (verbose)
          B2INFO(" EVT END: WIE " << hex << last_wie << " == END " << hex << w << " pad " << hex << pad);
      }
      break;
    case DHH_FRAME_HEADER_DATA_TYPE_UNUSED:
      B2INFO(" Error: type undefined ");
      type_error++;
      hw->print();
      error_flag = true;
      break;
    default:
      B2INFO(" Error: no data ");
      type_error++;
      hw->print();
      error_flag = true;
      break;
  }

  if (evtnr != last_evtnr) {
    B2INFO(" Error: Event Nr " << evtnr << " != " << last_evtnr);
    evtnr_error++;
    error_flag = true;
  }
  if (lfnr != ((last_framenr + 1) & 0xF)) {
    B2INFO(" Error: Frame Nr " << lfnr << " != " << last_framenr << " + 1 ");
    fnr_error++;
    error_flag = true;
  }
  last_framenr = lfnr;

  last_wie += len;

  last_end = le;
  last_start = ls;
}



