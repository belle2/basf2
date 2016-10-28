#pragma once

#include <string>
#include <vector>

#include <framework/pcore/Mergeable.h>

#include <set>
#include <framework/database/IntervalOfValidity.h>

namespace Belle2 {
  /// Mergeable object holding (unique) set of (exp,run) pairs
  class RunRange : public Mergeable {

  public:
    /// Constructor
    RunRange() : Mergeable() {};

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
    virtual void merge(const Mergeable* other)
    {
      const RunRange* to_merge = static_cast<const RunRange*>(other);
      m_expRunSet.insert(to_merge->m_expRunSet.begin(), to_merge->m_expRunSet.end());
    }

    /// Add an experiment and run number to the set
    void add(int exp, int run)
    {
      m_expRunSet.insert(std::make_pair(exp, run));
    }

    /// Get access to the stored set
    const std::set<std::pair<int, int>>& getExpRunSet()
    {
      return m_expRunSet;
    }

    /// Make IntervalOfValidity from the set, spanning all runs
    IntervalOfValidity getIntervalOfValidity()
    {
      if (m_expRunSet.empty())
        return IntervalOfValidity();

      auto low = m_expRunSet.begin();
      auto high = m_expRunSet.rbegin();

      return IntervalOfValidity(low->first, low->second, high->first, high->second);
    }

  private:
    /// The set of (exp,run) stored in object
    std::set<std::pair<int, int>> m_expRunSet = {};

    ClassDef(RunRange, 1) /**< Mergeable set of (exp,run) pairs */
  };
}
