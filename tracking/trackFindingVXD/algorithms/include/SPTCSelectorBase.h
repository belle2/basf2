/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>

namespace Belle2 {

  class SPTCSelectorBase {
  public:
    virtual ~SPTCSelectorBase() = default;

    virtual void addSPTC(std::vector<SpacePointTrackCand>& target, SpacePointTrackCand sptc) const
    {
      target.push_back(sptc);
    }
  };
}
