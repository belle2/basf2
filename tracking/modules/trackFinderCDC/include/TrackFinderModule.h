/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/complete/TrackFinder.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Module for the track finder combining global and local search
    class TFCDC_TrackFinderModule : public FindletModule<TrackFinder> {
    };
  }
}
