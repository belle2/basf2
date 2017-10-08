/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::RelationFilter<const CDCTrack>;

BaseTrackRelationFilter::BaseTrackRelationFilter() = default;

BaseTrackRelationFilter::~BaseTrackRelationFilter() = default;

std::vector<const CDCTrack*> BaseTrackRelationFilter::getPossibleNeighbors(
  const CDCTrack* track __attribute__((unused)),
  const std::vector<const CDCTrack*>::const_iterator& itBegin,
  const std::vector<const CDCTrack*>::const_iterator& itEnd) const
{
  return {itBegin, itEnd};
}
