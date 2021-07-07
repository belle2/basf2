/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <pxd/dataobjects/PXDDAQStatus.h>

#include <TH2F.h>
#include <string>

namespace Belle2 {

  namespace PXD {
    /** The PXD Gatint after Injection DQM module.
     *
     * PXD  after GatedDHC Histogramming with
     * time (ticks) taken from TTD packet
     *
     */
    class PXDGatedDHCDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDGatedDHCDQMModule();

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;

      /** Input array for DAQ Status. */
      StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;

      TH2F* hGateAfterInjLER{};          /**< Histogram  after LER injection */
      TH2F* hGateAfterInjHER{};          /**< Histogram  after HER injection */

      void initialize() override final; /**< initialize function */

      void beginRun() override final;  /**< beginRun function */

      void event() override final; /**< event function */

      void defineHisto() override final; /**< defineHisto function */

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
