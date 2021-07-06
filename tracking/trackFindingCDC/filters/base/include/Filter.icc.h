/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <string>
#include <cmath>

namespace Belle2 {
  class ModuleParamList;
}

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class AObject>
    Filter<AObject>::Filter() = default;

    template<class AObject>
    Filter<AObject>::~Filter() = default;

    template <class AObject>
    void Filter<AObject>::exposeParameters(ModuleParamList* moduleParamListw __attribute__((unused)),
                                           const std::string& prefix  __attribute__((unused)))
    {
    }

    template <class AObject>
    bool Filter<AObject>::needsTruthInformation()
    {
      return false;
    }

    template <class AObject>
    Weight Filter<AObject>::operator()(const Object& obj __attribute__((unused)))
    {
      return 1;
    }

    template <class AObject>
    Weight Filter<AObject>::operator()(const Object* obj)
    {
      return obj ? operator()(*obj) : NAN;
    }
  }
}
