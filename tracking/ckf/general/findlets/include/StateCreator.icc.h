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

#include <tracking/ckf/general/findlets/StateCreator.dcl.h>

namespace Belle2 {
  /// Findlet for tagging all space points in the results vector as used
  template<class AnObject, class AState>
  void StateCreator<AnObject, AState>::apply(const std::vector<AnObject*>& objects,
                                             std::vector<AState>& states)
  {
    for (AnObject* object : objects) {
      states.emplace_back(object);
    }
  }
}