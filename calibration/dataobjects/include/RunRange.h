#pragma once

#include <string>
#include <vector>
#include <set>
#include <TNamed.h>
#include <TCollection.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>
#include <calibration/Utilities.h>

namespace Belle2 {
  /// Mergeable object holding (unique) set of (exp,run) pairs
  class RunRange : public TNamed {

  public:
    /// Constructor
    RunRange() : TNamed() {};

    /// Constructor from vector of ExpRun objects
    explicit RunRange(std::vector<Calibration::ExpRun> expRuns) : TNamed()
    {
      for (auto expRun : expRuns) {
        this->add(expRun.first, expRun.second);
      }
    };

    /// Destructor
    virtual ~RunRange()
    {
      m_expRunSet.clear();
    }

    /// Implementation of clearing - resets stored run set
    virtual void clear()
    {
      m_expRunSet.clear();
    }

    /// Implementation of merging - other is added to the set (union)
    virtual void merge(const RunRange* other)
    {
      m_expRunSet.insert(other->m_expRunSet.begin(), other->m_expRunSet.end());
    }

    /// Add an experiment and run number to the set
    void add(int exp, int run)
    {
      m_expRunSet.insert(std::make_pair(exp, run));
    }

    /// Get access to the stored set
    const std::set<Calibration::ExpRun>& getExpRunSet()
    {
      return m_expRunSet;
    }

    /// Make IntervalOfValidity from the set, spanning all runs. Works because sets are sorted by default
    IntervalOfValidity getIntervalOfValidity()
    {
      if (m_expRunSet.empty())
        return IntervalOfValidity();

      auto low = m_expRunSet.begin();
      auto high = m_expRunSet.rbegin();

      return IntervalOfValidity(low->first, low->second, high->first, high->second);
    }

    /// Allow merging using TFileMerger if saved directly to a file.
    Long64_t Merge(TCollection* hlist)
    {
      B2DEBUG(100, "Running Merge() on " << this->GetName());
      Long64_t nMerged = 0;
      if (hlist) {
        const RunRange* xh = 0;
        TIter nxh(hlist);
        while ((xh = dynamic_cast<RunRange*>(nxh()))) {
          // Add xh to me
          merge(xh);
          ++nMerged;
        }
      }
      B2DEBUG(100, "Merged " << nMerged << " objects");
      return nMerged;
    }

    /// Set the m_granularity to an allowed value
    void setGranularity(std::string& granularity)
    {
      if (granularity == "all" || granularity == "run") {
        m_granularity = granularity;
      } else {
        B2WARNING("Tried to set RunRange granularity to something other than 'run' or 'all' -> " << granularity);
      }
    }

    /// Gets the m_granularity
    std::string getGranularity() const {return m_granularity;}
    /** Root-like Reset function for "template compatibility" with ROOT objects. Alias for clear(). */
    void Reset() {clear();}
    /// Root-like SetDirectory function for "template compatibility" with ROOT objects. Does nothing.
    void SetDirectory(TDirectory*) {}

    /// Comparison operator so that we can use RunRange in a map as a key
    bool operator<(const RunRange& other) const
    {
      if (m_granularity == other.m_granularity) {
        return m_expRunSet < other.m_expRunSet;
      }
      return m_granularity < other.m_granularity;
    }

  private:
    /// The set of (exp,run) stored in object
    std::set<Calibration::ExpRun> m_expRunSet = {};

    /// granularity used by the collector storing the information.
    std::string m_granularity = "run";

    ClassDef(RunRange, 2) /**< Mergeable set of (exp,run) pairs */
  };
}
