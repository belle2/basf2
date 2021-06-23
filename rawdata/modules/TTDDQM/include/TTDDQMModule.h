/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
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


    TH2I* hTrigAfterInjLER{};         /**< Histogram Veto tuning triggers after LER injection */
    TH2I* hTrigAfterInjHER{};         /**< Histogram Veto tuning triggers after HER injection */

    TH2I* hTrigBunchInjLER{};         /**< Histogram Veto tuning triggers after LER injection */
    TH2I* hTrigBunchInjHER{};         /**< Histogram Veto tuning triggers after HER injection */

    TH1I* hTriggersAfterTrigger{};          /**< Histogram for Nr Entries (=Triggers after Last Trigger */
    TH1I* hTriggersPerBunch{};          /**< Histogram for Trigger per Bunch  */

    TH1I* hBunchInjLER{}; /**< Histogram injected HER bunch (triggers) */
    TH1I* hBunchInjHER{}; /**< Histogram injected LER bunch (triggers) */

    void initialize() override final; /**< initialize function */

    void beginRun() override final;  /**< beginRun function */

    void event() override final; /**< event function */

    void defineHisto() override final; /**< defineHisto function */

  };//end class declaration


} // end namespace Belle2
