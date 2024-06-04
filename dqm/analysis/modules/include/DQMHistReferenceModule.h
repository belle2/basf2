/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TString.h>
#include <vector>

namespace Belle2 {

  /**
   * Class to add DQM reference histograms
   */

  class DQMHistReferenceModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistReferenceModule();

    /**
     * Destructor.
     */
    ~DQMHistReferenceModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

  private:

    /**
     * Struct for refence histogram info
     */
    typedef struct {
      /** online histogram name */
      std::string orghist_name;
      /** reference histogram name */
      std::string refhist_name;
      /** related canvas name name */
      TCanvas* canvas{nullptr};
      /** pointer to the org reference histogram */
      TH1* ref_org{nullptr};
      /** pointer to the cloned/scaled reference histogram */
      TH1* ref_clone{nullptr};
    } REFNODE;

    /** Struct for reference histogram  */
    std::vector<REFNODE> m_pnode;

    /** Reference Histogram Root file name */
    std::string m_referenceFile;

    /** Is first event in run */
    bool m_firstInRun = true;

    /** Reads reference histograms from input root file */
    void loadReferenceHistos();

  };
} // end namespace Belle2

