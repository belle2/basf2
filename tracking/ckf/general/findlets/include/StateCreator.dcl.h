/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
