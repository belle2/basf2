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
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 25 - Dec - 2015 ; first commit
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

    //! Constructor / Destructor
    DQMHistAnalysisOutputNSMModule();
    virtual ~DQMHistAnalysisOutputNSMModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
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

