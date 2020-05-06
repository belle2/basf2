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
#include <TCanvas.h>

//ARICH
#include <arich/utility/ARICHChannelHist.h>

namespace Belle2 {

  /**
   * Example module of how to use MonitoringObject in DQMHistAnalysis module
   */
  class DQMHistAnalysisARICHMonObjModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisARICHMonObjModule();

    /**
     * Destructor
     */
    virtual ~DQMHistAnalysisARICHMonObjModule();

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
    TCanvas* m_c_mirror = NULL; /**<Canvas with histograms related to mirrors*/
    TCanvas* m_c_tracks = NULL; /**<Canvas with histograms related to tracks*/
    TH2Poly* pp1 = NULL;
    TH2Poly* pp2 = NULL;
    TH2Poly* pflash = NULL;

    Belle2::ARICHChannelHist* m_apdHist = NULL; /**<ARICH TObject to draw hit map for each APD*/
    Belle2::ARICHChannelHist* m_chHist = NULL; /**<ARICH TObject to draw flash map for each channel*/
    Belle2::ARICHChannelHist* m_hapdHist = NULL; /**<ARICH TObject to draw flash map for each hapd*/
    MonitoringObject* m_monObj = NULL; /**< MonitoringObject to be produced by this module*/

  };

} // Belle2 namespace
