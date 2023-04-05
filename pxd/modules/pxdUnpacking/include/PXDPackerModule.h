/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDInjectionBGTiming.h>

#include <pxd/dbobjects/PXDDHHFirmwareVersionPar.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  namespace PXD {

    /** The PXDPacker module.
     *
     * This module is responsible for packing (simulated) Pixels back to
     * Raw PXD data. This is usefull as input for ONSEN as well
     * as for rechecking the unpacking process with well defined data.
     * Not yet ready for MC Production, only lab use recommended.
     */
    class PXDPackerModule : public Module {
      enum {PACKER_NUM_ROWS = 768};
      enum {PACKER_NUM_COLS = 250};

    public:
      /** Constructor defining the parameters */
      PXDPackerModule();

    private:
      /** Initialize the module */
      void initialize() override final;
      /** begin run */
      void beginRun() override final;
      /** do the packing */
      void event() override final;
      /** Terminate the module */
      void terminate() override final;

      std::string m_PXDDigitsName;  /**< The name of the StoreArray of PXDDigits to be processed */
      std::string m_RawPXDsName;  /**< The name of the StoreArray of generated RawPXDs */
      std::string m_InjectionBGTimingName;  /**< The name of the StoreObj InjectionBGTiming */

      bool m_InvertMapping; /**< Flag if we invert mapping to DHP row/col or use premapped coordinates */
      bool m_Clusterize; /**< Use clusterizer (FCE format) */

      /** Parameter dhc<->dhe list, mapping from steering file */
      std::vector< std::vector<int >> m_dhe_to_dhc;

//       /** mapping calculated from m_dhe_to_dhc for easier handling */
//       std::map <int, int> dhe_mapto_dhc;

      /** mapping calculated from m_dhe_to_dhc for easier handling */
      std::map <int, std::vector <int>> m_dhc_mapto_dhe;

      /** Event counter */
      unsigned int m_packed_events{0};
      /** Trigger Nr */
      unsigned int m_trigger_nr{0};
      /** Run+Subrun Nr */
      unsigned short m_run_nr_word1{0};
      /** Exp+Run Nr */
      unsigned short m_run_nr_word2{0};
      /** Time(Tag) from MetaInfo */
      unsigned long long int m_meta_time{0};

      /** DHP Readout Frame Nr for DHP and DHE headers */
      unsigned int m_trigger_dhp_framenr{0};
      /** DHE Trigger Gate for DHE headers */
      unsigned int m_trigger_dhe_gate{0};

      /** For one DHC event, we utilize one header (writing out, beware of endianess!) */
      std::vector <unsigned int> m_onsen_header;

      /** For one DHC event, we utilize one payload for all DHE/DHP frames */
      std::vector <std::vector <unsigned char>> m_onsen_payload;

      /** For current processed frames */
      std::vector <unsigned char> m_current_frame;

      /** Input array for Digits. */
      StoreArray<PXDDigit> m_storeDigits;
      /** Output array for RawPxds */
      StoreArray<RawPXD> m_storeRaws;
      /** Input Obj InjectionBGTiming */
      StoreObjPtr<PXDInjectionBGTiming> m_storeInjectionBGTiming;

      /** Firmware version, must be read from database on run change */
      int m_firmware{0};

      /** firmware version from DB. */
      OptionalDBObjPtr<PXDDHHFirmwareVersionPar> m_firmwareFromDB{"PXDDHHFirmwareVersion"};
      /** override firmware version from DB. */
      int m_overrideFirmwareVersion{0};

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
      void pack_dhp(int dhp_id, int dhe_id, int dhe_reformat, int startrow = 0);

      /** Pack one DHP RAW to buffer.
       */
      /* cppcheck-suppress unusedPrivateFunction */
      void pack_dhp_raw(int dhp_id, int dhe_id);

      void start_frame(void); ///< Start with a new Frame
      /* cppcheck-suppress unusedPrivateFunction */
      void append_int8(unsigned char w); ///< cat 8bit value to frame
      void append_int16(unsigned short w); ///< cat 16bit value to frame
      void append_int32(unsigned int w); ///< cat 32value value to frame
      void add_frame_to_payload(void); ///< Add Frame to Event payload

      /** function still to be implemented */
      void do_the_reverse_mapping(unsigned int& row, unsigned int& col, unsigned short layer, unsigned short sensor);

      /** Store start of Vxd Detector related digits */
      std::map <VxdID, int> startOfVxdID;

      /** temporary hitmap buffer for pixel to raw data conversion */
      unsigned char halfladder_pixmap[PACKER_NUM_ROWS][PACKER_NUM_COLS] {{0}};

      unsigned int dhe_byte_count{0}; /**< Byte count in current DHE package */
      unsigned int dhc_byte_count{0}; /**< Byte count in current DHC package */

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
