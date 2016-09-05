#ifndef ECL_DATA
#define ECL_DATA

#include <TSystem.h>
#include <TTree.h>
#include <TH1F.h>
#include <set>
#include <vector>

namespace Belle2 {
  class EclData {
  private:
    TTree* m_tree;
    int ch;
    int amp;
    int time;
    int evtn;
    bool m_expanded;

    /// Number of events for each crystal.
    int* m_event_counts;
    /// Max value in event_counts array.
    int m_event_count_max;
    /// Sum of amplitudes of every event captured by crystal.
    float* m_energy_sums;
    /// Max value in amp_sums array.
    float m_energy_sums_max;
    float m_energy_emission;

    /// Currently this variable is not updating during Update() and may shortly
    /// become obsolete.
    int m_time_max;

    int m_last_event_id;

    int m_time_range_min;
    int m_time_range_max;

    /// Events from ev_range_min will be counted in energy_sums and
    /// event_counts.
    int m_ev_range_min;
    /// Events up to ev_range_max will be counted in energy_sums and
    /// event_counts.
    int m_ev_range_max;

    /// Lower boundary of energy threshold.
    int m_en_range_min;
    /// Upper boundary of energy threshold.
    int m_en_range_max;

    /// Channels which are filtered out from the count of events and energy.
    std::set<int> m_excluded_ch;
    /// This vector holds the position of each entry which starts an event.
    std::vector<int> m_event_entry;

    /// File descriptor
    int m_in;

  public:
    EclData(int in);
    //EclData(const char* filename, bool update = true);
    //EclData(TTree* tree, bool update = true);

    ~EclData();

  private:
    void InitVariables();
    void InitEventRanges();

  public:
    void AddFile(const char* filename, bool update = true);
    TTree* GetTree();

    // TODO: This might be better categorized by enum.

    int* GetEventCounts();
    // Alias for GetEventCounts()
    int* GetEventCountsPerCrystal();
    /**
     * @return Maximum value in array of event counts.
     */
    int GetEventCountsMax();

    float* GetEnergySums();
    // Alias for GetEnergySums()
    float* GetEnergySumPerCrystal();
    /**
     * @return Maximum value in array of energy sums.
     */
    float GetEnergySumsMax();
    float GetEnergyEmission();

    // TODO: are there some fancy Root classes for this occurence?
    int GetTimeRangeMin();
    int GetTimeRangeMax();
    void SetTimeRange(int time_min, int time_max, bool update = true);

    int GetEventRangeMin();
    int GetEventRangeMax();
    void SetEventRange(int ev_min, int ev_max, bool update = true);

    void SetEnergyThreshold(int en_min, int en_max, bool update = true);

    /**
     * @return Time of the first event in TChain.
     */
    int GetTimeMin();
    /**
     * @return Time of the last event in TChain.
     */
    int GetTimeMax();

    /**
     * @return Id of the last event.
     */
    int GetLastEventId();

    /**
     * Excludes specific channel from the count of events and energy.
     */
    void ExcludeChannel(int ch, bool update = false);
    void IncludeChannel(int ch, bool update = false);

    /**
     * Update time_min, time_max, event_counts and amp_sums.
     */
    void Update();

    bool HasExpanded();
    void Deserialize();
    void FillAmpHistogram(TH1F* hist, int amp_min, int amp_max);
  };
}

#endif // ECL_DATA
