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
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {
  /**
   * Collector module used to create the histograms needed for the
   * SVD CoG-Time calibration
   */
  class SVDClusterTimeShifterCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    SVDClusterTimeShifterCollectorModule();

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

    /** set prefix of the list of clusters on track */
    void setSVDClusterOnTracksPrefix(const TString& str) {m_svdClsOnTrksPrefix = str;}
    /** set list of time algorithm */
    void setTimeAlgorithm(const std::vector<TString>& lst) {m_timeAlgorithms = lst;}

  private:

    int m_maxClusterSize = 6; /**< Maximum size of SVD clusters */
    std::vector<TString> m_timeAlgorithms = {"CoG3", "ELS3", "CoG6"}; /**< List of time algorithms to calibrate */
    TString m_svdClsOnTrksPrefix = "SVDClustersOnTracks"; /**< SVDClusterOnTracks */
    std::map<TString, StoreArray<SVDCluster>> m_svdClsOnTrk; /**< SVDClusters store array*/

    StoreObjPtr<EventMetaData> m_emdata; /**< EventMetaData store object pointer*/
    std::string m_eventTime = "EventT0"; /**< Name of the EventT0 store object pointer used as parameter of the module*/
    StoreObjPtr<EventT0> m_eventT0; /**< EventT0 store object pointer*/
  };

} // end namespace Belle2
