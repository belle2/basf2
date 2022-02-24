/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisOut.h
// Description : Output module to NSM vars for DQM Histogram analysis
//-

#pragma once

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <dqm/analysis/modules/DQMHistAnalysisOutput.h>

namespace Belle2 {

  /**
   * The module to output values to NSM network.
   */
  class DQMHistAnalysisOutputNSMModule : public DQMHistAnalysisOutputModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisOutputNSMModule();

    /**
     * Destructor.
     */
    virtual ~DQMHistAnalysisOutputNSMModule();

    /**
     * Initializer.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * This method is called for each event.
     */
    virtual void event() override;

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate() override;

    // Data members
  private:
    /** The NSM node name. */
    std::string m_nodename;
    /** The RC node name. */
    std::string m_rcnodename;
    /** The NSM callback object. */
    NSMCallback* m_callback = nullptr;

  };
} // end namespace Belle2

