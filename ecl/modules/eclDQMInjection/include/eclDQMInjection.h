/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/mapper/ECLChannelMapper.h>

/* Basf2 headers. */
#include <framework/core/HistoModule.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <rawdata/dataobjects/RawFTSW.h>

/* ROOT headers. */
#include <TH1.h>
#include <TH2.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  class ECLDsp;

  namespace ECL {
    /** The ECL Occ after Injection DQM module.
     *
     * Occ after Injection (taken from TTD packet)
     */
    class ECLDQMInjectionModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      ECLDQMInjectionModule();

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
      std::string m_ECLDigitsName;  /**< The name of the StoreArray of ECLRawHits to be generated */
      double m_revolutionTime;  /**< The beam revolution cycle time in \f$\mu s\f$ */
      double m_ECLThresholdforVetoTuning; /**< ECL threshold for injection veto tuning, ADC channels */


      /** StoreObjPtr EventMetaData */
      StoreObjPtr<EventMetaData> m_eventmetadata;
      /** StoreObjPtr TRGSummary  */
      StoreObjPtr<TRGSummary> m_l1Trigger;
      /** ECL channel mapper */
      ECLChannelMapper mapper;

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;
      /** Input array for ECL Raw Hits. */
      StoreArray<ECLDigit> m_storeHits;
      /** Input array for ECL burst suppresions. */
      StoreArray<ECLTrig> m_ECLTrigs;
      /** Input array for ECL waveform data */
      StoreArray<ECLDsp> m_ECLDsps;

      /** PSD waveform amplitude threshold. */
      DBObjPtr<ECLCrystalCalib> m_calibrationThrApsd;

      /** Global event number. */
      int m_iEvent{ -1};
      /** Flag to select events triggered by delayed bhabha. */
      bool m_DPHYTTYP{0};



      /** Vector to store psd wf amplitude threshold. */
      std::vector<int> v_totalthrApsd = {};


      TH1F* hHitsAfterInjLER{};         /**< Histogram Hits after LER injection */
      TH1F* hHitsAfterInjHER{};         /**< Histogram Hits after HER injection */

      TH1F* hEHitsAfterInjLER{};        /**< Histogram for Nr Entries (=Triggrs) for normalization after LER injection */
      TH1F* hEHitsAfterInjHER{};        /**< Histogram for Nr Entries (=Triggrs) for normalization after HER injection */

      TH1F* hBurstsAfterInjLER{};       /**< Histogram Bursts suppression after LER injection */
      TH1F* hBurstsAfterInjHER{};       /**< Histogram Bursts suppression after HER injection */

      TH1F* hEBurstsAfterInjLER{};      /**< Histogram Bursts suppression for normalization after LER injection */
      TH1F* hEBurstsAfterInjHER{};      /**< Histogram Bursts suppression for normalization after HER injection */

      TH2F* hVetoAfterInjLER{};         /**< Histogram Veto tuning w/ ECL hits after LER injection */
      TH2F* hVetoAfterInjHER{};         /**< Histogram Veto tuning w/ ECL hits after HER injection */

      TH2F* hOccAfterInjLER{};          /**< Histogram Occupancy after LER injection */
      TH2F* hOccAfterInjHER{};          /**< Histogram Occupancy after HER injection */

      TH2F* hInjkickTimeShift[2] = {};  /**< Histograms to determine injkick signal time offset for LER/HER injections */

      /**
       * Injection time range (in ms) for h_ped_peak histograms
       */
      std::vector<float> m_ped_peak_range = {};
      /**
       * Distribution of pedestal peak (peak in first 16 waveform samples)
       * after HER/LER injection, with separate histograms for forward endcap,
       * barrel and backward, also separated by time range after the injection
       * (m_ped_peak_range defines these time ranges)
       */
      std::vector<TH1F*> h_ped_peak = {};

      void initialize() override final; /**< initialize function */

      void beginRun() override final;  /**< beginRun function */

      void event() override final; /**< event function */

      void defineHisto() override final; /**< defineHisto function */

    };//end class declaration


  } //end ECL namespace;
} // end namespace Belle2
