/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisOutputRelayMsg.h
// Description : Output module for DQM Histogram analysis
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <TSocket.h>
#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputRelayMsgModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisOutputRelayMsgModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisOutputRelayMsgModule();

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

    // Data members
  private:
    /** The socket to the canvas server. */
    TSocket* m_sock = nullptr;
    /** The port of the canvas server. */
    int m_port;
    /** The hostname of the canvas server. */
    std::string m_hostname;
    /** Send untagged canvase by default */
    bool m_canvasSendDefault{true};
  };
} // end namespace Belle2

