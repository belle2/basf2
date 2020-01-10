/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <TH1.h>
#include <TH2.h>
#include <string>
#include <map>

namespace Belle2 {

  namespace SVD {
    /** The SVD Occupancy after Injection DQM module.
     *
     * Fill Histograms SVD with Occupancy (and nr of triggers) after
     * Injection with time (ticks) taken from TTD packet
     */
    class SVDDQMInjectionModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      SVDDQMInjectionModule();

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
      std::string m_SVDShaperDigitsName;  /**< The name of the StoreArray of SVDShaperDigit to be generated */

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;

      /** Input array for SVD Raw Hits. */
      StoreArray<SVDShaperDigit> m_digits;

      /** the VXD geometry */
      VXD::GeoCache& m_vxdGeometry;

      TH1F* m_hOccAfterInjLER = nullptr;          /**< Histogram Occupancy after LER injection */
      TH1F* m_hOccAfterInjHER = nullptr;          /**< Histogram Occupancy after HER injection */

      TH1F* m_hTrgOccAfterInjLER = nullptr;          /**< Histogram for Nr Entries (=Triggrs) for normalization after LER injection */
      TH1F* m_hTrgOccAfterInjHER = nullptr;          /**< Histogram for Nr Entries (=Triggrs) for normalization after HER injection */

      TH1F* m_hMaxOccAfterInjLER = nullptr;          /**< Histogram Max Occupancy after LER injection */
      TH1F* m_hMaxOccAfterInjHER = nullptr;          /**< Histogram Max Occupancy after HER injection */

      void initialize() override final; /**< initialize function */

      void beginRun() override final;  /**< beginRun function */

      void event() override final; /**< event function */

      void defineHisto() override final; /**< defineHisto function */

    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2
