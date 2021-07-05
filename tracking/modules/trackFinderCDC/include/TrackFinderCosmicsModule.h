/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/complete/TrackFinderCosmics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Module for the cellular automaton tracking for the CDC on cosmic events
    class TFCDC_TrackFinderCosmicsModule : public FindletModule<TrackFinderCosmics> {
    };
  }
}
