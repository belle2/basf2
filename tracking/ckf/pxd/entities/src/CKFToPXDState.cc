/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/pxd/entities/CKFToPXDState.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <genfit/MeasuredStateOnPlane.h>

using namespace Belle2;

CKFToPXDState::CKFToPXDState(const RecoTrack* seed) : CKFState(seed)
{
  setMeasuredStateOnPlane(seed->getMeasuredStateOnPlaneFromFirstHit());
}

unsigned int CKFToPXDState::getGeometricalLayer() const
{
  const SpacePoint* spacePoint = getHit();
  if (not spacePoint) {
    // return number of layer (2) + 1 -> we fake the 3th layer as the SVD
    return 3;
  }

  return spacePoint->getVxdID().getLayerNumber();
}

genfit::SharedPlanePtr CKFToPXDState::getPlane(const genfit::MeasuredStateOnPlane& state) const
{
  const PXDRecoHit& recoHit = getRecoHit();
  return recoHit.constructPlane(state);
}

const PXDRecoHit& CKFToPXDState::getRecoHit() const
{
  B2ASSERT("You are asking for the reco hit, although no hit is present.", not m_recoHits.empty());
  return m_recoHits.front();
}

const std::vector<PXDRecoHit>& CKFToPXDState::getRecoHits() const
{
  B2ASSERT("You are asking for the reco hit, although no hit is present.", not m_recoHits.empty());
  return m_recoHits;
}

CKFToPXDState::CKFToPXDState(const SpacePoint* hit) : CKFState<RecoTrack, SpacePoint>(hit)
{
  for (const PXDCluster& pxdCluster : hit->getRelationsTo<PXDCluster>()) {
    m_recoHits.emplace_back(&pxdCluster);
  }
}