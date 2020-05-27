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
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <TH1.h>
#include <TH2.h>
#include <string>
#include <map>

namespace Belle2 {

  namespace PXD {
    /** The PXD Occupancy after Injection DQM module.
     *
     * Fill Histograms PXD with Occupancy (and nr of triggers) after
     * Injection with time (ticks) taken from TTD packet
     */
    class PXDInjectionDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDInjectionDQMModule();

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
      std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits */
      std::string m_PXDClustersName;  /**< The name of the StoreArray of PXDClusters */

      bool m_eachModule{false}; ///< create a histo per module
      bool m_offlineStudy{false}; ///< create histos with much finer binning and larger range
      bool m_useClusters{false}; ///< use PXDClusters instead of Raw Hits
      bool m_createMaxHist{false};///< create max hits histogram, not multi processing save!!

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;

      /** Input array for PXD Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;

      /** Input array for PXD Clusters. */
      StoreArray<PXDCluster> m_storeClusters;

      /** the VXD geometry */
      VXD::GeoCache& m_vxdGeometry;

      TH1F* hOccAfterInjLER{};          /**< Histogram Occupancy after LER injection */
      TH1F* hOccAfterInjHER{};          /**< Histogram Occupancy after HER injection */

      std::map<VxdID, TH1F*> hOccModAfterInjLER; /**< Histogram Occupancy after LER injection */
      std::map<VxdID, TH1F*> hOccModAfterInjHER; /**< Histogram Occupancy after HER injection */

      TH1F* hEOccAfterInjLER{};          /**< Histogram for Nr Entries (=Triggrs) for normalization after LER injection */
      TH1F* hEOccAfterInjHER{};          /**< Histogram for Nr Entries (=Triggrs) for normalization after HER injection */

      TH1F* hMaxOccAfterInjLER{};          /**< Histogram Max Occupancy after LER injection */
      TH1F* hMaxOccAfterInjHER{};          /**< Histogram Max Occupancy after HER injection */

      std::map<VxdID, TH1F*> hMaxOccModAfterInjLER; /**< Histogram Max Occupancy after LER injection */
      std::map<VxdID, TH1F*> hMaxOccModAfterInjHER; /**< Histogram Max Occupancy after HER injection */

      TH2F* hOccAfterInjLERGate{};          /**< Occupancy after LER injection per Gate */
      TH2F* hOccAfterInjHERGate{};          /**< Occupancy after HER injection per Gate */

      void initialize() override final; /**< initialize function */

      void beginRun() override final;  /**< beginRun function */

      void event() override final; /**< event function */

      void defineHisto() override final; /**< defineHisto function */

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
