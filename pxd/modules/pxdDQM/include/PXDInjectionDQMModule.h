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

#include <framework/core/HistoModule.h>
// #include <pxd/dataobjects/PXDDAQStatus.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawFTSWFormat_latest.h>
#include <TH2.h>
#include <TH1.h>
#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  namespace PXD {
    /** The PXD Occ after Injection DQM module.
     *
     * Occ after Injection (taken from TTD packet)
     */
    class PXDInjectionDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDInjectionDQMModule();

      void initialize() override final;

      void beginRun() override final;

      void event() override final;

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
      bool m_eachModule{false};// create a histo per module

      /** Input array for DAQ Status. */
//       StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;
      StoreObjPtr<EventMetaData> m_evtPtr;
      StoreArray<RawFTSW> m_rawTTD;

      //the geometry
      VXD::GeoCache& m_vxdGeometry;

      TH1F* hOccAfterInjLER{};          /** Occupancy after LER injection */
      TH1F* hOccAfterInjHER{};          /** Occupancy after HER injection */
      std::map<VxdID, TH1F*> hOccModAfterInjLER;/** Occupancy after LER injection */
      std::map<VxdID, TH1F*> hOccModAfterInjHER;/** Occupancy after HER injection */

      void defineHisto() override final;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
