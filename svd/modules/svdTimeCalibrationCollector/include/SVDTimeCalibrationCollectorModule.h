/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <string>

#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <framework/dataobjects/EventT0.h>

#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>


namespace Belle2 {
  /**
   * Collector module used to create the histograms needed for the
   * SVD CoG-Time calibration
   */
  class SVDTimeCalibrationCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    SVDTimeCalibrationCollectorModule();

    /**
     * Initialize the module
     */
    void prepare() override final;

    /**
     * Called when entering a new run
     */
    void startRun() override final;

    /**
     * Event processor
     */
    void collect() override final;

  private:

    /**EventMetaData */
    StoreObjPtr<EventMetaData> m_emdata; /**< EventMetaData store object pointer*/

    /**SVDEventInfo */
    std::string m_svdEventInfo = "SVDEventInfo"; /**< Name of the SVDEventInfo store array used as parameter of the module*/
    StoreObjPtr<SVDEventInfo> m_svdEI; /**< SVDEventInfo store object pointer*/

    DBObjPtr<HardwareClockSettings> m_hwClock;  /**< systems clock*/

    /**SVDCluster */
    std::string m_svdClustersOnTracks =
      "SVDClustersFromTracks"; /**< Name of the SVDClusters store array used as parameter of the module*/
    StoreArray<SVDCluster> m_svdClsOnTrk; /**< SVDClusters store array*/

    /**EventT0 */
    std::string m_eventTime = "EventT0"; /**< Name of the EventT0 store object pointer used as parameter of the module*/
    StoreObjPtr<EventT0> m_eventT0; /**< EventT0 store object pointer*/

    TH1F* m_hEventT0FromCDC = nullptr; /**< Distribution of EventT0 reconstructed by the CDC for all sensos/side*/
    TH1F* m_hEventT0FromCDCSync = nullptr; /**< Distribution of EventT0 reconstructed by the CDC and synchronized for all sensos/side*/
    TH1F* m_hRawTimeL3V = nullptr; /**< Raw time distribution of layer3 V-side for IoV determination */
    TH1F* m_hRawTimeL3VFullRange = nullptr; /**< Raw time distribution of layer3 V-side */

    double m_rawCoGBinWidth = 2.; /**< Raw_CoG Bin Width [ns] for 2D-histogram */
    double m_minRawTimeForIoV = 0.; /**< Minimum value of the raw time distribution used to determine whether change IoV or not */
    double m_maxRawTimeForIoV = 150.; /**< Maxmum value of the raw time distribution used to determine whether change IoV or not */
  };

} // end namespace Belle2
