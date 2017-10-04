/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/OberverFilter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <framework/logging/Logger.h>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class AFilter>
    void ObserverFilter<AFilter>::terminate()
    {
      B2INFO("Filter said " << m_yesAnswers << " times yes and " << m_noAnswers << " times no.");
      Super::terminate();
    }

    template<class AFilter>
    Weight ObserverFilter<AFilter>::operator()(const Object& object)
    {
      Weight result = Super::operator()(object);
      if (std::isnan(result)) {
        m_noAnswers += 1;
      } else {
        m_yesAnswers += 1;
      }

      return result;
    }
  }
}
