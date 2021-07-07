/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
#include <TH1.h>
#include <TH2.h>
#include <string>

namespace Belle2 {

  /** TTD Injection DQM module.
   *
   * Fill Histograms with low level trigger info,
   * Injection with time (ticks) taken from TTD packet
   */
  class TTDDQMModule : public HistoModule {

  public:

    /** Constructor defining the parameters */
    TTDDQMModule();

  private:
    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

    /** Input object for TTD mdst object */
    StoreObjPtr<EventLevelTriggerTimeInfo> m_EventLevelTriggerTimeInfo;


    TH2I* hTrigAfterInjLER{};  /**< Histogram Veto tuning triggers after LER injection */
    TH2I* hTrigAfterInjHER{};  /**< Histogram Veto tuning triggers after HER injection */

    TH2I* hTrigBunchInjLER{};  /**< Histogram correlation triggered and injected bunch LER */
    TH2I* hTrigBunchInjHER{};  /**< Histogram correlation triggered and injected bunch HER */

    TH1I* hTriggersDeltaT{};   /**< Histogram for Delta T to previous Trigger */
    TH1I* hTriggersPerBunch{}; /**< Histogram for Triggers per Bunch  */

    TH1I* hBunchInjLER{};      /**< Histogram injected HER bunch (triggers) */
    TH1I* hBunchInjHER{};      /**< Histogram injected LER bunch (triggers) */

    void initialize() override final; /**< initialize function */

    void beginRun() override final;  /**< beginRun function */

    void event() override final; /**< event function */

    void defineHisto() override final; /**< defineHisto function */

  };//end class declaration


} // end namespace Belle2
