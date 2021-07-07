/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisExample.h
// Description : An example module for DQM histogram analysis
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TF1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisExampleModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisExampleModule();
    virtual ~DQMHistAnalysisExampleModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    //! Parameters accesible from basf2 scripts
  protected:
    /** The name of the histogram. */
    std::string m_histoname;
    /** The definition of the fitting function. */
    std::string m_function;

    //! Data members
  private:
    /** The fitting function. */
    TF1* m_f = nullptr;
    /** The drawing canvas for the fitting result. */
    TCanvas* m_c = nullptr;

  };
} // end namespace Belle2

