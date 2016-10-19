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
#include <TDatabasePDG.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>

#include <cassert>

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
    TVector3 truePos = middleSimHit->getPosTrack();
    TVector3 trueMom = middleSimHit->getMomentum();
    int pdgCode = middleSimHit->getPDGCode();
    const TParticlePDG* ptrTPDGParticle = TDatabasePDG::Instance()->GetParticle(pdgCode);
    if (not ptrTPDGParticle) {
      return false;
      B2WARNING("No particle for PDG code " << pdgCode << ". Could not get fit");
    }
    const TParticlePDG& tPDGParticle = *ptrTPDGParticle;
    double charge = tPDGParticle.Charge() / 3.0;
    double trueCurv  = CDCBFieldUtil::absMom2DToCurvature(trueMom.Perp(), charge, Vector3D(truePos));

    var<named("truth_pos_theta")>() = truePos.Theta();
    var<named("truth_mom_phi")>() = trueMom.Phi();
    var<named("truth_curv")>() = trueCurv;
    var<named("truth_alpha")>() = truePos.DeltaPhi(trueMom);
  } else {
    var<named("truth_pos_theta")>() = NAN;
    var<named("truth_mom_phi")>() = NAN;
    var<named("truth_curv")>() = NAN;
    var<named("truth_alpha")>() = NAN;
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
