/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                      *
 * Copyright(C) 2017 - Belle II Collaboration                              *
 *                                                                         *
 * Author: The Belle II Collaboration                                      *
 * Contributors: Luigi Corona, Giulia Casarosa                             *
 *                                                                         *
 *                                                                         *
 * This software is provided "as is" without any warranty.                 *
 ***************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <string>
#include "TH1F.h"

#include <svd/dataobjects/SVDHistograms.h>

#include "TH2F.h"

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <framework/dataobjects/EventT0.h>

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


    /**SVDCluster */
    std::string m_svdClusters = "SVDClustersFromTracks"; /**< Name of the SVDClusters store array used as parameter of the module*/
    StoreArray<SVDCluster> m_svdCls; /**< SVDClusters store array*/

    /**EventT0 */
    std::string m_eventTime = "EventT0"; /**< Name of the EventT0 store object pointer used as parameter of the module*/
    StoreObjPtr<EventT0> m_eventT0; /**< EventT0 store object pointer*/

    /**SVDHistograms */
    SVDHistograms<TH2F>* m_hEventT0vsCoG = nullptr; /**< Scatter plot t0 vs t_raw (CoG)*/
    SVDHistograms<TH1F>* m_hEventT0 = nullptr; /**< EventT0 synchronized distribution*/
    SVDHistograms<TH1F>* m_hEventT0nosync = nullptr; /**< EventT0 NOT synchroinized distribution*/

    TH1F* m_hEventT0FromCDC = nullptr; /**< Distribution of EventT0 reconstructed by the CDC for all sensos/side*/
    TH1F* m_hEventT0FromCDCSync = nullptr; /**< Distribution of EventT0 reconstructed by the CDC and synchronized for all sensos/side*/
    TH1F* m_hRawTimeL3V = nullptr; /**< Raw_CoG distribution of layer3 V-side */

    double m_rawCoGBinWidth = 2; /**< Raw_CoG Bin Width [ns] for 2D-histogram */
  };

} // end namespace Belle2
