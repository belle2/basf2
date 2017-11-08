/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/utils/RecoTrackUtil.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

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
      recoTrack.addCDCHit(cdcHit, sortingParameter, RightLeftInformation::c_left);
    } else if (rlInfo == ERightLeft::c_Right) {
      recoTrack.addCDCHit(cdcHit, sortingParameter, RightLeftInformation::c_right);
    } else if (rlInfo == ERightLeft::c_Invalid) {
      recoTrack.addCDCHit(cdcHit,
                          sortingParameter,
                          RightLeftInformation::c_invalidRightLeftInformation);
    } else {
      recoTrack.addCDCHit(cdcHit,
                          sortingParameter,
                          RightLeftInformation::c_undefinedRightLeftInformation);
    }
  }
}
