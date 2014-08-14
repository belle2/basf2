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

      /** Parameter dhhc<->dhh list, mapping from steering file */
      std::vector< std::vector<int >> m_dhh_to_dhhc;

//       /** mapping calculated from m_dhh_to_dhhc for easier handling */
//       std::map <int, int> dhh_mapto_dhhc;

      /** mapping calculated from m_dhh_to_dhhc for easier handling */
      std::map <int, std::vector <int>> m_dhhc_mapto_dhh;

      /** Event counter */
      unsigned int m_packed_events;
      /** Trigger Nr */
      unsigned int m_trigger_nr;
      /** Run+Subrun Nr */
      unsigned short m_run_nr_word1;
      /** Exp+Run Nr */
      unsigned short m_run_nr_word2;

      /** For one DHHC event, we utilize one header (writing out, beware of endianess!) */
      std::vector <unsigned int> m_onsen_header;

      /** For one DHHC event, we utilize one payload for all DHH/DHP frames */
      std::vector <std::vector <unsigned char>> m_onsen_payload;

      /** For current processed frames */
      std::vector <unsigned char> m_current_frame;

      /** Input array for Digits. */
      StoreArray<PXDDigit> storeDigits;
      /** Output array for RawPxds */
      StoreArray<RawPXD> m_storeRaws;

      /** Pack one event (several DHHC) stored in seperate RawPXD object.
       */
      void pack_event(void);

      /** Pack one DHHC (several DHH) stored in one RawPXD object.
       */
      void pack_dhhc(int dhhc_id, int dhh_mask, int* dhh_ids);

      /** Pack one DHH (several DHP) to buffer.
       */
      void pack_dhh(int dhh_id, int dhp_mask);

      /** Pack one DHP to buffer.
       */
      void pack_dhp(int dhp_id, int dhh_id, int dhh_reformat);

      /** Pack one DHP RAW to buffer.
       */
      void pack_dhp_raw(int dhp_id, int dhh_id, bool adcpedestal);

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

      /** Store start of Vxd Detector related digits */
      std::map <VxdID , int> startOfVxdID;

      unsigned char halfladder_pixmap[PACKER_NUM_ROWS][PACKER_NUM_COLS];//! temporary hitmap buffer for pixel to raw data conversion

      // ignore common mode for now...

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDPackerModule_H
