/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <TCanvas.h>

namespace Belle2 {

  /**
   * Example module of how to use MonitoringObject in DQMHistAnalysis module
   */
  class DQMHistAnalysisMonObjModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisMonObjModule();

    /**
     * Destructor
     */
    virtual ~DQMHistAnalysisMonObjModule();

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

    TCanvas* m_c_main = NULL; /**<Canvas with main run summary histograms*/
    TCanvas* m_c_mask = NULL; /**<Canvas with histograms related to channel masking*/

    MonitoringObject* m_monObj = NULL; /**< MonitoringObject to be produced by this module*/

  };

} // Belle2 namespace
