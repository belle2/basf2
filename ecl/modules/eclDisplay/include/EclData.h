/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#ifndef ECL_DATA
#define ECL_DATA

#include <TTree.h>
#include <TH1F.h>
#include <vector>
#include <framework/logging/Logger.h>
#include <ecl/dataobjects/ECLDigit.h>

namespace Belle2 {
  /**
   * @brief This class contains data for ECLSimHit's and provides several
   * relevant conversion functions for better event display.
   */
  class EclData {
  private:
    /// Tree with loaded events.
    TTree* m_tree;
    /// Tree channel field
    int ch;
    /// Tree amplitude field
    int amp;
    /// Tree time field
    int time;
    /// Tree event number field.
    int evtn;

    /// Number of events for each crystal.
    int* m_event_counts;
    /// Max value in event_counts array.
    int m_event_count_max;
    /// Sum of amplitudes of every event captured by crystal.
    float* m_energy_sums;
    /// Max value in amp_sums array.
    float m_energy_sums_max;
    /// Total energy for last displayed range of events.
    float m_energy_total;

    /// Id of the event with max recorded event number (evtn).
    int m_last_event_id;

    /// Maximum possible time.
    int m_time_max;
    /// Time range (min) for display. Setting time ranges means EclData will
    /// filter out excluded events in most of the data acquisition methods.
    int m_time_range_min;
    /// Time range (max) for display. Setting time ranges means EclData will
    /// filter out excluded events in most of the data acquisition methods.
    int m_time_range_max;

    /// Events from ev_range_min will be counted in energy_sums and
    /// event_counts. Setting time ranges means EclData will
    /// filter out excluded events in most of the data acquisition methods.
    int m_ev_range_min;
    /// Events up to ev_range_max will be counted in energy_sums and
    /// event_counts. Setting time ranges means EclData will
    /// filter out excluded events in most of the data acquisition methods.
    int m_ev_range_max;

    /// Lower boundary of energy threshold.
    int m_en_range_min;
    /// Upper boundary of energy threshold.
    int m_en_range_max;

    /// Channels which are filtered out from the count of events and energy.
    std::set<int> m_excluded_ch;
    /// This vector holds the position of each entry which starts an event.
    std::vector<int> m_event_entry;

    /// First crystal id in the beginning of i-th ECL ring.
    /// Taken from basf2/ecl/data/ecl_channels_map (two last columns)
    const int ring_start_id[70] = {
      // forward (0-12)
      1,    49,   97,   161,  225,  289,  385,  481,  577,  673,
      769,  865,  1009,
      // barrel (13-58)
      1153, 1297, 1441, 1585, 1729, 1873, 2017, 2161, 2305, 2449,
      2593, 2737, 2881, 3025, 3169, 3313, 3457, 3601, 3745, 3889,
      4033, 4177, 4321, 4465, 4609, 4753, 4897, 5041, 5185, 5329,
      5473, 5617, 5761, 5905, 6049, 6193, 6337, 6481, 6625, 6769,
      6913, 7057, 7201, 7345, 7489, 7633,
      // forward (59-68)
      7777, 7921, 8065, 8161, 8257, 8353, 8449, 8545, 8609, 8673,
      // last_crystal+1
      getCrystalCount() + 1
    };

  public:
    /// Subsystems of ECL:
    /// ALL all subsystems
    /// BARR barrel only
    /// FORW forward endcap only
    /// BACKW backward endcap only
    enum EclSubsystem {ALL, BARR, FORW, BACKW};

    /**
     * @brief Default constructor.
     */
    EclData();

    /**
     * @brief Copy constructor. Resets m_excluded_ch upon copy.
     */
    EclData(const EclData& data);


    ~EclData();

  private:
    /**
     * @brief Initialization of arrays.
     */
    void initVariables();
    /**
     * @brief Set initial values for time and event range.
     */
    void initEventRanges();

  public:
    /**
     * @brief Get number of crystals in ECL.
     */
    int getCrystalCount();

    /**
     * @brief Returns data contained in EclDisplay.
     */
    TTree* getTree();

    /**
     * @brief Returns array of event counts per crystal[getCrystalsCount()].
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
     * @brief Get array of total energy for each channel in the specified time
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
     * @brief Return min time in time range.
     */
    int getTimeRangeMin();
    /**
     * @brief Return max time in time range.
     */
    int getTimeRangeMax();
    /**
     * @brief Display only events in the specified time range.
     * This method currently can't be called from the GUI.
     */
    void setTimeRange(int time_min, int time_max, bool do_update = true);

    /**
     * @brief Return min event number to display.
     */
    int getEventRangeMin();
    /**
     * @brief Return max event number to display.
     */
    int getEventRangeMax();
    /**
     * @brief Sets event range to (ev_min, ev_max)
     * @param do_update Immediately update sums of energy and event count for
     * each channel.
     */
    void setEventRange(int ev_min, int ev_max, bool do_update = true);

    /**
     * @brief Sets energy range to (en_min, en_max).
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

    // TODO: Was unable to test ECLGeometryPar earlier, example crashed.
    // Need to test on another build of basf2.

    /**
     * @brief Converts (phi_id, theta_id) pair to ECL CellId.
     */
    int getChannel(int phi_id, int theta_id);
    /**
     * @brief ECL CellId -> phi_id
     */
    int getPhiId(int ch);
    /**
     * @brief ECL CellId -> theta_id
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
     * Update time_min, time_max, event_counts and amp_sums.
     */
    void update(bool reset_event_ranges = false);
//    bool hasExpanded();

    // Deprecated method.
    //void addEvent(int ch, int amp, int time, int evtn);
    /**
     * @brief Add ECLDigit event to inner TTree (m_tree).
     * @param event ECLDigit event
     * @param evtn Number of event.
     * @return If ECLDigit contains incorrect data, negative values are returned. Otherwise, return value is 0.
     */
    int addEvent(ECLDigit* event, int evtn);
    /**
     * @brief Fill amplitude per channel histogram for the specified EclSubsystem
     * (Barrel, forward endcap, backward endcap, all of them).
     */
    void fillAmpHistogram(TH1F* hist, int amp_min, int amp_max, EclSubsystem subsys);
    /**
     * @brief Fill amplitude per event histogram for the specified EclSubsystem
     * (Barrel, forward endcap, backward endcap, all of them).
     */
    void fillAmpSumHistogram(TH1F* hist, int amp_min, int amp_max, EclSubsystem subsys);
    /**
     * @brief Fill time histogram for the specified EclSubsystem
     * (Barrel, forward endcap, backward endcap, all of them).
     */
    void fillTimeHistogram(TH1F* hist, int time_min, int time_max, EclSubsystem subsys);
  };
}

#endif // ECL_DATA
