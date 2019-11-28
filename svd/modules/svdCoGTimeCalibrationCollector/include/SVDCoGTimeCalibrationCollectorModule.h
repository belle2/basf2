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

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <string>
#include "TH1F.h"
#include "TPaveStats.h"
#include <TString.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>

#include "TH2F.h"
#include "TTree.h"

#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

#include <analysis/utility/ReferenceFrame.h>
#include <iostream>
#include <framework/logging/Logger.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {
  /**
   * This collects the EventT0 vs CoG used for the
   * calibration of the SVD timing estimator (CoG) with CAF
   */
  class SVDCoGTimeCalibrationCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    SVDCoGTimeCalibrationCollectorModule();

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
    StoreObjPtr<EventMetaData> m_emdata;

    /** SVDCluster */
    std::string m_svdClusters; /*< Name of the SVDClusters store array used as parameter of the module*/
    StoreArray<SVDCluster> m_svdCls; /*< SVDClusters store array*/

    /** SVDRecoDigits */
    std::string m_svdRecoDigits; /*< Name of the SVDRecoDigits store array used as parameter of the module*/
    StoreArray<SVDRecoDigit> m_svdRD; /*< SVDRecoDigits store array*/

    /** EventT0 */
    std::string m_eventTime; /*< Name of the EventT0 store object pointer used as parameter of the module*/
    StoreObjPtr<EventT0> m_eventT0; /*< EventT0 store object pointer*/

    /** SVDHistograms */
    SVDHistograms<TH2F>* m_hEventT0vsCoG = NULL; /*< Scatter plot t0 vs t_raw (CoG)*/
    SVDHistograms<TH1F>* m_hEventT0 = NULL; /*< EventT0 synchronized distribution*/
    SVDHistograms<TH1F>* m_hEventT0nosync = NULL; /*< EventT0 NOT synchroinized distribution*/

  };

} // end namespace Belle2
