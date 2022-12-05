/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/core/DQMHistAnalysis.h>

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
    ~DQMHistAnalysisHLTMonObjModule();

    /**
     * Initialize the Module.
     */
    void initialize() override;


    /**
     * End-of-run action.
     */
    void endRun() override;

    /**
     * Termination action.
     */
    void terminate() override;

  protected:

    TCanvas* m_c_filter = nullptr; /**<Canvas with histograms related to HLT filter*/
    TCanvas* m_c_skim = nullptr; /**<Canvas with histograms related to HLT skims*/
    TCanvas* m_c_hardware = nullptr; /**<Canvas with histograms related to HLT hardware*/
    TCanvas* m_c_l1 = nullptr; /**<Canvas with histograms related to L1*/

    MonitoringObject* m_monObj = nullptr; /**< MonitoringObject to be produced by this module*/

  };

} // Belle2 namespace
