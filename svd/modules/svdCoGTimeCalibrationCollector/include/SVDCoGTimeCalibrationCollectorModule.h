/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                      *
 * Copyright(C) 2017 - Belle II Collaboration                              *
 *                                                                         *
 * Author: The Belle II Collaboration                                      *
 * Contributors: Gaetano de Marino, Tadeas Bilka                           *
 *                                                                         *
 *                                                                         *
 * This software is provided "as is" without any warranty.                 *
 ***************************************************************************/
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
#include <framework/dataobjects/EventMetaData.h>

#include <analysis/utility/ReferenceFrame.h>
#include <iostream>
#include <framework/logging/Logger.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {
  /**
   * This collects the position
   * and the dimension of the beamspot using mu+mu- events for
   * calibration of the SVDCoGTimeCalibration using CAF
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

    /**
     * End-of-run action.
     */
    void closeRun() override final;

    /**
     * Termination action.
     */
    void finish() override;

  private:

    /** SVDCluster */
    std::string m_svdClusters;
    StoreArray<SVDCluster> m_svdCls;

    /** SVDRecoDigits */
    std::string m_svdRecoDigits;
    StoreArray<SVDRecoDigit> m_svdRD;

    /** EventT0 */
    std::string m_eventTime;
    StoreObjPtr<EventT0> m_eventT0;

    /** SVDHistograms */
    SVDHistograms<TH2F>* m_hEventT0vsCoG = NULL;

    /** Tree */
    std::string m_tree;
    TTree* m_histogramTree = NULL;
    TH2F* m_hist = NULL;
    int m_layer = 0;
    int m_ladder = 0;
    int m_sensor = 0;
    int m_side = 0;

    TH1F* m_hEventT0 = NULL;
  };

} // end namespace Belle2

