/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

#include <mdst/dataobjects/EventLevelTrackingInfo.h>

namespace Belle2 {

  /** Tracking HLT DQM. */
  class TrackingHLTDQMModule : public DQMHistoModuleBase {

  public:
    /** Constructor */
    TrackingHLTDQMModule();
    /* Destructor */
    ~TrackingHLTDQMModule() { }

    /** Module functions */
    virtual void initialize() override;
    virtual void event() override;

    /** Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed to be placed in this function.
     * Also at the end function all m_histogramParameterChanges should be processed via the ProcessHistogramParameterChange function. */
    virtual void defineHisto() override;

  protected:
    /** All the following Define- functions should be used in the defineHisto() function to define histograms. The convention is that every Define- function is responsible for creating its
     * own TDirectory (if it's needed). In any case the function must then return to the original gDirectory.
     * For the creation of histograms the THFFactory or the Create- functions should be used. */
    virtual void DefineFlags();

    /** Acccess to the EventLevelTrackingInfo object in the datastore. */
    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;

    /** Monitors the Error flags set by the tracking code. As of the time of implementation there only were two flags:
     * VXDTF2AbortionFlag, i.e. how often the VXDTF2 did abort the event and did not produce tracks,
     * and UnspecifiedTrackFindingFailure.
     * The histogram records if any flag was set. */
    TH1F* m_trackingErrorFlags = nullptr;
  };
}
