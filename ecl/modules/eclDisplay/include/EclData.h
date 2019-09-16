/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#pragma once

//STL
#include <vector>
#include <set>

//Root
#include <TTree.h>
#include <TH1F.h>

class TTree;


namespace Belle2 {

  class ECLCalDigit;

  /**
   * This class contains data for ECLSimHit's and provides several
   * relevant conversion functions for better event display.
   */
  class EclData {
  private:
    /**  Tree with loaded events. */
    TTree* m_tree;
    /**  Tree channel field */
    int m_branch_ch;
    /**  Tree energy branch */
    double m_branch_energy;
    /**  Tree time branch */
    double m_branch_time;
    /**  Tree event number branch. */
    int m_branch_evtn;

    /**  Number of events for each crystal. */
    int* m_event_counts;
    /**  Max value in event_counts array. */
    int m_event_count_max;
    /**  Sum of energies of every event captured by crystal (MeV). */
    float* m_energy_sums;
    /**  Max value in m_energy_sums array. */
    float m_energy_sums_max;
    /**  Total energy for last displayed range of events. */
    float m_energy_total;

    /**  Id of the event with max recorded event number (evtn). */
    int m_last_event_id;

    /**  Maximum possible time. */
    int m_time_max;
    /**  Time range (min) for display. Setting time ranges means EclData will
     *   filter out excluded events in most of the data acquisition methods. */
    int m_time_range_min;
    /**  Time range (max) for display. Setting time ranges means EclData will
     *   filter out excluded events in most of the data acquisition methods. */
    int m_time_range_max;

    /**  Events from ev_range_min will be counted in energy_sums and
     *   event_counts. Setting time ranges means EclData will
     *   filter out excluded events in most of the data acquisition methods. */
    int m_ev_range_min;
    /**  Events up to ev_range_max will be counted in energy_sums and
     *   event_counts. Setting time ranges means EclData will
     *   filter out excluded events in most of the data acquisition methods. */
    int m_ev_range_max;

    /**  Lower boundary of energy threshold. */
    int m_en_range_min;
    /**  Upper boundary of energy threshold. */
    int m_en_range_max;

    /**  Channels which are filtered out from the count of events and energy. */
    std::set<int> m_excluded_ch;
    /**  This vector holds the position of each entry which starts an event. */
    std::vector<int> m_event_entry;

    /**  First crystal id in the beginning of i-th ECL ring.
     *  Taken from basf2/ecl/data/ecl_channels_map (two last columns)
     */
    static const int ring_start_id[70];

  public:
    /**  Subsystems of ECL:
     *  ALL all subsystems
     *  BARR barrel only
     *  FORW forward endcap only
     *  BACKW backward endcap only
     */
    enum EclSubsystem {ALL, BARR, FORW, BACKW};

    /**
     * Default constructor.
     */
    EclData();

    /**
     * Copy constructor. Resets m_excluded_ch upon copy.
     */
    EclData(const EclData& data);
    /**
     * Assignment operator: utilizes copy constructor.
     */
    EclData& operator=(const EclData& other);


    ~EclData();

  private:
    /**
     * Clone attributes from other EclData.
     */
    void cloneFrom(const EclData& other);

    /**
     * Initialization of arrays.
     */
    void initVariables();
    /**
     * Set initial values for time and event range.
     */
    void initEventRanges();

  public:
    /**
     * Get number of crystals in ECL.
     */
    static int getCrystalCount();

    /**
     * Returns data contained in EclDisplay.
     */
    TTree* getTree();

    /**
     * Returns array of event counts per crystal[getCrystalsCount()].
     */
    int* getEventCounts();
    /**
     * Alias for GetEventCounts()
     */
    int* getEventCountsPerCrystal();
    /**
     * @return Maximum value in array of event counts.
     */
    int getEventCountsMax();

    /**
     * Get array of total energy for each channel in the specified time
     * and event range.
     */
    float* getEnergySums();
    /**
     * Alias for GetEnergySums()
     */
    float* getEnergySumPerCrystal();
    /**
     * @return Maximum value in array of energy sums.
     */
    float getEnergySumsMax();
    /**
     * @return Get total energy in currently set range of time and/or events.
     */
    float getEnergyTotal();

    /**
     * @param subsys ID of the ECL subsystem.
     * @return Whether crystal is in barrel or forward/backward endcap.
     */
    bool isCrystalInSubsystem(int crystal, EclSubsystem subsys);

    /**
     * Return min time in time range.
     */
    int getTimeRangeMin();
    /**
     * Return max time in time range.
     */
    int getTimeRangeMax();
    /**
     * Display only events in the specified time range.
     * This method currently can't be called from the GUI.
     */
    void setTimeRange(int time_min, int time_max, bool do_update = true);

    /**
     * Return min event number to display.
     */
    int getEventRangeMin();
    /**
     * Return max event number to display.
     */
    int getEventRangeMax();
    /**
     * Sets event range to (ev_min, ev_max)
     * @param do_update Immediately update sums of energy and event count for
     * each channel.
     */
    void setEventRange(int ev_min, int ev_max, bool do_update = true);

    /**
     * Sets energy range to (en_min, en_max).
     * @param do_update Immediately update sums of energy and event count for
     * each channel.
     */
    void setEnergyThreshold(int en_min, int en_max, bool do_update = true);

    /**
     * @return Time of the first event in TTree.
     */
    int getTimeMin();
    /**
     * @return Time of the last event in TTrees.
     */
    int getTimeMax();

    /**
     * @return Id of the last event.
     */
    int getLastEventId();

    /**
     * Converts (phi_id, theta_id) pair to ECL CellId.
     */
    int getChannel(int phi_id, int theta_id);
    /**
     * ECL CellId -> phi_id
     */
    int getPhiId(int ch);
    /**
     * ECL CellId -> theta_id
     */
    int getThetaId(int ch);

    /**
     * Excludes specific channel from the count of events and energy.
     */
    void excludeChannel(int ch, bool do_update = false);
    /**
     * Includes specific channel in the count of events and energy.
     */
    void includeChannel(int ch, bool do_update = false);

    /**
     * Load root file containing ECLCalDigit data from the specified path.
     */
    void loadRootFile(const char* path);

    /**
     * Update time_min, time_max, event_counts and energy_sums.
     */
    void update(bool reset_event_ranges = false);

    /**
     * Add ECLDigit event to inner TTree (m_tree).
     * @param event ECLDigit event
     * @param evtn Number of event.
     * @return If ECLDigit contains incorrect data, negative values are returned. Otherwise, return value is 0.
     */
    int addEvent(ECLCalDigit* event, int evtn);
    /**
     * Fill energy per channel histogram for the specified EclSubsystem
     * (Barrel, forward endcap, backward endcap, all of them).
     */
    void fillEnergyHistogram(TH1F* hist, int energy_min, int energy_max, EclSubsystem subsys);
    /**
     * Fill energy per event histogram for the specified EclSubsystem
     * (Barrel, forward endcap, backward endcap, all of them).
     */
    void fillEnergySumHistogram(TH1F* hist, int energy_min, int energy_max, EclSubsystem subsys);
    /**
     * Fill time histogram for the specified EclSubsystem
     * (Barrel, forward endcap, backward endcap, all of them).
     */
    void fillTimeHistogram(TH1F* hist, int time_min, int time_max, EclSubsystem subsys);
  };
}
