/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDOccupancy.h
// Description : module for DQM histogram analysis of SVD sensors occupancies
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysisSVD.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDSummaryPlots.h>

#include <TFile.h>
#include <TText.h>
#include <TPaveText.h>
#include <TCanvas.h>
#include <TH2F.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisSVDOccupancyModule final : public DQMHistAnalysisSVDModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisSVDOccupancyModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisSVDOccupancyModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

  private:

    // parameters
    bool m_printCanvas; /**< if true print the pdf of the canvases */
    bool m_3Samples; /**< if true enable 3 samples histograms analysis */
    double m_occError; /**<error level of the occupancy */
    double m_occWarning; /**< warning level of the occupancy */
    double m_occEmpty; /**<empty level of the occupancy */

    double m_onlineOccError; /**<error level of the onlineOccupancy */
    double m_onlineOccWarning; /**< warning level of the onlineOccupancy */
    double m_onlineOccEmpty; /**<empty level of the occupancy */

    //! Parameters accesible from basf2 scripts
    //  protected:
    SVDSummaryPlots* m_hOccupancy = nullptr;  /**< occupancy histos */
    SVDSummaryPlots* m_hOnlineOccupancy = nullptr;  /**< online occupancy histos */
    SVDSummaryPlots* m_hOccupancyGroupId0 = nullptr;  /**< occupancy histos for cluster time group id=0*/
    SVDSummaryPlots* m_hOccupancy3Samples = nullptr;  /**<  occupancy histos for 3 samples*/
    SVDSummaryPlots* m_hOnlineOccupancy3Samples = nullptr;  /**< online occupancy histos for 3 sampels*/

    TCanvas* m_cOccupancyU = nullptr; /**< occupancy U histo canvas */
    TCanvas* m_cOccupancyU3Samples = nullptr; /**< occupancy U histo canvas for 3 sampes */
    TCanvas* m_cOccupancyUGroupId0 = nullptr; /**< occupancy U histo canvas  for cluster time group Id = 0*/

    TCanvas* m_cOccupancyV = nullptr; /**< occupancy V histo canvas */
    TCanvas* m_cOccupancyV3Samples = nullptr; /**< occupancy V histo canvas  for 3 samples*/
    TCanvas* m_cOccupancyVGroupId0 = nullptr; /**< occupancy V histo canvas  for cluster time group Id = 0*/

    /** additional plots flag*/
    bool m_additionalPlots = false;

    TH1F m_hOccupancyChartChip; /**< occupancy chart histo */
    TCanvas* m_cOccupancyChartChip = nullptr; /**< occupancy chart histo canvas */

    TCanvas* m_cOnlineOccupancyU = nullptr; /**< online occupancy U histo canvas */
    TCanvas* m_cOnlineOccupancyU3Samples = nullptr; /**< online occupancy U histo canvas for 3 samples*/
    TCanvas* m_cOnlineOccupancyV = nullptr; /**< online Occupancy V histo canvas */
    TCanvas* m_cOnlineOccupancyV3Samples = nullptr; /**< online Occupancy V histo canvas  for 3 samples*/

    int m_sensors = 0; /**< number of sensors to considired*/
    TH1F m_hStripOccupancyU[172]; /**< u-side strip chart occupancy histos*/
    TCanvas** m_cStripOccupancyU = nullptr; /**< u-side strip chart occupancy canvas*/
    TH1F m_hStripOccupancyV[172]; /**< u-side strip chart occupancy histos*/
    TCanvas** m_cStripOccupancyV = nullptr; /**< u-side strip chart occupancy canvas*/

    Int_t findBinY(Int_t layer, Int_t sensor); /**< find Y bin corresponding to sensor, occupancy plot*/
    void setOccStatus(float occ, int& occStatus, bool online = false); /**< get occupancy status */

    Int_t m_occUstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/
    Int_t m_occVstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/
    Int_t m_occU3Samples = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 samples*/
    Int_t m_occV3Samples = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 sampels*/

    Int_t m_occUGroupId0 = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 samples*/
    Int_t m_occVGroupId0 = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 sampels*/

    Int_t m_onlineOccUstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/
    Int_t m_onlineOccVstatus = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error*/

    Int_t m_onlineOccU3Samples = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 sample*/
    Int_t m_onlineOccV3Samples = 0; /**< 0 = normal, 1 = empty, 2 = warning, 3 = error for 3 sampes*/

    std::string m_pvPrefix; /**< string prefix for EPICS PVs */

    //! IDs of all SVD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

    /** occupancy status flags */
    enum occStatus {
      good = 0,    /**< green frame */
      warning = 1, /**< orange frame */
      error = 2,   /**< red frame */
      lowStat = 3,  /**< gray frame */
      noStat = 4 /**< purple frame */
    };

  };
} // end namespace Belle2

