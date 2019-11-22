/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                      *
 * Copyright(C) 2017 - Belle II Collaboration                              *
 *                                                                         *
 * Author: The Belle II Collaboration                                      *
 * Contributors: Laura Zani (2019)                                         *
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

#include <svd/dataobjects/SVDShaperDigit.h>

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


namespace Belle2 {
  /**
   * This This module collects hits from shaper digits to compute per sensor
   * SVD occupancy using mu+mu- events for
   * calibration of the SVDOccupancyCalibration payload using CAF
   */
  class SVDOccupancyCalibrationsCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    SVDOccupancyCalibrationsCollectorModule();

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

    /** SVDShaperDigits*/
    std::string m_svdShaperDigitName;
    StoreArray<SVDShaperDigit> m_storeDigits; /**< shaper digits store array*/

    /** EventMetaData */
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data store array*/

    /** SVDHistograms */
    SVDHistograms<TH1F>* hm_occupancy = nullptr; /**< strip occupancy per sensor*/


    /** Tree */
    std::string m_tree;
    TTree* m_histogramTree = NULL;

    TH1F* m_hnevents = NULL;
    TH1F* m_hist = NULL;
    int m_layer = 0;
    int m_ladder = 0;
    int m_sensor = 0;
    int m_side = 0;

  private:


    static const int m_nLayers = 4;// to avoid empty layers we start from 0
    static const int m_nLadders = 16;
    static const int m_nSensors = 5;
    static const int m_nSides = 2;

    const int nBins = 1500;

    unsigned int sensorsOnLayer[4];


    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = nullptr);  /**< thf */

  };

} // end namespace Belle2

