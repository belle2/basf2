/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/CDCSegmentPairTruthVarSet.h>
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCSegmentPairTruthVarSet::CDCSegmentPairTruthVarSet(const std::string& prefix) :
  FilterVarSet<MCSegmentPairFilter>(prefix + "truth_",
                                    std::unique_ptr<MCSegmentPairFilter>(new MCSegmentPairFilter()))
{
}
