/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/cluster/AllClusterFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllClusterFilter::operator()(const CDCWireHitCluster& cluster)
{
  return cluster.size();
}
