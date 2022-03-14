/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <TH1.h>

namespace Belle2 {

  /** Tracking HLT DQM. */
  class TrackingHLTDQMModule : public DQMHistoModuleBase {

  public:
    /** Constructor */
    TrackingHLTDQMModule();
    /** Destructor */
    ~TrackingHLTDQMModule() { }

    /** Module functions */
    void initialize() override;
    /** fill of the histograms happens here */
    void event() override;

    /** Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed to be placed in this function.
     * Also at the end function all m_histogramParameterChanges should be processed via the ProcessHistogramParameterChange function. */
    virtual void defineHisto() override;

  protected:
    /** All the following Define- functions should be used in the defineHisto() function to define histograms. The convention is that every Define- function is responsible for creating its
     * own TDirectory (if it's needed). In any case the function must then return to the original gDirectory.
     * For the creation of histograms the THFFactory or the Create- functions should be used. */
    virtual void DefineAbortFlagsHistograms();

    /** Input array for DAQ Status. */
    StoreArray<RawFTSW> m_rawTTD;

    /** Acccess to the EventLevelTrackingInfo object in the datastore. */
    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;

    /** Monitors the Error flags set by the tracking code. As of the time of implementation there only were two flags:
     * VXDTF2AbortionFlag, i.e. how often the VXDTF2 did abort the event and did not produce tracks,
     * and UnspecifiedTrackFindingFailure.
     * The histogram records if any flag was set. */
    TH1F* m_trackingErrorFlags = nullptr;

    /** abort rate as a function of time after injection and time within a beam cycle - HER*/
    TH2F* m_abortVStimeHER = nullptr;

    /** number of events as a function of time after injection and time within a beam cycle - HER*/
    TH2F* m_allVStimeHER = nullptr;

    /** abort rate as a function of time after injection and time within a beam cycle - LER*/
    TH2F* m_abortVStimeLER = nullptr;

    /** number of events as a function of time after injection and time within a beam cycle - LER*/
    TH2F* m_allVStimeLER = nullptr;

    /** Beam revolution time in microseconds (approximated).
     *
     * The exact time could be obtained as
     * `5120 / HardwareClockSettings::getAcceleratorRF() * 1e3`
     * but this would run after defineHisto() if used in initialize().
     * Since defineHisto() uses this value, using a run-independent
     * approximated constant value is the only way.
     */
    static constexpr double c_revolutionTime = 5120.0 / 508.0;

    /** Defines the range of the x axis of the 2D time histogram */
    static constexpr double c_noInjectionTime = 30e3;

    /** Approximated global clock frequency in MHz.
     *
     * Used to convert TTD timing to us.
     *
     * The exact frequency could be obtained as
     * `HardwareClockSettings::getGlobalClockFrequency() * 1e3`
     * but this would produce inconsistent histograms since I am
     * forced to use an approximated accelerator RF (508 MHz) for the
     * beam revolution period.
     *
     * @sa c_revolutionTime
     */
    static constexpr double c_globalClock = 127.0;
  };
}
