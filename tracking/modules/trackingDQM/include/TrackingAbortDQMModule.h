/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <svd/calibration/SVDNoiseCalibrations.h>

#include <framework/core/HistoModule.h>
#include <string>
#include <TH2S.h>

namespace Belle2 {

  class EventMetaData;
  class EventLevelTrackingInfo;
  class TRGSummary;
  class SVDCluster;
  class SVDShaperDigit;
  class CDCHit;

  /** Tracking DQM Module to monitor aborts & background conditions before the HLT filter*/
  class TrackingAbortDQMModule : public HistoModule {

  public:

    /** Constructor */
    TrackingAbortDQMModule();

    /* Destructor */
    virtual ~TrackingAbortDQMModule();

    /** Module function initialize */
    void initialize() override final;
    /** Module function beginRun */
    void beginRun() override final;
    /** Module function event */
    void event() override final;

    /**
     * Defines Histograms
    */
    void defineHisto() override final;

  private:

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;


    //calibration objects
    SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibration db object*/

    static constexpr int m_nStripsL3U = 768 * 2 * 7; /**< number of U-side L3 strips*/

    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo; /**< tracking abort info*/
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data*/

    StoreArray<SVDShaperDigit> m_strips; /**< SVD strips*/
    StoreArray<SVDCluster> m_clusters; /**< SVD clusters*/
    StoreArray<CDCHit> m_cdcHits; /**< CDCHits*/
    StoreObjPtr<TRGSummary> m_trgSummary; /**< trg summary */

    //index: 0 = passive veto; 1 = active veto
    TH1F* m_nEventsWithAbort[2]; /**< 0: no abort; 1: at least one abort*/;
    TH1F* m_trackingErrorFlagsReasons[2]; /**< stores the reason of the abort */
    TH1F* m_svdL3uZS5Occupancy[2]; /**<distribution of the SVD L3 V ZS5 occupancy*/
    TH1F* m_nCDCExtraHits[2]; /**< distribution of the number of extra CDC hits */
    TH1F* m_svdTime[2]; /**< L3 V-side time for all clusters*/
    TH1D* m_integratedAverages[2]; /**< integrated averages of additional SVD, CDC variables */

    /** function to update the bin content */
    void updateBinContent(int index, int bin, float valueToBeAdded);

  };

}


