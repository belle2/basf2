/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/StateCreator.dcl.h>

namespace Belle2 {
  /// Findlet for tagging all space points in the results vector as used
  template<class AnObject, class AState>
  void StateCreator<AnObject, AState>::apply(const std::vector<AnObject*>& objects,
                                             std::vector<AState>& states)
  {
    states.reserve(states.size() + objects.size());

    for (AnObject* object : objects) {
      states.emplace_back(object);
    }
  }
}