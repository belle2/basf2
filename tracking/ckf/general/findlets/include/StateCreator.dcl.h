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
#include <vector>

namespace Belle2 {
  /// Create new states and add them to a vector from a given object vector.
  template <class AnObject, class AState>
  class StateCreator : public TrackFindingCDC::Findlet<AnObject* const, AState> {
  public:
    /// Add new states to the list of states using all given objects.
    void apply(const std::vector<AnObject*>& objects,
               std::vector<AState>& states) override;
  };
}
