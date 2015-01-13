/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/NeighborWeight.h"

#include <limits>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;




const NeighborWeight CDCLocalTracking::NOT_A_NEIGHBOR = std::numeric_limits<Weight>::quiet_NaN();;
