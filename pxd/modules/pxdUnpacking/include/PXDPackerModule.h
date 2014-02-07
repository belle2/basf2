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
//#include <pxd/dataobjects/PXDRawHit.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace PXD {

    /** \addtogroup modules
     * @{
     */

    /** The PXDPacker module.
     *
     * This module is responsible for packing the Raw PXD data to Pixels
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

      /** Event counter */
      unsigned int m_packed_events;
      /** Trigger Nr */
      unsigned int m_trigger_nr;

      /** For one DHHC event, we utilize one header (writing out, beware of endianess!) */
      std::vector <unsigned int> m_onsen_header;

      /** For one DHHC event, we utilize one payload for all DHH/DHP frames */
      std::vector <std::vector <unsigned char>> m_onsen_payload;

      /** For current processed frames */
      std::vector <unsigned char> m_current_frame;

      /** Input array for Raw Hits. */
      //StoreArray<PXDRawHit> m_storeRawHits;
      /** Output array for RawPxds */
      StoreArray<RawPXD> m_storeRaws;
      /** Pack one event (several DHHC) stored in seperate RawPXD object.
       */
      void pack_event(void);

      /** Pack one DHHC (several DHH) stored in one RawPXD object.
       */
      void pack_dhhc(int dhhc_id, int dhh_mask);

      /** Pack one DHH (several DHP) to buffer.
       */
      void pack_dhh(int dhh_id, int dhp_mask);

      /** Pack one DHP to buffer.
       */
      void pack_dhp(int dhp_id, int dhh_id);

      /** Swap endianes inside all shorts of this frame besides CRC.
       * @param data pointer to frame
       * @param len length of frame
       */
      void endian_swap_frame(unsigned short* data, int len);

      void start_frame(void);
      void add_int16(unsigned short w);
      void add_int32(unsigned int w);
      void add_frame_to_payload(void);

    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDPackerModule_H
