/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TrackingPreFilterDQMModule_H
#define TrackingPreFilterDQMModule_H

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <framework/dataobjects/EventMetaData.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <cdc/dataobjects/CDCHit.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <framework/core/HistoModule.h>
#include <string>
#include <TH2S.h>

namespace Belle2 {

  /** SVD DQM Module for the Unpacker*/
  class TrackingPreFilterDQMModule : public HistoModule {

  public:

    /** Constructor */
    TrackingPreFilterDQMModule();

    /* Destructor */
    virtual ~TrackingPreFilterDQMModule();

    /** Module function initialize */
    void initialize() override final;
    /** Module function beginRun */
    void beginRun() override final;
    /** Module function event */
    void event() override final;
    /** Module function endRun */
    void endRun() override final;

    /**
     * Defines Histograms
    */
    void defineHisto() override final;

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

  private:

//calibration objects
    SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibration db object*/

    int const nStripsL3V = 768 * 2 * 7; /**< number of V-side L3 strips*/

    int m_runNumber; /**< run number*/
    int m_expNumber; /**< experiment number*/

    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo; /**< tracking abort info*/
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data*/

    StoreArray<SVDShaperDigit> m_strips; /**< strips*/
    StoreArray<CDCHit> m_cdcHits; /**< CDC hits*/
    StoreObjPtr<TRGSummary> m_trgSummary; /**< trg summary */

    //vector: 0 = passive veto; 1 = active veto
    TH1F* m_nEventsWithAbort[2]; /**< 0: no abort; 1: at least one abort*/;
    TH1F* m_trackingErrorFlagsReasons[2]; /**< stores the reason of the abort */
    TH1F* m_svdL3vZS5Occupancy[2]; /**<distribution of the SVD L3 V ZS5 occupancy*/
    TH1F* m_nCDCExtraHits[2]; /**< distribution of the number of extra CDC hits */
  };

}

#endif // TrackingPreFilterDQMModule_H

