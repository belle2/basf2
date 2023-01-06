/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisV0.h
// Description : Overlay plotting for V0
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TH1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisV0Module final : public DQMHistAnalysisModule {

    //! List of histograms for display
    typedef std::map<std::string, TH1*> HistList;


    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisV0Module();

  private:

    //! TCanvas objects for x vs y plots
    TCanvas* m_c_xvsy[32] = {nullptr};
    //! TCanvas object for x vs z plot
    TCanvas* m_c_xvsz = nullptr;
    //! Vector of TList objects for contour in XY plane
    std::vector<TList*> contLevelXY;
    //! TList objects for contour in XZ plane
    TList* contLevelXZ = NULL;

    //! Path to overlay file
    std::string m_OverlayPath = "";

    //! Module functions to be called from main process
    void initialize() override final;

    //! Module functions to be called from event process
    void event() override final;

  };
} // end namespace Belle2

