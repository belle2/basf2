/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Description: Create plots NOW-TriggerTime for performance monitoring   *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include "TH1D.h"

namespace Belle2 {

  /** Processing Delay DQM Module */
  class DelayDQMModule : public HistoModule {

  public:

    /** Constructor */
    DelayDQMModule();

  private:

    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;

    void defineHisto() override final;

  private:
    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
    std::string m_title; /**< Prefix for title (NOT histo name) */

    /** Input ptr for EventMetaData. */
    StoreObjPtr<EventMetaData> m_eventMetaData;

    TH1D* m_DelayS = nullptr;        /**< Delay between trigger and end of processing in s */
    TH1D* m_DelayMs = nullptr;        /**< Delay between trigger and end of processing in ms*/
    TH1D* m_DelayLog = nullptr;        /**< Delay between trigger and end of processing log scale */

    void BinLogX(TH1* h); /**< helper function to replace X axis by a log scaled axis */
  };

}
