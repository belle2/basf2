#pragma once

#include <string>
#include <vector>
#include <set>
#include <TNamed.h>
#include <TCollection.h>
#include <framework/database/IntervalOfValidity.h>
#include <calibration/Utilities.h>

namespace Belle2 {
  /// Mergeable object holding (unique) set of (exp,run) pairs
  class RunRangeNew : public TNamed {

  public:
    /// Constructor
    RunRangeNew() : TNamed() {};

    /// Destructor
    virtual ~RunRangeNew()
    {
      m_expRunSet.clear();
    }

    /// Implementation of clearing - resets stored run set
    virtual void clear()
    {
      m_expRunSet.clear();
    }

    /// Implementation of merging - other is added to the set (union)
    virtual void merge(const RunRangeNew* other)
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

    /// Make IntervalOfValidity from the set, spanning all runs (NOT CONVINCED THIS IS RIGHT!)
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
        const RunRangeNew* xh = 0;
        TIter nxh(hlist);
        while ((xh = dynamic_cast<RunRangeNew*>(nxh()))) {
          // Add xh to me
          merge(xh);
          ++nMerged;
        }
      }
      B2DEBUG(100, "Merged " << nMerged << " objects");
      return nMerged;
    }

  private:
    /// The set of (exp,run) stored in object
    std::set<Calibration::ExpRun> m_expRunSet = {};

    ClassDef(RunRangeNew, 1) /**< Mergeable set of (exp,run) pairs */
  };
}
