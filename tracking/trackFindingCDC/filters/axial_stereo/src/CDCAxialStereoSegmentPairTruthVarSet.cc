/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CDCAxialStereoSegmentPairTruthVarSet.h"
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCAxialStereoSegmentPairTruthVarSet::CDCAxialStereoSegmentPairTruthVarSet(const std::string& prefix) :
  VarSet<CDCAxialStereoSegmentPairTruthVarNames>(prefix)
{
}

bool CDCAxialStereoSegmentPairTruthVarSet::extract(const CDCAxialStereoSegmentPair* ptrSegmentPair)
{
  bool extracted = extractNested(ptrSegmentPair);
  if (not extracted or not ptrSegmentPair) return false;

  const CDCAxialStereoSegmentPair& segmentPair = *ptrSegmentPair;

  const CellWeight mcWeight = m_mcAxialStereoSegmentPairFilter(segmentPair);
  var<named("truth")>() =  not isNotACell(mcWeight);
  return true;
}

void CDCAxialStereoSegmentPairTruthVarSet::initialize()
{
  m_mcAxialStereoSegmentPairFilter.initialize();
}

void CDCAxialStereoSegmentPairTruthVarSet::terminate()
{
  m_mcAxialStereoSegmentPairFilter.terminate();
}
