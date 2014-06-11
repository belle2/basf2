/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDUnpackerModule_H
#define PXDUnpackerModule_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDRawAdc.h>
#include <pxd/dataobjects/PXDRawPedestal.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace PXD {

#define ONSEN_MAX_TYPE_ERR  32

    /** \addtogroup modules
     * @{
     */

    /** The PXDUnpacker module.
     *
     * This module is responsible for unpacking the Raw PXD data to Pixels
     */
    class PXDUnpackerModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDUnpackerModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the unpacking */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    private:

      /**  Swap the endianess of the ONSEN header yes/no */
      bool m_headerEndianSwap;
      /**  ignore missing DATCON */
      bool m_ignoreDATCON;
      /** Only unpack, but Do Not Store anything to file */
      bool m_doNotStore;
      /** Event Number and compare mask grabbed from FTSW for now */
      unsigned int ftsw_evt_nr, ftsw_evt_mask;
      /** Event Number from MetaInfo */
      unsigned long m_meta_event_nr;
      /** Run Number from MetaInfo */
      unsigned long m_meta_run_nr;
      /** Subrun Number from MetaInfo */
      unsigned long m_meta_subrun_nr;
      /** Experiment from MetaInfo */
      unsigned long m_meta_experiment;

      /** Event counter */
      unsigned int m_unpackedEventsCount;
      /** Error counters */
      unsigned int m_errorCounter[ONSEN_MAX_TYPE_ERR];
      /** Output array for Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;
      /** Output array for Raw Hits. */
      StoreArray<PXDRawROIs> m_storeROIs;
      /** Output array for Raw Adcs. */
      StoreArray<PXDRawAdc> m_storeRawAdc;
      /** Output array for Raw Adc:Pedestals. */
      StoreArray<PXDRawPedestal> m_storeRawPedestal;

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
       * @param Frame_Number current frame number
       * @param Frames_in_event number of frames in PxdRaw object (subevent)
       */
      void unpack_dhhc_frame(void* data, const int len, const int Frame_Number, const int Frames_in_event);

      /** Unpack DHP data within one DHH frame
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhh_first_readout_frame_lo 16 bit of the first readout frame from DHH Start
       * @param dhh_ID raw DHH ID from DHHC frame
       * @param dhh_DHPport raw DHP port from DHHC frame
       * @param dhh_reformat flag if DHH did reformatting
       * @param toffset triggered row (offset)
       * @param vxd_id vertex Detector ID
       */
      void unpack_dhp(void* data, unsigned int len, unsigned int dhh_first_readout_frame_lo, unsigned int dhh_ID, unsigned dhh_DHPport, unsigned dhh_reformat, unsigned short toffset, VxdID vxd_id);

      /** Unpack DHP RAW data within one DHH frame (pedestals, etc)
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhh_ID raw DHH ID from DHHC frame
       * @param dhh_DHPport raw DHP port from DHHC frame
       * @param vxd_id vertex Detector ID
       */
      void unpack_dhp_raw(void* data, unsigned int len, unsigned int dhh_ID, unsigned dhh_DHPport, VxdID vxd_id);

      int nr5bits(int i) const;/// helper function to "count" nr of set bits within lower 5 bits


      /** Error Mask set per packet / event*/
      unsigned int m_errorMask;
      /** give verbose unpacking information -> will eb a parameter in next release */
      bool verbose = true;
      /** ignore missing datcon (dont show error) */
      bool ignore_datcon_flag = true;

    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerModule_H
