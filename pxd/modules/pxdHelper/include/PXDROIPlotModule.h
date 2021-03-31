/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <tracking/dataobjects/ROIid.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <string>

namespace Belle2 {

  namespace PXD {
    /**
     * Plot each event with ROI and Pixels
     */
    class PXDROIPlotModule : public Module {

    public:

      /** Constructor defining the parameters */
      PXDROIPlotModule();

    private:
      /** Initialize */
      void initialize() override final;

      /** Event */
      void event() override final;

    private:
      std::string m_storeRawHitsName;             /**< PXDRawHits StoreArray name */
      std::string m_ROIsName;        /**< ROIs StoreArray name */
      std::string m_DCROIsName;        /**< DC ROIs StoreArray name */
      std::string m_HLTROIsName;        /**< HLT ROIs StoreArray name */
      std::string m_prefix; /**< prefix for plot names */

      StoreObjPtr<EventMetaData> m_eventMetaData;/**< Event Metadata StorePtr */

      /** Storearray for raw pixels   */
      StoreArray<PXDRawHit> m_storeRawHits;

      /** Storearray for ROIs  */
      StoreArray<ROIid> m_storeROIs;
      /** Storearray for ROIs (DC) */
      StoreArray<ROIid> m_storeDCROIs;
      /** Storearray for ROIs (HLT) */
      StoreArray<ROIid> m_storeHLTROIs;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

