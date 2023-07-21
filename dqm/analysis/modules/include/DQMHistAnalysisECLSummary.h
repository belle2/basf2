/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

//DQM
#include <dqm/core/DQMHistAnalysis.h>

//ROOT
#include <TCanvas.h>
#include <TH2F.h>
#include <TExec.h>

//std
#include <set>

//ECL
#include <ecl/mapper/ECLChannelMapper.h>
#include <ecl/geometry/ECLNeighbours.h>

namespace Belle2 {

  /**
   * This module is for analysis of ECL DQM histograms.
   */

  class DQMHistAnalysisECLSummaryModule final : public DQMHistAnalysisModule { /**< derived from DQMHistAnalysisModule class. */

  public:

    /** Constructor. */
    DQMHistAnalysisECLSummaryModule();

    /** Destructor. */
    ~DQMHistAnalysisECLSummaryModule();

    /** Initialize the module. */
    void initialize() override final;
    /** Call when a run begins. */
    void beginRun() override final;
    /** Event processor. */
    void event() override final;
    /** Call when a run ends. */
    void endRun() override final;
    /** Terminate. */
    void terminate() override final;

    /**
     * Get outlier channels, ones with values (occupancy, errors, etc) that are much
     * higher than the values of their neghbours.
     *
     * @param  hist                  Histogram that is used to search for outliers
     * @param  total_events          Total events in the run (after filter)
     * @param  neighbours            vector [Cell ID - 1] -> [vector of neighbours' Cell IDs]
     * @param  max_deviation         Maximum acceptable deviation (relative) from the expected value
     * @param  occupancy_histogram   If true, search for dead/cold/hot channels, otherwise search for bad_chi2 channels
     *
     * @return     map: [Cell ID] -> [bit mask for channel problems]
     *
     * Cell IDs correspond to the range of [1, 8736].
     */
    std::map<int, int> getSuspiciousChannels(TH1* hist, double total_events,
                                             const std::vector< std::vector<short> >& neighbours,
                                             double max_deviation, bool occupancy_histogram);

  private:
    /** Definition for different ECL alarm types */
    struct ECLAlarmType {
      std::string name; /**< Internal name */
      std::string title; /**< Name to be shown on canvas */
      int warning_limit; /**< If value > warning_limit, show warning */
      int alarm_limit; /**< If value > alarm_limit, show error*/
      double required_statistics; /**< Minimum number of events for this alarm to be active */
    };

    /** Returns index of the specific alarm type and detailed information */
    std::pair<int, ECLAlarmType> getAlarmByName(std::string name);

    /** Set alarm limits in DQM based on EPICS PV limits */
    void updateAlarmConfig();

    /** Calculate and return number of problems per crate for all alarm categories
     * @return     vector: [alarm_type][crate_id - 1] -> number of times the specific issue occurred for that crate
     */
    std::vector< std::vector<int> > updateAlarmCounts(bool update_mirabelle = false);

    /**
     * @return     map: [Cell ID] -> [bit mask for channel problems]
     */
    std::map<int, int> getChannelsWithOccupancyProblems();

    /**
     * @return     map: [Cell ID] -> [bit mask for channel problems]
     */
    std::map<int, int> getChannelsWithChi2Problems();

    /** Draw grid with TLine primitives for the specified histogram */
    void drawGrid(TH2* hist);

  private:
    /** Prefix to use for PVs registered by this module.
     *  DQM framework will also prepend either 'TEST:' or 'DQM:' prefix.
     */
    std::string m_pvPrefix;
    /** If true, mask Cell IDs based on DQM:ECL:channels_info:{alarm_type} PV info */
    bool m_useChannelMask;

    /** Different alarms monitored in h_channels_summary */
    std::vector<ECLAlarmType> m_ecl_alarms;

    /** List of masked Cell IDs for each alarm type */
    std::map<std::string, std::set<int> > m_mask;

    /** Number of events with ECL data in the current run */
    double m_total_events = 0;

    /** Summarized information about ECL channels */
    TH2F* h_channels_summary = nullptr;
    /** TCanvas for ECL alarms regarding suspicious channels */
    TCanvas* c_channels_summary = nullptr;
    /** Special object to specify style parameters */
    TExec* m_ecl_style = nullptr;
    /** Special object to revert changes done by ecl_style */
    TExec* m_default_style = nullptr;
    /** Object to map ECL crystal ID to ECL crate ID */
    ECL::ECLChannelMapper m_mapper;
    /** Object to get ECL crystal neighbours */
    ECL::ECLNeighbours m_neighbours_obj;

    /** ECL occupancy histogram with highlighted suspicious channels */
    TCanvas* c_occupancy = nullptr;
    /** ECL bad chi2 histogram with highlighted suspicious channels */
    TCanvas* c_bad_chi2 = nullptr;
    TH1F* h_bad_occ_overlay = nullptr;  /**< Overlay to indicate bad channels on occupancy histogram */
    TH1F* h_bad_chi2_overlay = nullptr; /**< Overlay to indicate bad channels on chi2 histogram */
    TH1F* h_bad_occ_overlay_green = nullptr;  /**< Overlay to indicate masked bad channels on occupancy histogram */
    TH1F* h_bad_chi2_overlay_green = nullptr; /**< Overlay to indicate masked bad channels on chi2 histogram */

    MonitoringObject* m_monObj = nullptr; /**< MiraBelle monitoring object */
  };
} // end namespace Belle2

