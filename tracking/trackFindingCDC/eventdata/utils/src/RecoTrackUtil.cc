/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Dmitrii Neverov                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/utils/RecoTrackUtil.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBFieldUtil.h>

#include <tracking/trackFindingCDC/numerics/TMatrixConversion.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

#include <framework/datastore/StoreArray.h>

#include <TMatrixDSymfwd.h>
#include <TMatrixTSym.h>
#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

RecoTrack* RecoTrackUtil::storeInto(const CDCTrack& track, StoreArray<RecoTrack>& recoTracks, const double momentumSeedMagnitude)
{
  CDCTrack firstHits;
  ISuperLayer closestLayer = track.getStartISuperLayer();
  if (ISuperLayerUtil::isAxial(closestLayer)) closestLayer = ISuperLayerUtil::getNextOutwards(closestLayer);

  for (const CDCRecoHit3D hit : track) {
    if (hit.isAxial()) continue;
    firstHits.push_back(hit);
    if ((hit.getISuperLayer() != closestLayer) and (firstHits.size() > 3)) break;
  }

  const CDCSZFitter& szFitter = CDCSZFitter::getFitter();
  const CDCTrajectorySZ& szTrajectory = szFitter.fitWithStereoHits(firstHits);

  Vector3D position(track.getStartTrajectory3D().getSupport().x(),
                    track.getStartTrajectory3D().getSupport().y(),
                    szTrajectory.getZ0());

  Vector3D momentum(track.getStartTrajectory3D().getFlightDirection3DAtSupport());
  const double z0 = szTrajectory.getZ0();
  const double lambda = std::atan(szTrajectory.getTanLambda());
  momentum.scale(std::cos(lambda));
  momentum.setZ(std::sin(lambda));
  momentum *= momentumSeedMagnitude;

  ESign charge;
  double votes(0);
  int nHits(0);
  closestLayer = track.getEndISuperLayer();
  if (ISuperLayerUtil::isAxial(closestLayer)) closestLayer = ISuperLayerUtil::getNextInwards(closestLayer);
  for (auto it = track.rbegin(); it != track.rend(); it++) {
    CDCRecoHit3D hit = *it;
    if (hit.isAxial()) continue;
    votes += (hit.getRecoZ() - z0 - std::tan(lambda) * hit.getArcLength2D());
    nHits++;
    if ((hit.getISuperLayer() != closestLayer) and (nHits > 3)) break;
  }
  votes > 0 ? charge = ESign::c_Plus : charge = ESign::c_Minus;

  RecoTrack* newRecoTrack = recoTracks.appendNew(position, momentum, charge);
  RecoTrackUtil::fill(track, *newRecoTrack);
  return newRecoTrack;
}

RecoTrack* RecoTrackUtil::storeInto(const CDCTrack& track, StoreArray<RecoTrack>& recoTracks)
{
  const CDCTrajectory3D& traj3D = track.getStartTrajectory3D();
  RecoTrack* newRecoTrack = storeInto(traj3D, recoTracks);
  if (not newRecoTrack) return nullptr;

  RecoTrackUtil::fill(track, *newRecoTrack);
  return newRecoTrack;
}

RecoTrack*
RecoTrackUtil::storeInto(const CDCTrajectory3D& traj3D, StoreArray<RecoTrack>& recoTracks)
{
  Vector3D position = traj3D.getSupport();
  return storeInto(traj3D, CDCBFieldUtil::getBFieldZ(position), recoTracks);
}

RecoTrack* RecoTrackUtil::storeInto(const CDCTrajectory3D& traj3D,
                                    const double bZ,
                                    StoreArray<RecoTrack>& recoTracks)
{
  // Set the start parameters
  Vector3D position = traj3D.getSupport();
  Vector3D momentum =
    bZ == 0 ? traj3D.getFlightDirection3DAtSupport() : traj3D.getMom3DAtSupport(bZ);
  ESign charge = traj3D.getChargeSign();

  // Do not propagate invalid fits, signal that the fit is invalid to the caller.
  if (not ESignUtil::isValid(charge) or momentum.hasNAN() or position.hasNAN()) {
    return nullptr;
  }

  RecoTrack* newRecoTrack = recoTracks.appendNew(position, momentum, charge);
  if (std::isfinite(traj3D.getFlightTime())) {
    newRecoTrack->setTimeSeed(traj3D.getFlightTime());
  }

  const CovarianceMatrix<6>& cov6 = traj3D.getCartesianCovariance(bZ);
  TMatrixDSym covSeed = TMatrixConversion::toTMatrix(cov6);
  newRecoTrack->setSeedCovariance(covSeed);
  return newRecoTrack;
}

/// List of explicit specialisations - at the moment only CDCTrack.
template void RecoTrackUtil::fill<CDCTrack>(const CDCTrack&, RecoTrack&);

/// fill() specialisation for a class that holds right-left wire-hit information
template <class ARLHitHolderRange>
void RecoTrackUtil::fill(const ARLHitHolderRange& rlWireHitHolders, RecoTrack& recoTrack)
{
  int sortingParameter = -1;
  for (const auto& rlWireHitHolder : rlWireHitHolders) {
    ++sortingParameter;

    const CDCRLWireHit rlWireHit = rlWireHitHolder.getRLWireHit();
    const CDCWireHit& wireHit = rlWireHit.getWireHit();
    const CDCHit* cdcHit = wireHit.getHit();

    // Right left ambiguity resolution
    ERightLeft rlInfo = rlWireHit.getRLInfo();
    using RightLeftInformation = RecoHitInformation::RightLeftInformation;
    if (rlInfo == ERightLeft::c_Left) {
      recoTrack.addCDCHit(cdcHit,
                          sortingParameter,
                          RightLeftInformation::c_left,
                          RecoHitInformation::c_CDCTrackFinder);
    } else if (rlInfo == ERightLeft::c_Right) {
      recoTrack.addCDCHit(cdcHit,
                          sortingParameter,
                          RightLeftInformation::c_right,
                          RecoHitInformation::c_CDCTrackFinder);
    } else if (rlInfo == ERightLeft::c_Invalid) {
      recoTrack.addCDCHit(cdcHit,
                          sortingParameter,
                          RightLeftInformation::c_invalidRightLeftInformation,
                          RecoHitInformation::c_CDCTrackFinder);
    } else {
      recoTrack.addCDCHit(cdcHit,
                          sortingParameter,
                          RightLeftInformation::c_undefinedRightLeftInformation,
                          RecoHitInformation::c_CDCTrackFinder);
    }
  }
}
