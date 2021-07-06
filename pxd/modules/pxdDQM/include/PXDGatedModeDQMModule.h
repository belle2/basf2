/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <TH2.h>
#include <string>
#include <map>

namespace Belle2 {

  namespace PXD {
    /** The PXD for GatedMode DQM module.
     *
     * PXD Occupancy etc for GatedMode Histogramming with
     * injection info taken from TTD packet
     *
     */
    class PXDGatedModeDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDGatedModeDQMModule();

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
      std::string m_PXDRawHitsName; /**< The name of the StoreArray of PXDRawHits to be generated */
      bool m_perGate; /**< make one plot per possible gate */
      int m_minTimeCutLER; /**< minimum time cut in us after LER kick */
      int m_maxTimeCutLER; /**< maximum time cut in us after LER kick */
      int m_minTimeCutHER; /**< minimum time cut in us after HER kick */
      int m_maxTimeCutHER; /**< maximum time cut in us after HER kick */
      int m_outsideTimeCut; /**< outside GM time cut in us after kick */
      int m_chargeCut; /**< minimum charge cut in ADU */
      int m_chargeCutHigh; /**< minimum charge cut for second set of histograms in ADU */

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;

      /** Input array for PXD Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;

      /** the VXD geometry */
      VXD::GeoCache& m_vxdGeometry;


      /** Histo for last LER injected bunches */
      TH1F* hBunchInjLER = nullptr;
      /** Histo for last HER injected bunches */
      TH1F* hBunchInjHER = nullptr;
      /** Histo for triggered bunches */
      TH1F* hBunchTrg = nullptr;

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

      std::map<std::pair<VxdID, int>, TH2F*> hGatedModeMapADCLER; /**< Histogram Occupancy after LER injection */
      std::map<std::pair<VxdID, int>, TH2F*> hGatedModeMapADCHER; /**< Histogram Occupancy after HER injection */

      std::map<std::pair<VxdID, int>, TH2F*> hGatedModeMapCutADCLER; /**< Histogram Occupancy after LER injection */
      std::map<std::pair<VxdID, int>, TH2F*> hGatedModeMapCutADCHER; /**< Histogram Occupancy after HER injection */

      std::map<VxdID, TH2F*> hGatedModeMapSubADCLER; /**< Histogram Occupancy after LER injection */
      std::map<VxdID, TH2F*> hGatedModeMapSubADCHER; /**< Histogram Occupancy after HER injection */

      std::map<VxdID, TH2F*> hGatedModeMapAddADCLER; /**< Histogram Occupancy after LER injection */
      std::map<VxdID, TH2F*> hGatedModeMapAddADCHER; /**< Histogram Occupancy after HER injection */

      std::map<VxdID, TH2F*> hGatedModeProjADCLER; /**< Histogram Occupancy after LER injection */
      std::map<VxdID, TH2F*> hGatedModeProjADCHER; /**< Histogram Occupancy after HER injection */

      void initialize() override final; /**< initialize function */

      void beginRun() override final;  /**< beginRun function */

      void event() override final; /**< event function */

      void defineHisto() override final; /**< defineHisto function */

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
