/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/TruthFacetVarSet.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TruthFacetVarSet::TruthFacetVarSet()
  : Super()
{
}

bool TruthFacetVarSet::extract(const CDCFacet* ptrFacet)
{
  bool extracted = extractNested(ptrFacet);
  if (not extracted or not ptrFacet) return false;
  const CDCFacet& facet = *ptrFacet;

  const CDCRLWireHitTriple& rlWireHitTriple = facet;

  const Weight mcWeight = m_mcFacetFilter(facet);
  var<named("truth")>() =  not std::isnan(mcWeight);

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  const CDCWireHit& middleWireHit = rlWireHitTriple.getMiddleWireHit();

  const CDCSimHit* middleSimHit = mcHitLookUp.getSimHit(middleWireHit.getHit());
  if (middleSimHit) {
    var<named("truth_pos_theta")>() = middleSimHit->getPosTrack().Theta();
    var<named("truth_mom_phi")>() = middleSimHit->getMomentum().Phi();
  } else {
    var<named("truth_pos_theta")>() = NAN;
    var<named("truth_mom_phi")>() = NAN;
  }

  return true;
}

void TruthFacetVarSet::initialize()
{
  m_mcFacetFilter.initialize();
}

void TruthFacetVarSet::terminate()
{
  m_mcFacetFilter.terminate();
}
