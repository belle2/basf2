/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* DQM headers. */
#include <dqm/analysis/modules/DQMHistAnalysis.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>


/* ROOT headers. */
#include <TCanvas.h>
#include <TH1.h>
#include <TString.h>
#include <TText.h>
#include <TLine.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * Analysis of KLM DQM histograms.
   */
  class DQMHistAnalysisKLM2Module : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisKLM2Module();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisKLM2Module();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:


    /**
     * Process histogram containing the number of hits in plane.
     * @param[in]  histName  Histogram name.
     * @param[in]  histogram Histogram itself.
     */
    void processPlaneHistogram(const std::string& histName, TH1* histogram);


    /** TLine for boundary in plane histograms. */
    TLine m_PlaneLine;

    /** TText for names in plane histograms. */
    TText m_PlaneText;

    /** Histogram from DQMInfo with run type. */
    TH1* m_RunType = NULL;

    /** String with run type. */
    TString m_RunTypeString;

    /** Run type flag for null runs. */
    bool m_IsPhysicsRun = false;

    /** Histogram for BKLM plane efficiency. */
    TH1* m_eff_bklm = NULL;

    /** BKLM plane efficiency canvas */
    TCanvas* m_c_eff_bklm = NULL;

    /** Histogram for EKLM plane efficiency. */
    TH1* m_eff_eklm = NULL;

    /** EKLM plane efficiency canvas */
    TCanvas* m_c_eff_eklm = NULL;

    /** Histogram for BKLM sector efficiency. */
    TH1* m_eff_bklm_sector = NULL;

    /** Histogram for BKLM sector efficiency. */
    TCanvas* m_c_eff_bklm_sector = NULL;

    /** Histogram for EKLM sector efficiency. */
    TH1* m_eff_eklm_sector = NULL;

    /** Histogram for EKLM sector efficiency. */
    TCanvas* m_c_eff_eklm_sector = NULL;


    /** EKLM element numbers. */
    const EKLMElementNumbers* m_EklmElementNumbers;


    /*******************************************/
    //OTHER USEFUL VARIABLES
    /*******************************************/

    /** Number of channel hit histograms per sector for BKLM. */
    const int m_ChannelHitHistogramsBKLM = 2;

    /** Number of channel hit histograms per sector for EKLM. */
    const int m_ChannelHitHistogramsEKLM = 3;

  };

}
