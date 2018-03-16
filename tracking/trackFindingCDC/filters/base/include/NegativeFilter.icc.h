/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/NegativeFilter.dcl.h>

#include <memory>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    template<class AFilter>
    Weight NegativeFilter<AFilter>::operator()(const typename AFilter::Object& obj)
    {
      return -Super::operator()(obj);
    }
  }
}
