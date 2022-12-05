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

//ARICH
#include <arich/utility/ARICHChannelHist.h>

#include <vector>

#include <TCanvas.h>
#include <TLine.h>
#include <TH2Poly.h>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisARICHModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisARICHModule();

    /**
     * Destructor
     */
    ~DQMHistAnalysisARICHModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override;

    /**
     * Event processor.
     */
    void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate() override;

  protected:
    bool m_debug;/**<debug*/
    bool m_enableAlert;/**<Enable alert by base color of canvases*/

    int alertColor[3] = {kWhite, kYellow, kRed};/**<Alert color of canvases*/

    int m_minStats = 10000;/**<The lower limit on the number of events to enable the alert*/

    //TObjects for DQM analysis
    TLine* m_LineForMB[5] = {}; /**<Lines to divide the sectors on mergerHit histogram*/
    TCanvas* m_c_bits = NULL; /**<Canvas for modified bits histogram*/
    TCanvas* m_c_mergerHit = NULL; /**<Canvas for modified mergerHit histogram*/
    TCanvas* m_c_hitsPerEvent = NULL; /**<Canvas for modified hitsPerTrack histogram*/
    TCanvas* m_c_theta = NULL; /**<Canvas for modified theta histogram*/

    Belle2::ARICHChannelHist* m_apdHist = NULL; /**<ARICH TObject to draw hit map for each APD*/
    TH2Poly* m_apdPoly = NULL;  /**< hit map for each APD */
    TCanvas* m_c_apdHist = NULL; /**<Canvas for 2D hit map of APDs*/

    std::vector<int> maskedMergers{53}; /**<The id numbers of masked merger boards to avoid unnecessary alert*/
  };

} // Belle2 namespace
