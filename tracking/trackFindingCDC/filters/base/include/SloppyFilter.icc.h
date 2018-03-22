/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/SloppyFilter.dcl.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <TRandom.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class AFilter>
    void Sloppy<AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      Super::exposeParameters(moduleParamList, prefix);
      moduleParamList->addParameter("sloppinessFactor",
                                    m_param_sloppinessFactor,
                                    "Only accept every <sloppinessFactor>th instance "
                                    "that would normally pass the filter.",
                                    m_param_sloppinessFactor);
    }

    template<class AFilter>
    Weight Sloppy<AFilter>::operator()(const typename Super::Object& object)
    {
      Weight response = Super::operator()(object);

      if (std::isnan(response)) {
        const unsigned int randomNumber = gRandom->Integer(m_param_sloppinessFactor);
        if (randomNumber != 0) {
          return NAN;
        }
      }
      return response;
    }
  }
}
