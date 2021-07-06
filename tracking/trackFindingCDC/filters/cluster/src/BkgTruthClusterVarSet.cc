/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/cluster/BkgTruthClusterVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void BkgTruthClusterVarSet::initialize()
{
  Super::initialize();
  CDCMCManager::getInstance().requireTruthInformation();
}

void BkgTruthClusterVarSet::beginEvent()
{
  Super::beginEvent();
  CDCMCManager::getInstance().fill();
}

bool BkgTruthClusterVarSet::extract(const CDCWireHitCluster* ptrCluster)
{
  if (not ptrCluster) return false;
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
  var<named("truth")>() = (1.0 * nBackgroundHits / cluster.size() > 0.8) ? 0 : 1;
  return true;
}
