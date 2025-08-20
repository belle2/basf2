/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>

#include <TH1F.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! Module to monitor the PXD bowing*/

  class DQMHistAnalysisPXDBowModule final : public DQMHistAnalysisModule {

    /// Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPXDBowModule();


    /**
     * Initializer.
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
     * This method is called if the current run ends.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

    //! Parameters accessible from basf2 scripts
  private:
    //! name of histogram directory
    std::string m_histogramDirectoryName{""};

    //! Data members
    /** The final drawing canvas. */
    TCanvas* m_cResV = nullptr;

    /** The final histogram*/
    TH1F m_hResV;

    //! vector for the IDs of all forward PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    float m_roiThreshold{0.1}; /**< Threshold values for warning flag on the resV, the value is related on the dimesion of the ROI*/
    float m_statThreshold{300}; /**< Threshold values for statistic flag on the plotted histograms*/
    float m_sagittaThreshold{0.06}; /**< Threshold values for error flag on the sagitta*/
    std::string m_moduleName{"2.2.1"}; /**< name of the module which distribution will be plotted on the dqm*/
  };
} // end namespace Belle2

