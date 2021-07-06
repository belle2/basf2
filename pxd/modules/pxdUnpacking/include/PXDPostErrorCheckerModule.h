/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * Does a few error checks which cannot be done during unpacking.
     * Including double data checks and missing modules.
     * In this process it is decided if an event is "good" for analysis.
     * All data for "broken" events is deleted. (Add: we might decide
     * for a finer granularity, e.g. module level. The data is then
     * only tagged as bad, but not removed). PXDHitSorter etc must check
     * the tags and exclude Hits before clustering.
     *
     */
    class PXDPostErrorCheckerModule : public Module {

    public:

      /** Constructor defining the parameters */
      PXDPostErrorCheckerModule();

    private:
      /** Initialize */
      void initialize() override final;

      /** Event */
      void event() override final;

    private:
      std::string m_PXDDAQEvtStatsName;  /**< The name of the StoreObjPtr of PXDDAQStatus to be read and modified */
      std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be modified */
      std::string m_PXDRawAdcsName;  /**< The name of the StoreArray of PXDRawAdcs to be modified */
      std::string m_PXDRawROIsName;  /**< The name of the StoreArray of PXDRawROIs to be modified */

      /** Critical error mask which defines when data should be trashed, whole event only! */
      PXDError::PXDErrorFlags  m_criticalErrorMask; // TODO this does not work with addParam()
      /// TODO another mask for DHE Level if we want to clean ONLY modules?

      /** Flag: Ignore different triggergate between DHEs */
      bool m_ignoreTriggerGate;
      /** Flag: Ignore different dhp frame between DHEs */
      bool m_ignoreDHPFrame;

      /** Input array for DAQ Status. */
      StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;
      /** In/Output array for Raw Hits. Only touched if data needs to be removed*/
      StoreArray<PXDRawHit> m_storeRawHits;
      /** In/Output array for Raw ROIs. Only touched if data needs to be removed */
      StoreArray<PXDRawROIs> m_storeROIs;
      /** In/Output array for Raw Adcs. Only touched if data needs to be removed */
      StoreArray<PXDRawAdc> m_storeRawAdc;
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
