/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/core/DQMHistAnalysis.h>
#include <TCanvas.h>

namespace Belle2 {

  /**
   * DQM core framework module for enabling EPICS usage in analysis modules
   * This module must run before the first analysis module using EPICS.
   */
  class DQMHistAnalysisEpicsEnableModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisEpicsEnableModule();

  private:
    /**
     * Destructor
     */
    ~DQMHistAnalysisEpicsEnableModule() override;

    /**
     * Initialize the Module.
     */
    void initialize() override final;

    /**
     * Read Only local flag for EPICS
     */
    bool m_useEpicsRO;

    /**
     * local PVPrefix for setting as global
     */
    std::string m_locPVPrefix;
  };

} // Belle2 namespace
