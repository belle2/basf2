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

#include <tracking/trackFindingCDC/filters/base/PassThroughFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class AFilter>
    Weight PassThroughFilter<AFilter>::operator()(const Object& object)
    {
      return object.getWeight();
    }
  }
}
