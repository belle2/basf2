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

#ifndef DelayDQMMODULE_H_
#define DelayDQMMODULE_H_

#include <framework/core/HistoModule.h>

#include "TH1D.h"

namespace Belle2 {

  /**  Delay DQM  Module */
  class DelayDQMModule : public HistoModule {

  public:

    /** Constructor */
    DelayDQMModule();
    /* Destructor */
    virtual ~DelayDQMModule();

    /** Module functions */
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    virtual void defineHisto() override;

  private:
    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
    std::string m_title; /**< Prefix for title (NOT histo name) */

    TH1D* m_DelayS = nullptr;        /**< Delay between trigger and end of processing in s */
    TH1D* m_DelayMs = nullptr;        /**< Delay between trigger and end of processing in ms*/
    TH1D* m_DelayLog = nullptr;        /**< Delay between trigger and end of processing log scale */

    void BinLogX(TH1* h);
  };

}
#endif

