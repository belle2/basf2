/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDPackerModule_H
#define PXDPackerModule_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <pxd/dataobjects/PXDDigit.h>

namespace Belle2 {

  namespace PXD {

#define PACKER_NUM_ROWS 768
#define PACKER_NUM_COLS 250


    /** The PXDPacker module.
     *
     * This module is responsible for packing (simulated) Pixels back to
     * Raw PXD data. This is usefull as input for ONSEN as well
     * as for rechecking the unpacking process with well defined data.
     * Not yet ready for MC Production, only lab use recommended.
     */
    class PXDPackerModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDPackerModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the packing */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    private:

      std::string m_PXDDigitsName;  /**< The name of the StoreArray of PXDDigits to be processed */
      std::string m_RawPXDsName;  /**< The name of the StoreArray of generated RawPXDs */

      unsigned int dhe_byte_count;/**< Byte count in current DHE package */
      unsigned int dhc_byte_count;/**< Byte count in current DHC package */

      /** Parameter dhc<->dhe list, mapping from steering file */
      std::vector< std::vector<int >> m_dhe_to_dhc;

//       /** mapping calculated from m_dhe_to_dhc for easier handling */
//       std::map <int, int> dhe_mapto_dhc;

      /** mapping calculated from m_dhe_to_dhc for easier handling */
      std::map <int, std::vector <int>> m_dhc_mapto_dhe;

      /** Event counter */
      unsigned int m_packed_events;
      /** Trigger Nr */
      unsigned int m_trigger_nr;
      /** Run+Subrun Nr */
      unsigned short m_run_nr_word1;
      /** Exp+Run Nr */
      unsigned short m_run_nr_word2;
      /** Time(Tag) from MetaInfo */
      unsigned long long int m_meta_time;

      /** For one DHC event, we utilize one header (writing out, beware of endianess!) */
      std::vector <unsigned int> m_onsen_header;

      /** For one DHC event, we utilize one payload for all DHE/DHP frames */
      std::vector <std::vector <unsigned char>> m_onsen_payload;

      /** For current processed frames */
      std::vector <unsigned char> m_current_frame;

      /** Input array for Digits. */
      StoreArray<PXDDigit> storeDigits;
      /** Output array for RawPxds */
      StoreArray<RawPXD> m_storeRaws;

      /** Pack one event (several DHC) stored in seperate RawPXD object.
       */
      void pack_event(void);

      /** Pack one DHC (several DHE) stored in one RawPXD object.
       */
      void pack_dhc(int dhc_id, int dhe_mask, int* dhe_ids);

      /** Pack one DHE (several DHP) to buffer.
       */
      void pack_dhe(int dhe_id, int dhp_mask);

      /** Pack one DHP to buffer.
       */
      void pack_dhp(int dhp_id, int dhe_id, int dhe_reformat);

      /** Pack one DHP RAW to buffer.
       */
      void pack_dhp_raw(int dhp_id, int dhe_id, bool adcpedestal);

      /** Swap endianes inside all shorts of this frame besides CRC.
       * @param data pointer to frame
       * @param len length of frame
       */
      void endian_swap_frame(unsigned short* data, int len);

      void start_frame(void);//! Start with a new Frame
      void append_int8(unsigned char w);//! cat 8bit value to frame
      void append_int16(unsigned short w);//! cat 16bit value to frame
      void append_int32(unsigned int w);//! cat 32value value to frame
      void add_frame_to_payload(void);//! Add Frame to Event payload

      void do_the_reverse_mapping(unsigned int& row, unsigned int& col, unsigned short layer, unsigned short sensor);

      /** Store start of Vxd Detector related digits */
      std::map <VxdID , int> startOfVxdID;

      unsigned char halfladder_pixmap[PACKER_NUM_ROWS][PACKER_NUM_COLS];//! temporary hitmap buffer for pixel to raw data conversion

      // ignore common mode for now...

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDPackerModule_H
