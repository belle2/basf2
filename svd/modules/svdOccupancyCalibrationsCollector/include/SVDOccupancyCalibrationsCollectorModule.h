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

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <string>
#include "TH1F.h"

#include <svd/dataobjects/SVDShaperDigit.h>

#include <svd/dataobjects/SVDHistograms.h>

#include "TTree.h"

#include <framework/dataobjects/EventMetaData.h>


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
    std::string m_tree = "tree";
    TTree* m_histogramTree = nullptr; /**<tree containing as events the histograms per layer, ladder, sensor, side*/

    TH1F* m_hnevents = nullptr; /**<first bin of the histogram is counting the processed events*/
    TH1F* m_hist = nullptr; /**<pointer to occupancy histogram*/
    int m_layer = 0;  /**<SVD layer identifier*/
    int m_ladder = 0;/**<SVD ladder identifier*/
    int m_sensor = 0;/**<SVD sensor identifier*/
    int m_side = 0;/**<SVD side identifier*/

  private:


    static const int m_nSides = 2; /**< number of sides*/

  };

} // end namespace Belle2

