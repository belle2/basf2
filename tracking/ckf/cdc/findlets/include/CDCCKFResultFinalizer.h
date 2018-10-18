/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>
#include <tracking/ckf/cdc/filters/paths/CDCPathFilterFactory.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/ckf/cdc/entities/CDCCKFPath.h>
#include <tracking/ckf/cdc/entities/CDCCKFResult.h>

#include <vector>

namespace Belle2 {
  class RecoTrack;

  class ModuleParamList;

  class CDCCKFResultFinalizer : public TrackFindingCDC::Findlet<const CDCCKFPath, CDCCKFResult> {
  public:
    CDCCKFResultFinalizer()
    {
      addProcessingSignalListener(&m_filter);
    }

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      m_filter.exposeParameters(moduleParamList, prefix);
    }

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
    TrackFindingCDC::ChooseableFilter<CDCPathFilterFactory> m_filter;
  };
}
