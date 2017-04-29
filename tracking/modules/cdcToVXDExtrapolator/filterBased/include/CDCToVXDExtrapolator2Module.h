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

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/modules/cdcToVXDExtrapolator/filterBased/CDCToVXDExtrapolatorFindlet.h>

namespace Belle2 {
  class CDCToVXDExtrapolator2Module : public TrackFindingCDC::FindletModule<CDCToVXDExtrapolatorFindlet> {

  public:
    CDCToVXDExtrapolator2Module()
    {
      setDescription("");
    }
  };
}