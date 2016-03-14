/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/AllClusterFilter.h"

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CellWeight AllClusterFilter::isGoodCluster(const CDCWireHitCluster& cluster)
{
  return cluster.size();
}

