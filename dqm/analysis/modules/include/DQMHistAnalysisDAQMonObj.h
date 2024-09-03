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

#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>

#include <TH1F.h>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisDAQMonObjModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisDAQMonObjModule();

    /**
     * Destructor
     */
    ~DQMHistAnalysisDAQMonObjModule();

    /**
     * Initialize the Module.
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
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override final;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate() override final;

  protected:
    MonitoringObject* m_monObj = nullptr; /**< monitoring object */

    TH1F* m_PXDDataSize = nullptr; /**< Histogram of PXD event size  */
    TH1F* m_SVDDataSize = nullptr; /**< Histogram of SVD event size  */
    TH1F* m_CDCDataSize = nullptr; /**< Histogram of CDC event size  */
    TH1F* m_TOPDataSize = nullptr; /**< Histogram of TOP event size  */
    TH1F* m_ARICHDataSize = nullptr; /**< Histogram of ARICH event size  */
    TH1F* m_ECLDataSize = nullptr; /**< Histogram of ECL event size  */
    TH1F* m_KLMDataSize = nullptr; /**< Histogram of KLM event size  */
    TH1F* m_TRGDataSize = nullptr; /**< Histogram of TRG event size  */
    TH1F* m_HLTDataSize = nullptr; /**< Histogram of HLT event size  */
    TH1F* m_TotalDataSize = nullptr; /**< Histogram of Total event size  */
  };

} // Belle2 namespace


