/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
