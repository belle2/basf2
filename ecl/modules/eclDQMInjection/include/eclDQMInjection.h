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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawFTSWFormat_latest.h>
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

      void initialize() override final;

      void beginRun() override final;

      void event() override final;

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
      std::string m_ECLDigitsName;  /**< The name of the StoreArray of ECLRawHits to be generated */

      bool m_eachModule{false};// create a histo per module

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;

      /** Input array for ECL Raw Hits. */
      StoreArray<ECLDigit> m_storeHits;

      TH1F* hOccAfterInjLER{};          /** Occupancy after LER injection */
      TH1F* hOccAfterInjHER{};          /** Occupancy after HER injection */

      TH1F* hEOccAfterInjLER{};          /** Occupancy after LER injection */
      TH1F* hEOccAfterInjHER{};          /** Occupancy after HER injection */

      void defineHisto() override final;

    };//end class declaration


  } //end ECL namespace;
} // end namespace Belle2
