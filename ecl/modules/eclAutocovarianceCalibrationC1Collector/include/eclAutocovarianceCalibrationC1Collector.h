/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//Calibration
#include <calibration/CalibrationCollectorModule.h>

//Framework
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

//Root
#include <TH2F.h>

namespace Belle2 {

  class ECLDsp;

  /** Calibration collector module to estimate noise level of delayed Bhabha waveforms. First step to use delayed Bhabha to compute coveriance matrix */
  class eclAutocovarianceCalibrationC1CollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor.
     */
    eclAutocovarianceCalibrationC1CollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

    void closeRun() override;

  private:

    StoreArray<ECLDsp> m_eclDsps; /**< Required input array of ECLDSPs */

    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    TH2F* PPVsCrysID; /**< histogram of Cell ID vs. Peak to Peak amplitude*/

  };
} // end Belle2 namespace
