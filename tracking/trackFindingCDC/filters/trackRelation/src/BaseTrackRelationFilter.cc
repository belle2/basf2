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

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::Filter<Relation<const CDCTrack> >;

std::vector<const CDCTrack*> BaseTrackRelationFilter::getPossibleNeighbors(
  const CDCTrack* track __attribute__((unused)),
  const std::vector<const CDCTrack*>::const_iterator& itBegin,
  const std::vector<const CDCTrack*>::const_iterator& itEnd) const
{
  return {itBegin, itEnd};
}

Weight BaseTrackRelationFilter::operator()(const CDCTrack& from __attribute__((unused)),
                                           const CDCTrack& to __attribute__((unused)))
{
  return 1;
}

Weight BaseTrackRelationFilter::operator()(const Relation<const CDCTrack>& relation)
{
  const CDCTrack* ptrFrom(relation.first);
  const CDCTrack* ptrTo(relation.second);
  if (ptrFrom == ptrTo) return NAN; // Prevent relation to same.
  if ((ptrFrom == nullptr) or (ptrTo == nullptr)) return NAN;
  return this->operator()(*ptrFrom, *ptrTo);
}
