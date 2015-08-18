/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/ca/CellState.h>

#include <limits>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;





const CellState TrackFindingCDC::NO_CONTINUATION = -std::numeric_limits<Weight>::infinity();



