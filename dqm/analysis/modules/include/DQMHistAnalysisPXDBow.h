/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDBow.h
// Description : An example module for DQM histogram analysis
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>

#include <TH1F.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisPXDBowModule final : public DQMHistAnalysisModule {

    /// Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPXDBowModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisPXDBowModule();

    /**
     * Initializer.
     * This method is called at the beginning of data processing.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

    //! Parameters accessible from basf2 scripts
  private:
    //! name of histogram directory
    std::string m_histogramDirectoryName;

    //! Data members
  private:
    /** The final drawing canvas. */
    TCanvas* m_cResV = nullptr;

    /** The final histogram*/
    TH1F m_hResV;

    //! vector for the IDs of all forward PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    Float_t m_roiThreshold{0.1};
    Float_t m_statThreshold{300};
    Float_t m_sagittaThreshold{0.06};
    std::string m_moduleName{"2.2.1"};
  };
} // end namespace Belle2

