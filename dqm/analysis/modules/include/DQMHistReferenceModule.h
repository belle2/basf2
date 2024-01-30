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
      TString histo1;
      TString histo2;
      TString canvas_name;
      TCanvas* canvas{nullptr};
      TH1* ref_clone{nullptr};
    } REFNODE;

    /**
     * Finds "online" histogram in the corresponding canvas.
     * @param node reference histogram node
     */
    TH1* find_histo_in_canvas(REFNODE* node);

    /**
     * Finds canvas with a given name.
     * @param name canvas name
     */
    TCanvas* find_canvas(TString name);

    /** Struct for reference histogram  */
    std::vector<REFNODE*> m_pnode;

    /** Reference Histogram Root file name */
    std::map<std::string, std::string> m_listRefFiles;

    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;

  };
} // end namespace Belle2

