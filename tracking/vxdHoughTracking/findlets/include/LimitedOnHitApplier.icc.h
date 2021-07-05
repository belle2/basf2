/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/findlets/LimitedOnHitApplier.dcl.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <algorithm>

namespace Belle2 {
  namespace vxdHoughTracking {

    template <class AHit, class AFilter>
    LimitedOnHitApplier<AHit, AFilter>::LimitedOnHitApplier()
    {
      this->addProcessingSignalListener(&m_filter);
    };

    template <class AHit, class AFilter>
    void LimitedOnHitApplier<AHit, AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      m_filter.exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useNBestHits"), m_param_useNHits, "Only use the best N hits",
                                    m_param_useNHits);
    };

    template <class AHit, class AFilter>
    void LimitedOnHitApplier<AHit, AFilter>::apply(const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& currentPath,
                                                   std::vector<TrackFindingCDC::WithWeight<AHit*>>& childHits)
    {
      Super::apply(currentPath, childHits);

      if (m_param_useNHits > 0 and childHits.size() > static_cast<unsigned int>(m_param_useNHits)) {
        std::sort(childHits.begin(), childHits.end(), TrackFindingCDC::GreaterOf<TrackFindingCDC::GetWeight>());
        childHits.erase(childHits.begin() + m_param_useNHits, childHits.end());
      }
    };

    template <class AHit, class AFilter>
    TrackFindingCDC::Weight LimitedOnHitApplier<AHit, AFilter>::operator()(const Object& object)
    {
      return m_filter(object);
    };

  }
}
