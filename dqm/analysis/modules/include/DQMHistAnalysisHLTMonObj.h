/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

namespace Belle2 {

  /**
   * Creates monitoring object for HLT
   */
  class DQMHistAnalysisHLTMonObjModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisHLTMonObjModule();

    /**
     * Destructor
     */
    virtual ~DQMHistAnalysisHLTMonObjModule();

    /**
     * Initialize the Module.
     */
    virtual void initialize() override;

    /**
     * Begin run function
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     */
    virtual void terminate() override;

  protected:

    TCanvas* m_c_filter = nullptr; /**<Canvas with histograms related to HLT filter*/
    TCanvas* m_c_skim = nullptr; /**<Canvas with histograms related to HLT skims*/
    TCanvas* m_c_hardware = nullptr; /**<Canvas with histograms related to HLT hardware*/
    TCanvas* m_c_l1 = nullptr; /**<Canvas with histograms related to L1*/

    MonitoringObject* m_monObj = nullptr; /**< MonitoringObject to be produced by this module*/

  };

} // Belle2 namespace
