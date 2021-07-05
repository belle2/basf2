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

#include <svd/dataobjects/SVDHistograms.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <framework/dataobjects/EventT0.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

namespace Belle2 {
  /**
   * Collector module used to create the histograms needed for the
   * SVD CoG-Time calibration
   */
  class SVDTimeValidationCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    SVDTimeValidationCollectorModule();

    /**
     * Initialize the module
     */
    void prepare() override final;

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
    std::string m_svdClusters = "SVDClusters"; /**< Name of the SVDClusters store array used as parameter of the module*/
    StoreArray<SVDCluster> m_svdCls; /**< SVDClusters store array*/

    /**SVDClusterOnTracks */
    std::string m_svdClustersOnTracks =
      "SVDClustersOnTracks"; /**< Name of the SVDClusters store array used as parameter of the module*/
    StoreArray<SVDCluster> m_svdClsOnTrk; /**< SVDClusters store array*/

    /**EventT0 */
    std::string m_eventTime = "EventT0"; /**< Name of the EventT0 store object pointer used as parameter of the module*/
    StoreObjPtr<EventT0> m_eventT0; /**< EventT0 store object pointer*/

    /**RecoTracks */
    std::string m_recotrack = "RecoTracks"; /**< Name of the RecoTracks store object pointer used as parameter of the module*/
    StoreArray<RecoTrack> m_recoTrk; /**< RecoTracks store object pointer*/

    /**Tracks */
    std::string m_track = "Tracks"; /**< Name of the Tracks store object pointer used as parameter of the module*/
    StoreArray<Track> m_trk; /**< Tracks store object pointer*/

  };

} // end namespace Belle2
