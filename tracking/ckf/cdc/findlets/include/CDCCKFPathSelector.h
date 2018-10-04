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
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

namespace Belle2 {
  class CDCCKFPathSelector : public TrackFindingCDC::Findlet<CDCCKFPath>  {
  public:
    void apply(std::vector<CDCCKFPath>& newPaths) override
    {
      // TODO
    }
  };
}
