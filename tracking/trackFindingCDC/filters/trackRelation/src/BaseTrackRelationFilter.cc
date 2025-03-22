/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

#include <tracking/trackingUtilities/filters/base/RelationFilter.icc.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

template class TrackingUtilities::RelationFilter<const CDCTrack>;

BaseTrackRelationFilter::BaseTrackRelationFilter() = default;

BaseTrackRelationFilter::~BaseTrackRelationFilter() = default;

std::vector<const CDCTrack*> BaseTrackRelationFilter::getPossibleTos(
  const CDCTrack* from __attribute__((unused)),
  const std::vector<const CDCTrack*>& tracks) const
{
  // All tracks a possible - no geometric selection here.
  return tracks;
}
