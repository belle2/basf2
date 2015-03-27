/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CDCWireHitClusterBkgTruthVarSet.h"

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

/// Important to define the constexpr here such that it gains external linkage.
constexpr const char* const CDCWireHitClusterBkgTruthVarNames::names[nNames];

CDCWireHitClusterBkgTruthVarSet::CDCWireHitClusterBkgTruthVarSet(const std::string& prefix) :
  VarSet<CDCWireHitClusterBkgTruthVarNames>(prefix)
{
}

bool CDCWireHitClusterBkgTruthVarSet::extract(const CDCWireHitCluster* ptrCluster)
{
  bool extracted = extractNested(ptrCluster);
  if (not extracted or not ptrCluster) return false;
  const CDCWireHitCluster& cluster = *ptrCluster;

  int nBackgroundHits = 0;

  for (const CDCWireHit* wireHit : cluster) {
    const CDCHit* hit = wireHit->getHit();

    if (CDCMCHitLookUp::getInstance().isBackground(hit)) {
      ++nBackgroundHits;
    }
  }

  // Variables with Monte Carlo information should carry truth in their name.
  var<named("n_background_hits_truth")>() = nBackgroundHits;
  var<named("background_fraction_truth")>() = 1.0 * nBackgroundHits / cluster.size();
  var<named("weight")>() = cluster.size();
  return true;
}
