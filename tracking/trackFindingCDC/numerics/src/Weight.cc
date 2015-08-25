/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <limits>

using namespace Belle2;
using namespace TrackFindingCDC;

const Weight TrackFindingCDC::HIGHEST_WEIGHT = std::numeric_limits<Weight>::infinity();
const Weight TrackFindingCDC::LOWEST_WEIGHT = -std::numeric_limits<Weight>::infinity();
