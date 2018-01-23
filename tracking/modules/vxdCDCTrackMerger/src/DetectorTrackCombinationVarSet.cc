/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Malwin Weiler, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/vxdCDCTrackMerger/DetectorTrackCombinationVarSet.h>
#include <tracking/trackFindingCDC/numerics/ToFinite.h>

#include <TMath.h>

#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool DetectorTrackCombinationVarSet::extract(const BaseDetectorTrackCombinationFilter::Object* pair)
{
  // TODO: Make parameter
  const double cdcRadius = 16.25;

  const RecoTrack* collectorItem = *(pair->getFrom());
  const RecoTrack* collectionItem = *(pair->getTo());

  if (not pair or not collectorItem or not collectionItem) return false;

  const auto& vxdPosition = collectionItem->getPositionSeed();
  const auto& vxdMomentum = collectionItem->getMomentumSeed();
  const auto vxdCharge = collectionItem->getChargeSeed();

  const auto& cdcPosition = collectorItem->getPositionSeed();
  const auto& cdcMomentum = collectorItem->getMomentumSeed();
  const auto cdcCharge = collectorItem->getChargeSeed();

  const double bField = BFieldManager::getField(cdcPosition).Z() / Unit::T;

  Belle2::Helix VXDHelix(vxdPosition, vxdMomentum, vxdCharge, bField);
  Belle2::Helix CDCHelix(cdcPosition, cdcMomentum, cdcCharge, bField);

  const double phiCDC = CDCHelix.getMomentum(bField).Phi();
  const double thetaCDC = CDCHelix.getMomentum(bField).Theta();
  const double pCDC = CDCHelix.getTransverseMomentum(bField);

  const double phiVXD = VXDHelix.getMomentum(bField).Phi();
  const double thetaVXD = VXDHelix.getMomentum(bField).Theta();
  const double pVXD = VXDHelix.getTransverseMomentum(bField);

  //Absolute and relative theta
  const double thetaAbs = fabs(thetaCDC - thetaVXD);
  const double thetaRel = fabs(thetaCDC - thetaVXD) / thetaVXD;

  //Absolute and relative phi
  const double phiAbs = fmod(fabs(phiCDC - phiVXD), 2 * TMath::Pi());
  const double phiRel = fabs(phiCDC - phiVXD) / phiVXD;

  //absolute and relative transverse momentum
  const double pAbs = fabs(pCDC - pVXD);
  const double pRel = fabs(pCDC - pVXD) / pVXD;

  //charge of both tracks
  const int chargeVXD = collectionItem->getChargeSeed();
  const int chargeCDC = collectorItem->getChargeSeed();

  const int chargeDif = chargeVXD - chargeCDC;

  //number of hits is the detectors by both tracks
  const unsigned int numberHitsCDC = collectorItem->getNumberOfCDCHits();
  const unsigned int numberHitsVXD = collectionItem->getNumberOfSVDHits() + collectionItem->getNumberOfPXDHits();

  //distance between the two tracks on the detector interface
  const TVector3& posCDC = CDCHelix.getPositionAtArcLength2D(CDCHelix.getArcLength2DAtCylindricalR(cdcRadius));
  const TVector3& posVXD = VXDHelix.getPositionAtArcLength2D(VXDHelix.getArcLength2DAtCylindricalR(cdcRadius));
  const double distance = (posCDC - posVXD).Perp();

  const double vertex = (posCDC - posVXD).Mag();
  const double vertexCDC = posCDC.Mag();
  const double vertexVXD = posVXD.Mag();

  // Short cut:
  if (distance >= 3 or (phiAbs > 0.3 and thetaAbs > 0.3)) {
    return false;
  }

  // Do not use curlers, as they are to hard to decide properly (because of clones etc.)
  if (pCDC < 0.3) {
    return false;
  }

  var<named("phiCDC")>() = toFinite(phiCDC, 0);
  var<named("thetaCDC")>() = toFinite(thetaCDC, 0);
  var<named("pCDC")>() = toFinite(pCDC, 0);
  var<named("phiVXD")>() = toFinite(phiVXD, 0);
  var<named("thetaVXD")>() = toFinite(thetaVXD, 0);
  var<named("pVXD")>() = toFinite(pVXD, 0);
  var<named("phiAbs")>() = toFinite(phiAbs, 0);
  var<named("phiRel")>() = toFinite(phiRel, 0);
  var<named("thetaAbs")>() = toFinite(thetaAbs, 0);
  var<named("thetaRel")>() = toFinite(thetaRel, 0);
  var<named("pAbs")>() = toFinite(pAbs, 0);
  var<named("pRel")>() = toFinite(pRel, 0);
  var<named("chargeVXD")>() = toFinite(chargeVXD, 0);
  var<named("chargeCDC")>() = toFinite(chargeCDC, 0);
  var<named("numberHitsCDC")>() = toFinite(numberHitsCDC, 0);
  var<named("numberHitsVXD")>() = toFinite(numberHitsVXD, 0);
  var<named("distance")>() = toFinite(distance, 0);
  var<named("vertex")>() = toFinite(vertex, 0);
  var<named("vertexVXD")>() = toFinite(vertexVXD, 0);
  var<named("vertexCDC")>() = toFinite(vertexCDC, 0);
  var<named("chargeDif")>() = toFinite(chargeDif, 0);

  return true;
}
