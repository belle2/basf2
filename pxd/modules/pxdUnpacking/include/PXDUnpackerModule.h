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
#include <boost/format.hpp>
#include <string>
#include <deque>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/RawPXD.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>




namespace Belle2 {

  namespace PXD {
    /** \addtogroup modules
     * @{
     */

    /** The PXDUnpacker module.
     *
     * This module is responsible to cluster all hits found in the PXD and
     * write them to the apropriate collections. It does this in a "streaming" way:
     * The hits are examined in an ordered way (sorted by row, then by column) and for each
     * pixel we only have to check the left neighbor and the three adjacent
     * hits in the last row. By caching the last row, each pixel gets examined
     * only once and the 4 adjacent pixels are accessed in constant time.
     * @see ClusterCache
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
      void unpack_event(RawPXD* px);

      /** Unpack one frame (within an event).
       * @param data pointer to frame
       * @param len length of frame
       */
      void unpack_frame(void* data, int len, int pad, int& last_framenr, int& last_wie, int& last_start, int& last_end, unsigned int& last_evtnr);

      void fill_pixelmap(void* data, unsigned int len, unsigned int dhh_first_frame_id_lo, unsigned int dhh_ID, unsigned short toffset);

      int format_raw_from_dhp(void* dhp_in, int anzahl, void* raw_out, int& raw_anzahl, bool printflag, bool commode);

      /** Write clusters to collection.
       * This method will check all cluster candidates and write valid ones to the datastore
       */
      /** Name of the collection to use for the PXDDigits */
      //Pxd
      std::string m_storeRAWPxdName;
      std::string m_storeRawHitsName;

    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerModule_H
