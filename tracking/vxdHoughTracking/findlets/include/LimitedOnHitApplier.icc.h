/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/findlets/LimitedOnHitApplier.dcl.h>
#include <tracking/trackingUtilities/utilities/Functional.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>
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

      moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "useNBestHits"), m_useNHits, "Only use the best N hits",
                                    m_useNHits);
    };

    template <class AHit, class AFilter>
    void LimitedOnHitApplier<AHit, AFilter>::apply(const std::vector<TrackingUtilities::WithWeight<const AHit*>>& currentPath,
                                                   std::vector<TrackingUtilities::WithWeight<AHit*>>& childHits)
    {
      Super::apply(currentPath, childHits);

      if (m_useNHits > 0 and childHits.size() > static_cast<unsigned int>(m_useNHits)) {
        std::sort(childHits.begin(), childHits.end(), TrackingUtilities::GreaterOf<TrackingUtilities::GetWeight>());
        childHits.erase(childHits.begin() + m_useNHits, childHits.end());
      }
    };

    template <class AHit, class AFilter>
    TrackingUtilities::Weight LimitedOnHitApplier<AHit, AFilter>::operator()(const Object& object)
    {
      return m_filter(object);
    };

  }
}
