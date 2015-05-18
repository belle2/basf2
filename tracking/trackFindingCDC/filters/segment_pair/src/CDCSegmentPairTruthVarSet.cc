/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CDCSegmentPairTruthVarSet.h"
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCSegmentPairTruthVarSet::CDCSegmentPairTruthVarSet(const std::string& prefix) :
  VarSet<CDCSegmentPairTruthVarNames>(prefix)
{
}

bool CDCSegmentPairTruthVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  bool extracted = extractNested(ptrSegmentPair);
  if (not extracted or not ptrSegmentPair) return false;

  const CDCSegmentPair& segmentPair = *ptrSegmentPair;

  const CellWeight mcWeight = m_mcSegmentPairFilter(segmentPair);
  var<named("truth")>() =  not isNotACell(mcWeight);
  return true;
}

void CDCSegmentPairTruthVarSet::initialize()
{
  m_mcSegmentPairFilter.initialize();
}

void CDCSegmentPairTruthVarSet::terminate()
{
  m_mcSegmentPairFilter.terminate();
}
