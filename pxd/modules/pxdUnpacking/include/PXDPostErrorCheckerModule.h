/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDRawAdc.h>
#include <pxd/dataobjects/PXDRawROIs.h>

namespace Belle2 {

  namespace PXD {
    /** The PXD DAQ Post Unpacking Error Check.
     *
     * does a few error checks which cannot be done during unpacking
     */
    class PXDPostErrorCheckerModule : public Module {

    public:

      /** Constructor defining the parameters */
      PXDPostErrorCheckerModule();

    private:
      void initialize() override final;

      void event() override final;

    private:
      std::string m_PXDDAQEvtStatsName;  /**< The name of the StoreObjPtr of PXDDAQStatus to be generated */
      std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be generated */
      std::string m_PXDRawAdcsName;  /**< The name of the StoreArray of PXDRawAdcs to be generated */
      std::string m_PXDRawROIsName;  /**< The name of the StoreArray of PXDRawROIs to be generated */

      /** Critical error mask which defines return value of task */
      uint64_t m_criticalErrorMask; // TODO this should be type PXDErrorFlag .. but that does not work with addParam()

      /** Flag: Ignore different triggergate between DHEs */
      bool m_ignoreTriggerGate;
      /** Flag: Ignore different dhp frame between DHEs */
      bool m_ignoreDHPFrame;

      /** Input array for DAQ Status. */
      StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;
      /** Output array for Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;
      /** Output array for Raw ROIs. */
      StoreArray<PXDRawROIs> m_storeROIs;
      /** Output array for Raw Adcs. */
      StoreArray<PXDRawAdc> m_storeRawAdc;
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
