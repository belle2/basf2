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

#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.dcl.h>

#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class ATruthVarSet>
    TruthVarFilter<ATruthVarSet>::TruthVarFilter()
      : Super("truth")
    {
    }

    template <class ATruthVarSet>
    TruthVarFilter<ATruthVarSet>::~TruthVarFilter() = default;

    template<class ATruthVarSet>
    auto TruthVarFilter<ATruthVarSet>::operator()(const Object& object) -> Weight {
      Weight value = Super::operator()(object);
      if (std::isnan(value) or value == 0) return NAN;
      return 1.0;
    }
  }
}
