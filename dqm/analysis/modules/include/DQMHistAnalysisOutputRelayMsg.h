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

namespace Belle2 {
  /**
   * Module to relay Canvas+Histogram to our old jsroot webserver. deprecated.
   */

  class DQMHistAnalysisOutputRelayMsgModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisOutputRelayMsgModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

    // Data members
  private:
    /** The socket to the canvas server. */
    TSocket* m_sock = nullptr;
    /** The port of the canvas server. */
    int m_port;
    /** The hostname of the canvas server. */
    std::string m_hostname;
    /** Send untagged canvas by default */
    bool m_canvasSendDefault{true};
  };
} // end namespace Belle2

