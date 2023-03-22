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

//Root
#include <TH2F.h>

namespace Belle2 {

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

    void closeRun() override;

  private:

    StoreArray<ECLDsp> m_eclDsps; /**< Required input array of ECLDSPs */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    DBObjPtr<ECLCrystalCalib> m_ECLAutocovarianceCalibrationC1Threshold; /**< peak to peak noise threshold computed from step C1 */
    std::vector<float> m_PeakToPeakThresholds; /**< vector of thresholds obtained from DB object */

    float myHist[8736][32];  /**< container for histogram values  */

    TH2F* m_BaselineInfoVsCrysID; /**< final histogram returned by collector  */

  };
} // end Belle2 namespace
