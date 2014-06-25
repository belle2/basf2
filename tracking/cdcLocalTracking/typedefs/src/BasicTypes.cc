/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/BasicTypes.h"

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <limits>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


const Weight CDCLocalTracking::HIGHEST_WEIGHT = std::numeric_limits<Weight>::infinity();

const Weight CDCLocalTracking::LOWEST_WEIGHT = -std::numeric_limits<Weight>::infinity();
