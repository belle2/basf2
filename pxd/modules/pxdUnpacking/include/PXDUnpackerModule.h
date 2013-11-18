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
#include <pxd/dataobjects/RawPXD.h>
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
      void unpack_frame(void* data, int len, bool pad, int& last_framenr, int& last_wie, int& last_start, int& last_end, unsigned int& last_evtnr);

      /** Unpack DHP data within one DHH frame
       * @param data pointer to dhp data
       * @param len length of dhp data
       */
      void unpack_dhp(void* data, unsigned int len, unsigned int dhh_first_frame_id_lo, unsigned int dhh_ID, unsigned short toffset);


      /** Write clusters to collection.
       * This method will check all cluster candidates and write valid ones to the datastore
       */
      /** Name of the collection to use for the PXDDigits */
      //Pxd

      /** Output array. */
      StoreArray<PXDRawHit> m_storeRawHits;

    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerModule_H
