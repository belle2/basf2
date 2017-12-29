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
  /// Findlet for tagging all space points in the results vector as used
  template <class AnObject, class AState>
  class StateCreator : public TrackFindingCDC::Findlet<AnObject* const, AState> {
  public:
    /// Mark all space points as used, that they share clusters if the given kind with the results.
    void apply(const std::vector<AnObject*>& objects,
               std::vector<AState>& states) override;
  };
}
