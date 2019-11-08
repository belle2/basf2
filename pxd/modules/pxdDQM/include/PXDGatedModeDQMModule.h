/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <TH1.h>
#include <TH2F.h>
#include <string>
#include <map>

namespace Belle2 {

  namespace PXD {
    /** The PXD for GatedMode DQM module.
     *
     * PXD Occupancy etc for GatedMode Histogramming with
     * info taken from TTD packet
     *
     */
    class PXDGatedModeDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDGatedModeDQMModule();

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
      std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be generated */

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;

      /** Input array for PXD Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;

      /** the VXD geometry */
      VXD::GeoCache& m_vxdGeometry;


      TH1F* hBunchLER, *hBunchHER;

      std::map<std::pair<VxdID, int>, TH2F*> hGatedModeMapLER; /**< Histogram Occupancy after LER injection */
      std::map<std::pair<VxdID, int>, TH2F*> hGatedModeMapHER; /**< Histogram Occupancy after HER injection */

      std::map<std::pair<VxdID, int>, TH2F*> hGatedModeMapCutLER; /**< Histogram Occupancy after LER injection */
      std::map<std::pair<VxdID, int>, TH2F*> hGatedModeMapCutHER; /**< Histogram Occupancy after HER injection */

      std::map<VxdID, TH2F*> hGatedModeMapSubLER; /**< Histogram Occupancy after LER injection */
      std::map<VxdID, TH2F*> hGatedModeMapSubHER; /**< Histogram Occupancy after HER injection */

      std::map<VxdID, TH2F*> hGatedModeMapAddLER; /**< Histogram Occupancy after LER injection */
      std::map<VxdID, TH2F*> hGatedModeMapAddHER; /**< Histogram Occupancy after HER injection */

      std::map<VxdID, TH2F*> hGatedModeProjLER; /**< Histogram Occupancy after LER injection */
      std::map<VxdID, TH2F*> hGatedModeProjHER; /**< Histogram Occupancy after HER injection */

      void initialize() override final; /**< initialize function */

      void beginRun() override final;  /**< beginRun function */

      void event() override final; /**< event function */

      void defineHisto() override final; /**< defineHisto function */

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
