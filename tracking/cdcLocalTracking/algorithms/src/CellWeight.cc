/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CellWeight.h"

#include <limits>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;





const CellWeight CDCLocalTracking::NOT_A_CELL = std::numeric_limits<Weight>::quiet_NaN();


