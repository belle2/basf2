/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisOut.h
// Description : Output module for DQM Histogram analysis
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputModule();
    virtual ~DQMHistAnalysisOutputModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:

  };
} // end namespace Belle2

