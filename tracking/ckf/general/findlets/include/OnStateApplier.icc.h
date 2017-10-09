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

#include <tracking/ckf/general/findlets/OnStateApplier.dcl.h>

namespace Belle2 {
  template <class AState>
  OnStateApplier<AState>::OnStateApplier() : Super()
  {
  };

  template <class AState>
  void OnStateApplier<AState>::exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                                                const std::string& prefix __attribute__((unused)))
  {
  };

  template <class AState>
  void OnStateApplier<AState>::apply(const std::vector<const AState*>& currentPath __attribute__((unused)),
                                     std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates __attribute__((unused)))
  {
  };

  template <class AState>
  bool OnStateApplier<AState>::needsTruthInformation()
  {
    return false;
  };

  template <class AState>
  TrackFindingCDC::Weight OnStateApplier<AState>::operator()(const Object& object __attribute__((unused)))
  {
    return NAN;
  };
}
