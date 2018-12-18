/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <pxd/dataobjects/PXDDAQStatus.h>

namespace Belle2 {

  namespace PXD {

    /** The PXD bad sensor tagger module.
     *
     * This module flags sensors/modules (by DHE id) with broken data content
     * (e.g. daq says data is o.k., but content is crap
     *
     */

    class PXDBadSensorTagModule : public Module {

    public:
      /** Constructor */
      PXDBadSensorTagModule();

    private:

      /** Initialize the module */
      void initialize() override final;
      void event() override final;

      std::string m_PXDDAQEvtStatsName;  /**< The name of the StoreObjPtr of PXDDAQStatus to be generated */
      std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be generated */

      /** Input array for PXD Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;
      /** Output array for DAQ Status. */
      StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;

      /** Cuts fore each module */
      std::map <VxdID, int> m_cut;
      /** Minimum charge for a digit to carry */
      int m_0cut;

    };//end class declaration

  } //end PXD namespace;
} // end namespace Belle2
