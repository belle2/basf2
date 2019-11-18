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
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <TH1.h>
#include <string>

namespace Belle2 {

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

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;

      /** Input array for ECL Raw Hits. */
      StoreArray<ECLDigit> m_storeHits;

      TH1F* hOccAfterInjLER{};          /**< Histogram Occupancy after LER injection */
      TH1F* hOccAfterInjHER{};          /**< Histogram Occupancy after HER injection */

      TH1F* hEOccAfterInjLER{};          /**< Histogram for Nr Entries (=Triggrs) for normalization after LER injection */
      TH1F* hEOccAfterInjHER{};          /**< Histogram for Nr Entries (=Triggrs) for normalization after HER injection */

      void initialize() override final; /**< initialize function */

      void beginRun() override final;  /**< beginRun function */

      void event() override final; /**< event function */

      void defineHisto() override final; /**< defineHisto function */

    };//end class declaration


  } //end ECL namespace;
} // end namespace Belle2
