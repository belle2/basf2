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
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <framework/dataobjects/EventMetaData.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TLine.h>
#include <string>


namespace Belle2 {

  namespace PXD {
    /**
     * Plot each event with ROI and Pixels
     */
    class PXDEventPlotModule : public Module {

    public:

      /** Constructor defining the parameters */
      PXDEventPlotModule();

    private:
      /** Initialize */
      void initialize() override final;

      /** Event */
      void event() override final;

    private:

      VXD::GeoCache& m_vxdGeometry; /**< the geometry */

      std::string m_storeRawHitsName; /**< PXDRawHits StoreArray name */
      bool m_gateModeFlag{false}; /**< Flag for creating extra GM plots */

      StoreObjPtr<EventMetaData> m_eventMetaData;/**< Event Metadata StorePtr */

      /** Input array for DAQ Status. */
      StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;

      /** Storearray for raw pixels   */
      StoreArray<PXDRawHit> m_storeRawHits;

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;

      TCanvas* m_c{};  /**< list of canvases */
      std::map <VxdID, TH2F*> m_histos; /**< map for plots per modules */
      std::map <VxdID, TH2F*> m_histos_gm; /**< map for plots per modules with gm+ */
      std::map <VxdID, TH2F*> m_histos_gm2; /**< map for plots per modules with gm- */

      TLine* m_l1{}, /**< Lines to guide the eye in plots/canvases */
             *m_l2{}, /**< Lines to guide the eye in plots/canvases */
             *m_l3{}; /**< Lines to guide the eye in plots/canvases */

    };//end class declaration

  } //end PXD namespace;
} // end namespace Belle2

