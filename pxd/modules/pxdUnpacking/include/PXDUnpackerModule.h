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
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {

  namespace PXD {
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

    private:

      /**  Swap the endianess of the ONSEN header yes/no */
      bool m_headerEndianSwap;
      /**  Run in DHHC mode yes/no */
      bool m_DHHCmode;
      /**  ignore missing DATCON */
      bool m_ignoreDATCON;
      /** Event Number and compare mask grabbed from FTSW for now */
      unsigned int ftsw_evt_nr, ftsw_evt_mask;
      /** Output array for Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;

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
      void unpack_dhhc_frame(void* data, int len, bool pad, int& last_wie, unsigned int& last_evtnr, int Frame_Number, int Frames_in_event);

      /** Unpack DHP data within one DHH frame
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhh_first_readout_frame_lo 16 bit of the first readout frame from DHH Start
       * @param dhh_ID raw DHH ID from DHHC frame
       * @param dhh_DHPport raw DHP port from DHHC frame
       * @param dhh_reformat flag if DHH did reformatting
       * @param toffset triggered row (offset)
       */
      void unpack_dhp(void* data, unsigned int len, unsigned int dhh_first_readout_frame_lo, unsigned int dhh_ID, unsigned dhh_DHPport, unsigned dhh_reformat, unsigned short toffset);

    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerModule_H
