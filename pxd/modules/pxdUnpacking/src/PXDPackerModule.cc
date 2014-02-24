/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdUnpacking/PXDPackerModule.h>
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
REG_MODULE(PXDPacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

using boost::crc_optimal;
typedef crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhh_crc_32_type;
const unsigned int CRC_INIT = 0x00000000;

///******************************************************************
///*********************** Main packer code *************************
///******************************************************************

PXDPackerModule::PXDPackerModule() :
  Module(),
  m_storeRaws()
{
  //Set module properties
  setDescription("Pack PXD Hits to raw data object");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void PXDPackerModule::initialize()
{
  B2INFO("PXD Packer --> Init");
  //Register output collections
  m_storeRaws.registerAsPersistent();

  m_packed_events = 0;
}

void PXDPackerModule::terminate()
{
}

void PXDPackerModule::event()
{
  StoreArray<RawFTSW> storeFTSW;/// needed for event number

  B2INFO("PXD Packer --> Event");

//  int nRaws = storeRaws.getEntries();

  /*  ftsw_evt_nr = 0;
    ftsw_evt_mask = 0;
    for (auto & it : storeFTSW) {
      ftsw_evt_nr = it.GetEveNo(0);
      ftsw_evt_mask = 0x7FFF;
      B2INFO("PXD Packer --> FTSW Event Number: $" << hex << ftsw_evt_nr);
      break;
    }
  */

  VxdID lastVxdId = -1; /// invalid ... force to set first itertor/index
  /// We assume the Digits are sorted by VxdID (P.K. says they are)
  /// Thie saves some iterating lateron
  for (auto it = storeDigits.begin() ; it != storeDigits.end(); it++) {
    VxdID currentVxdId;
    currentVxdId = it->getSensorID();
    if (currentVxdId != lastVxdId) {
      // do something...
      lastVxdId = currentVxdId;
      startOfVxdID[currentVxdId] = std::distance(storeDigits.begin(), it);
    }
  }

  m_trigger_nr = m_packed_events;
  pack_event();
  /*
    int nsr = 0;
    for (auto & it : storeRaws) {
      if (verbose) {
        B2INFO("PXD Packer --> Pack Objects: ");
      };
      pack_event(it);
      nsr++;
    }
  */
  m_packed_events++;
}

void PXDPackerModule::endian_swap_frame(unsigned short* dataptr, int len)
{
  /// swap endianess of all shorts in frame BUT not the CRC (2 shorts)
  for (int i = 0; i < len / 2 - 2; i++) {
    dataptr[i] = htons(dataptr[i]);
  }
}

void PXDPackerModule::pack_event(void)
{
  for (int i = 0; i < 1; i++) {
    // loop for each DHHC in system
    // get active DHHCs from a database?
    // get active DHH mask from a database?

    m_onsen_header.clear();// Reset
    m_onsen_payload.clear();// Reset
    pack_dhhc(i, 0x1); // arbitrary until now
    // and write to PxdRaw object
    // header will be finished and endian swapped by contructor; payload already has be on filling the vector
    m_storeRaws.appendNew(m_onsen_header, m_onsen_payload);
  }

}

void PXDPackerModule::add_frame_to_payload(void)
{
  // checksum frame
  dhh_crc_32_type current_crc;
  current_crc.process_bytes(m_current_frame.data(), m_current_frame.size());
  add_int32(current_crc.checksum());

  // and add it
  m_onsen_header.push_back(m_current_frame.size());
  m_onsen_payload.push_back(m_current_frame);
}

void PXDPackerModule::add_int16(unsigned short w)
{
  m_current_frame.push_back((unsigned char)(w >> 8));
  m_current_frame.push_back((unsigned char)(w));
}

void PXDPackerModule::add_int32(unsigned int w)
{
  m_current_frame.push_back((unsigned char)(w >> 24));
  m_current_frame.push_back((unsigned char)(w >> 16));
  m_current_frame.push_back((unsigned char)(w >> 8));
  m_current_frame.push_back((unsigned char)(w));
}

void PXDPackerModule::start_frame(void)
{
  //m_current_crc.reset(CRC_INIT);
  m_current_frame.clear();
}

void PXDPackerModule::pack_dhhc(int dhhc_id, int dhh_active)
{

  /// HLT frame ??? t.b.d.

  /// DHHC Start

  start_frame();
  add_int32(0x58000000 | ((dhhc_id & 0xF) << 21) | ((dhh_active & 0x1F) << 16) | (m_trigger_nr & 0xFFFF));
  add_int16((m_trigger_nr & 0xFFFF0000) >> 16);
  add_int32(0x00000000); // TT 11-0 | Type | TT 27-12
  add_int32(0x00000000);  // TT 43-27 | Frame count
  add_frame_to_payload();

  // loop for each DHH in system
  // get active DHHs from a database?
  // we use events_packed as event number until we get it from somewhere else
  // Run & TTT etc are zero until better idea

  for (int i = 0; i < 5; i++) {
    if (dhh_active & 0x1) pack_dhh(i, 0xF);
    dhh_active >>= 1;
  }

  /// lets copy the HLT/ROI frame

  //  start_frame();
  //  crc_value = 0x38000000;//t.b.d.
  //  add_int32(crc_value);
  //  add_frame_to_payload();


  /// DHHC End

  start_frame();
  add_int32(0x60000000 | ((dhhc_id & 0xF) << 21) | (m_trigger_nr & 0xFFFF));
  add_int32(0x00000000);  // 16 bit word count
  add_int32(0x00000000);  // Error Flags
  add_frame_to_payload();

}

void PXDPackerModule::pack_dhh(int dhh_id, int dhp_active)
{
  // dhh_id is not dhh_id ...

  /// DHH Start

  start_frame();
  add_int32(0x18000000 | ((dhh_id & 0x3F) << 20) | ((dhp_active & 0xF) << 16) | (m_trigger_nr & 0xFFFF));
  add_int32(0x00000000);  // DHH Timer Lo | DHH Time Hi
  add_int16(0x00000000);  // Last DHP Frame Nr, Trigger Offset (nur 16 bit?)
  add_frame_to_payload();

// now prepare the data from one halfladder
// do the ROI selection???
// then loop for each DHP in system
// get active DHPs from a database?
// and pack dat per halfladder.
// we fake the framenr and startframenr until we find ome better solution

  if (dhp_active != 0) { /// is there any hardware switched on?
    /// clear pixelmap
    VxdID currentVxdId = 0; /// TODO get from somewhere
    auto map_it = startOfVxdID.find(currentVxdId);
    if (map_it != startOfVxdID.end()) {
      auto it = storeDigits.begin();
      advance(it, map_it->second);
      for (; it != storeDigits.end(); it++) {
        if (currentVxdId != it->getSensorID()) break; /// another sensor starts
        /// Fill pixel to pixelmap
      }
    }

    for (int i = 0; i < 4; i++) {
      if (dhp_active & 0x1) pack_dhp(i, dhh_id);
      dhp_active >>= 1;
    }
  }

  /// DHH End
  start_frame();
  add_int32(0x20000000 | ((dhh_id & 0x3F) << 20) | (m_trigger_nr & 0xFFFF));
  add_int32(0x00000000);  // 16 bit word count
  add_int32(0x00000000);  // Error Flags
  add_frame_to_payload();
}

void PXDPackerModule::pack_dhp(int chip_id, int dhh_id)
{
  // remark: chip_id != port most of the time ...


  start_frame();
  /// Ghost Frame
  add_int32(0x90000000 | ((dhh_id & 0x3F) << 20) | ((chip_id & 0x03) << 16) | (m_trigger_nr & 0xFFFF));
  add_frame_to_payload();

}

#if 0

#define NR_PIXELS_RAND  10
#define NR_PIXELS_RAND_OFF  -5

#define NR_LAYER_ONE  1 // 8
#define NR_LAYER_TWO  1 // 12
#define NR_CHIPS  1  // 4
#define NR_ROW   768
#define NR_COL_CHIP   64

#define BUFFERSIZE  65536

bool run_thread = true;

unsigned char bufferin[65536];
unsigned char bufferout[65536];
int bufferin_size = 0, bufferout_size;

struct onsen_header {
  unsigned int magic;
  unsigned int frames;
  unsigned int frame_length[256 - 2]; // oder so
} myonsen;

int rec_bytes = 0, rec_frames = 0, rec_trig = 0;
int send_bytes = 0, send_frames = 0, send_trig = 0;

void endian_swap(unsigned int* x)
{
  *x = (*x >> 24) |
       ((*x << 8) & 0x00FF0000) |
       ((*x >> 8) & 0x0000FF00) |
       (*x << 24);
};

void endian_swap(unsigned int& x)
{
  x = (x >> 24) |
      ((x << 8) & 0x00FF0000) |
      ((x >> 8) & 0x0000FF00) |
      (x << 24);
};

unsigned char pixelmap[NR_ROW][NR_COL_CHIP];
int dhp_pix[200000];

unsigned int roi_buffer[10000], roibuffer_cnt;

int generate_data_for_trig(unsigned int m_trigger_nr, unsigned char* buffer, unsigned int buffer_len)
{
  int  nr_pixel;

  /// Now the data frames ...
#if 1
  for (int dhh_ly = 0; dhh_ly <= 1; dhh_ly++) {
    for (int dhh_ld = 1; ((dhh_ly == 0 && dhh_ld <= NR_LAYER_ONE) || (dhh_ly == 1 && dhh_ld <= NR_LAYER_TWO)); dhh_ld++) {
      for (int dhh_se = 0; dhh_se <= 1; dhh_se++) {
        dhh_id = (dhh_ly & 0x01) << 5 | (dhh_ld & 0x0F) << 1 | (dhh_se & 0x01);
        for (int chip_id = 0; chip_id < NR_CHIPS; chip_id++) {

          int bufferout_sizestart;

          bufferout_sizestart = bufferout_size;
          nr_pixel = (rand() % NR_PIXELS_RAND) + NR_PIXELS_RAND_OFF;
//             memset(pixelmap, 0, sizeof(pixelmap));
          if (nr_pixel > 0) {

            /*
            //               printf("gen map\n");
              for (int i = 0; i < nr_pixel; i++) {
                int row, col, adc;
                row = rand() % NR_ROW;
                col = rand() % NR_COL_CHIP;
                adc = row + col;
                if (adc == 0) adc = 0x42;
                pixelmap[row][col] = adc;
                ///dhp_pix[i] = (row&0x3FF)<<20 | (col&0x3FF)<<8 | (adc&0xFF);

              }
              */
//               printf("gen list\n");

            unsigned int k;
            k = 0;
            /*
            for (int i = 0; i < NR_ROW; i++) {
                for (int j = 0; j < NR_COL_CHIP; j++) {
                  if (pixelmap[i][j]) {
                    dhp_pix[k++] = (i & 0x3FF) << 20 | (j & 0x3FF) << 8 | (pixelmap[i][j] & 0xFF);
                  }
                }
              }*/


            dhp_pix[k++] = ((m_trigger_nr & 0x1F) & 0x3FF) << 20 | (((m_trigger_nr & 0x3E0) >> 5) & 0x3FF) << 8 | ((m_trigger_nr >> 4) & 0xFF);
            dhp_pix[k++] = ((m_trigger_nr & 0x1F) & 0x3FF) << 20 | (((m_trigger_nr & 0xF80) >> 7) & 0x3FF) << 8 | ((m_trigger_nr >> 4) & 0xFF);
            dhp_pix[k++] = ((m_trigger_nr & 0x1F) & 0x3FF) << 20 | ((((m_trigger_nr & 0x3E0) >> 5) + 1) & 0x3FF) << 8 | ((m_trigger_nr >> 5) & 0xFF);
            dhp_pix[k++] = (((m_trigger_nr & 0x1F) + 1) & 0x3FF) << 20 | (((m_trigger_nr & 0xF80) >> 7) & 0x3FF) << 8 | ((m_trigger_nr >> 5) & 0xFF);

            nr_pixel = k;

            /*          printf("sort list\n");
                     for (int i=0; i<nr_pixel; i++) {
                        for (int j=i+1; j<nr_pixel; j++) {
                           int row_col_i, row_col_j;
                           if (dhp_pix[i] == dhp_pix[j]) {
                              int row = ((dhp_pix[j]>>20)&0x3FF) + 1;
                              dhp_pix[j] = dhp_pix[j]&0x003FFFF | (row&0x3FF)<<20 ;
                           }
                           if (dhp_pix[i] > dhp_pix[j]) {
                              int pix_sw;
                              pix_sw = dhp_pix[i];
                              dhp_pix[i] = dhp_pix[j];
                              dhp_pix[j] = pix_sw;
                           }
                        }
                     }*/

            count_dhp = 0;
            count_in_roi = 0;
            int last_in_roi = 0xFFFFFFFF;
            int pix_dhh, pix_row, pix_col;
            for (int i = 0; i < nr_pixel; i++) {
              int row_act, row_pre;

              row_act = (dhp_pix[i] >> 20) & 0x3FF;
              row_pre = (dhp_pix[i - 1] >> 20) & 0x3FF;
              if ((i == 0) || (row_act != row_pre)) {
                count_dhp++;
              }
              count_dhp++;

              pix_dhh = dhh_id;
              pix_row = (dhp_pix[i] >> 20) & 0x3FF ;
              pix_col = (dhp_pix[i] >> 8) & 0x3FF ;

              int is_in_roi = 1;
//                 int is_in_roi = 0;
//                 for (int r = 0; r < n_rois; r++) {
//                   if (pix_dhh == rois[0][r] && pix_row >= rois[1][r] && pix_row <= rois[3][r] && pix_col >= rois[2][r] && pix_col <= rois[4][r]) { is_in_roi = 1; }
//                 }

              if (is_in_roi == 1) {
                row_act = (dhp_pix[i] >> 20) & 0x3FF;
                row_pre = (last_in_roi >> 20) & 0x3FF;
                if ((last_in_roi == 0xFFFFFFFF) || (row_act != row_pre)) {
                  count_in_roi++;
                }
                count_in_roi++;

                last_in_roi = dhp_pix[i];
              }

            }

            unsigned int data_size;
            unsigned int data_size2;
            data_size2 = 0;
            if (count_in_roi % 2 != 0) { /// payload bytes
              data_size = count_in_roi * 2 + 14;
            } else {
              data_size = count_in_roi * 2 + 12;
            }

            crc_out.reset(CRC_INIT);
            int reformat = 1;

            crc_value = 0x28000000 | (dhh_id & 0x3F) << 20 | (reformat & 0x1) << 19 | (chip_id & 0x3) << 16 | m_trigger_nr & 0xFFFF ;
            crc_out.process_byte((crc_value >> 24) & 0xFF);   crc_out.process_byte((crc_value >> 16) & 0xFF);   crc_out.process_byte((crc_value >> 8) & 0xFF);   crc_out.process_byte(crc_value & 0xFF);
            add_int32(crc_value);
            data_size2 += 4;

            crc_value = 0xA0000000 | (dhh_id & 0x3F) << 18 | (chip_id & 0x03) << 16 ;
            crc_out.process_byte((crc_value >> 24) & 0xFF);   crc_out.process_byte((crc_value >> 16) & 0xFF);   crc_out.process_byte((crc_value >> 8) & 0xFF);   crc_out.process_byte(crc_value & 0xFF);
            add_int32(crc_value);
            data_size2 += 4;

            frame_nr++;

            count_dhp = 0;
            count_in_roi = 0;
            last_in_roi = 0xFFFFFFFF;
//        printf("\n");
            for (int i = 0; i < nr_pixel; i++) {
//    printf("%08X | ", dhp_pix[i]);
//    printf("R %03X  ", (dhp_pix[i]>>20)&0x3FF);
//    printf("C %03X  ", (dhp_pix[i]>>8)&0x3FF);
//    printf("A %02X | ", dhp_pix[i]&0xFF);

              int sor, sor_d;
              int row_act, row_pre;
#if 0
              row_act = (dhp_pix[i] >> 20) & 0x3FF;
              row_pre = (dhp_pix[i - 1] >> 20) & 0x3FF;
              if ((i == 0) || (row_act != row_pre)) {
                sor = ((dhp_pix[i] >> 20) & 0x3FE) << 5;
                sor_d = ((dhp_pix[i] >> 20) & 0x001);
//                   add_int16(DHH_file, sor);
                crc_value = sor ;
                crc_dhh.process_byte((crc_value >> 8) & 0xFF);   crc_dhh.process_byte(crc_value & 0xFF);
//      printf("SOR %04X  ", sor);
                count_dhp++;
//    } else { printf("          ", sor);
              }
              int dw;
              dw = 0x8000 | (sor_d & 0x001) << 15 | (dhp_pix[i] & 0x3FFF);
//                 add_int16(DHH_file, dw);
              crc_value = dw ;
              crc_dhh.process_byte((crc_value >> 8) & 0xFF);   crc_dhh.process_byte(crc_value & 0xFF);
//    printf("DW %04X | ", dw);
              count_dhp++;
#endif
              pix_dhh = dhh_id;
              pix_row = (dhp_pix[i] >> 20) & 0x3FF ;
              pix_col = (dhp_pix[i] >> 8) & 0x3FF ;

              int is_in_roi = 1;
//                 int is_in_roi = 0;
//                 for (int r = 0; r < n_rois; r++) {
//                   if (pix_dhh == rois[0][r] && pix_row >= rois[1][r] && pix_row <= rois[3][r] && pix_col >= rois[2][r] && pix_col <= rois[4][r]) { is_in_roi = 1; }
//                 }

              if (is_in_roi == 1) {

                row_act = (dhp_pix[i] >> 20) & 0x3FF;
                row_pre = (last_in_roi >> 20) & 0x3FF;
                if ((last_in_roi == 0xFFFFFFFF) || (row_act != row_pre)) {
                  sor = ((dhp_pix[i] >> 20) & 0x3FE) << 5;
                  sor_d = ((dhp_pix[i] >> 20) & 0x001);
                  crc_value = sor ;
                  crc_out.process_byte((crc_value >> 8) & 0xFF);   crc_out.process_byte(crc_value & 0xFF);
                  add_int16(crc_value);
                  data_size2 += 2;
                  //        printf("SOR %04X  ", sor);
                  count_in_roi++;
//      } else { printf("          ", sor);
                }
                int dw;
                dw = 0x8000 | (sor_d & 0x001) << 15 | (dhp_pix[i] & 0x3FFF);
                crc_value = dw ;
                crc_out.process_byte((crc_value >> 8) & 0xFF);   crc_out.process_byte(crc_value & 0xFF);
                add_int16(crc_value);
                data_size2 += 2;
                //      printf("DW %04X ", dw);
                count_in_roi++;

                last_in_roi = dhp_pix[i];
              }
//    printf("\n");

            }

            if (count_in_roi % 2 != 0) {
              crc_value = 0x0000 ;
              crc_out.process_byte((crc_value >> 8) & 0xFF);   crc_out.process_byte(crc_value & 0xFF);
              add_int16(crc_value);
              data_size2 += 2;
            }

            add_int32(crc_out() & 0xFFFFFFFF);
            data_size2 += 4;
            if (data_size2 != data_size) {
              fprintf(stderr, "==== data creation error ... -> call Bjoern ... %d %d %d %d ====\n", data_size, data_size2, count_in_roi, bufferout_size - bufferout_sizestart);
              return 0;
            }
            //      printf("%08X   DHH CRC \n", 0xC0010005);
            myonsen.frame_length[myonsen.frames++] = htonl(data_size);

          } else {
            crc_out.reset(CRC_INIT);

            crc_value = 0x90000000 | (dhh_id & 0x3F) << 20 | (chip_id & 0x03) << 16 | m_trigger_nr & 0xFFFF ;
            crc_out.process_byte((crc_value >> 24) & 0xFF);   crc_out.process_byte((crc_value >> 16) & 0xFF);   crc_out.process_byte((crc_value >> 8) & 0xFF);   crc_out.process_byte(crc_value & 0xFF);
            add_int32(crc_value);
            add_int32(crc_out() & 0xFFFFFFFF);
            frame_nr++;
            myonsen.frame_length[myonsen.frames++] = htonl(0x08);

          }

        }
      }
    }
  }
#endif



#endif

