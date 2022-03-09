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
#include <TH1.h>
#include <string>

namespace Belle2 {

  /** Tracking ExpressReco DQM. */
  class TrackingExpressRecoDQMModule : public DQMHistoModuleBase {

  public:
    /** Constructor */
    TrackingExpressRecoDQMModule();
    /** Destructor */
    ~TrackingExpressRecoDQMModule() { }

    /** Module functions */
    void initialize() override;
    /** fill of the histograms happens here */
    void event() override;

    /** Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed to be placed in this function.
     * Also at the end function all m_histogramParameterChanges should be processed via the ProcessHistogramParameterChange function. */
    void defineHisto() override;

  protected:
    /* All the following Define- functions should be used in the defineHisto() function to define histograms.
     * The convention is that every Define- function is responsible for creating its
     * own TDirectory (if it's needed). In any case the function must then return to the original gDirectory.
     * For the creation of histograms the THFFactory or the Create- functions should be used. */
    /** Defines the histograms for the tracking abort flags. */
    virtual void DefineAbortFlagsHistograms();

    /** Acccess to the EventLevelTrackingInfo object in the datastore. */
    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;

    /** Monitors the Error flags set by the tracking code.
     * The histogram records if any flag was set. */
    TH1F* m_trackingErrorFlags = nullptr;

    /** Monitors the Error flags set by the tracking code.
     * The histogram has a bin for each flag. */
    TH1F* m_trackingErrorFlagsReasons = nullptr;

  private:
    /** if True, the module produces the 1D Track Residual plot for each VXD sensor*/
    bool m_produce1Dres = true;
    /** if True, the module produces the 2D Track Residual plot for each VXD sensor*/
    bool m_produce2Dres = false;
    /** Name of the directory for the histograms. */
    std::string m_histogramDirectoryName = "TrackingERDQM";
    /** Optional suffix for the histogram's title. */
    std::string m_histogramTitleSuffix = "";

  };
}
