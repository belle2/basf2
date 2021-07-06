/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>
#include <tracking/ckf/cdc/filters/paths/CDCPathFilterFactory.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/ckf/cdc/entities/CDCCKFPath.h>
#include <tracking/ckf/cdc/entities/CDCCKFResult.h>

#include <vector>

namespace Belle2 {
  class ModuleParamList;

  /// Findlet to finalize CKF Paths in terms of final result.
  class CDCCKFResultFinalizer : public TrackFindingCDC::Findlet<const CDCCKFPath, CDCCKFResult> {
  public:
    CDCCKFResultFinalizer()
    {
      addProcessingSignalListener(&m_filter);
    }

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      m_filter.exposeParameters(moduleParamList, prefix);
    }

    /// main method of the findlet, for a list of paths create a list of results.
    void apply(const std::vector<CDCCKFPath>& paths, std::vector<CDCCKFResult>& results) override
    {
      if (paths.empty()) {
        return;
      }

      const CDCCKFPath* bestElement = nullptr;
      TrackFindingCDC::Weight bestWeight = -NAN;

      for (const CDCCKFPath& path : paths) {
        const TrackFindingCDC::Weight weight = m_filter(path);
        if (weight <= bestWeight) {
          continue;
        }
        bestWeight = weight;
        bestElement = &path;
      }

      if (bestElement and not std::isnan(bestWeight)) {
        results.push_back(*bestElement);
      }
    }

  private:
    /// Filter to weigth the best path
    TrackFindingCDC::ChooseableFilter<CDCPathFilterFactory> m_filter;
  };
}
