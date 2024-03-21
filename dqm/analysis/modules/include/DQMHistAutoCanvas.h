/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAutoCanvas.h
// Description : This module auto-plots each histogram in its canvas
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

namespace Belle2 {
  /** Class definition for the reference histogram display. */

  class DQMHistAutoCanvasModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAutoCanvasModule();

    /**
     * This method is called for each run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    // Data members
  private:
    /** The list of folders for which automatically generate canvases. */
    std::vector<std::string> m_inclfolders;
    /** The list of folders which are excluded from automatically generate canvases. */
    std::vector<std::string> m_exclfolders;
    /** The filename of a list canvas names to auto generate. */
    std::vector<std::string> m_listfile;

    /** list of wanted canvas name if included as list file*/
    std::set<std::string> m_canvaslist;

    /** The map of histogram names to canvas pointers for buffering. */
    std::map<std::string, TCanvas*> m_cs;
  };
} // end namespace Belle2

