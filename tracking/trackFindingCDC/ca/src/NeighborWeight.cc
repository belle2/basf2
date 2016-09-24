/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/ca/NeighborWeight.h>

#include <limits>

using namespace Belle2;
using namespace TrackFindingCDC;




const NeighborWeight TrackFindingCDC::NOT_A_NEIGHBOR = std::numeric_limits<Weight>::quiet_NaN();;
