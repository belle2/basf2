/**************************************************************************
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

namespace Belle2 {

  class ECLDigit;
  class ECLDsp;
  class ECLCrystalCalib;

  /** Calibration collector module that uses delayed Bhabha to compute coveriance matrix */
  class eclAutocovarianceCalibrationC2CollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor.
     */
    eclAutocovarianceCalibrationC2CollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:

    StoreArray<ECLDigit> m_eclDigits; /**< Required input array of ECLDigits */
    StoreArray<ECLDsp> m_eclDsps; /**< Required input array of ECLDSPs */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    DBObjPtr<ECLCrystalCalib> m_ECLAutocovarianceCalibrationC1Threshold;
    std::vector<float> m_PeakToPeakThresholds; /**< vector of thresholds obtained from DB object */

  };
} // end Belle2 namespace
