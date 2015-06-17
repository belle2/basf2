/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDUnpackerDesy1314Module_H
#define PXDUnpackerDesy1314Module_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace PXD {

#define ONSEN_MAX_TYPE_ERR  32


    /** The PXDUnpackerDesy1314 module.
     *
     * This module is responsible for unpacking the Raw PXD data to Pixels
     */
    class PXDUnpackerDesy1314Module : public Module {

    public:
      /** Constructor defining the parameters */
      PXDUnpackerDesy1314Module();

      /** Initialize the module */
      virtual void initialize();
      /** do the unpacking */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    private:

      /**  Swap the endianess of the ONSEN header yes/no */
      bool m_headerEndianSwap;
      /**  Run in DHC mode yes/no */
      bool m_DHCmode;
      /**  ignore missing DATCON */
      bool m_ignoreDATCON;
      /**  ignore wrong nr of frames info in DHC Start*/
      bool m_ignore_headernrframes;
      /**  ignore missing DHPs from DHP mask in DHE Start*/
      bool m_ignore_dhpmask;
      /**  ignore wrong DHP size for empty dhp frames (bug in Davids core) and checksum */
      bool m_ignore_empty_dhp_wrong_size;
      /**  ignore wrong DHP ports in DHE header vs DHP header */
      bool m_ignore_dhpportdiffer;
      /** Only unpack, but Do Not Store anything to file */
      bool m_doNotStore;
      /** Event Number and compare mask grabbed from FTSW for now */
      unsigned int ftsw_evt_nr, ftsw_evt_mask;
      /** Event counter */
      unsigned int unpacked_events;
      /** Error counters */
      unsigned int error_counter[ONSEN_MAX_TYPE_ERR];
      /** Output array for Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;
      /** Output array for Raw Hits. */
      StoreArray<PXDRawROIs> m_storeROIs;

      /** Unpack one event (several frames) stored in RawPXD object.
       * @param px RawPXD data object
       */
      void unpack_event(RawPXD& px);

      /** Swap endianes inside all shorts of this frame besides CRC.
       * @param data pointer to frame
       * @param len length of frame
       */
      void endian_swap_frame(unsigned short* data, int len);

      /** Unpack one frame (within an event).
       * @param data pointer to frame
       * @param len length of frame
       */
      void unpack_dhc_frame(void* data, int len, bool pad, int& last_wie, unsigned int& last_evtnr, int Frame_Number,
                            int Frames_in_event);

      /** Unpack DHP data within one DHE frame
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhe_first_readout_frame_lo 16 bit of the first readout frame from DHE Start
       * @param dhe_ID raw DHE ID from DHC frame
       * @param dhe_DHPport raw DHP port from DHC frame
       * @param dhe_reformat flag if DHE did reformatting
       * @param toffset triggered row (offset)
       */
      void unpack_dhp(void* data, unsigned int len, unsigned int dhe_first_readout_frame_lo, unsigned int dhe_ID, unsigned dhe_DHPport,
                      unsigned dhe_reformat, unsigned short toffset, VxdID vxd_id);

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerDesy1314Module_H
