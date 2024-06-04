/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TrackingAbortDQMModule_H
#define TrackingAbortDQMModule_H

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <svd/calibration/SVDNoiseCalibrations.h>

#include <framework/core/HistoModule.h>
#include <string>
#include <TH2S.h>

namespace Belle2 {

  class EventMetaData;
  class SVDShaperDigit;
  class EventLevelTrackingInfo;
  class TRGSummary;

  /** SVD DQM Module for the Unpacker*/
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

    int const nStripsL3V = 768 * 2 * 7; /**< number of V-side L3 strips*/

    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo; /**< tracking abort info*/
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data*/

    StoreArray<SVDShaperDigit> m_strips; /**< strips*/
    StoreObjPtr<TRGSummary> m_trgSummary; /**< trg summary */

    //vector: 0 = passive veto; 1 = active veto
    TH1F* m_nEventsWithAbort[2]; /**< 0: no abort; 1: at least one abort*/;
    TH1F* m_trackingErrorFlagsReasons[2]; /**< stores the reason of the abort */
    TH1F* m_svdL3vZS5Occupancy[2]; /**<distribution of the SVD L3 V ZS5 occupancy*/
    TH1F* m_nCDCExtraHits[2]; /**< distribution of the number of extra CDC hits */
  };

}

#endif // TrackingAbortDQMModule_H

