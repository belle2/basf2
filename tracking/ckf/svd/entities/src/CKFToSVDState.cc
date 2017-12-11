/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/svd/entities/CKFToSVDState.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <svd/dataobjects/SVDCluster.h>

#include <genfit/MeasuredStateOnPlane.h>

using namespace Belle2;

CKFToSVDState::CKFToSVDState(const RecoTrack* seed, bool reversed) : CKFState(seed)
{
  if (reversed) {
    setMeasuredStateOnPlane(seed->getMeasuredStateOnPlaneFromLastHit());
  } else {
    setMeasuredStateOnPlane(seed->getMeasuredStateOnPlaneFromFirstHit());
  }
}

unsigned int CKFToSVDState::getGeometricalLayer() const
{
  const SpacePoint* spacePoint = getHit();
  if (not spacePoint) {
    // return number of layer (6) + 1 -> we fake the 7th layer as the CDC
    return 7;
  }

  return spacePoint->getVxdID().getLayerNumber();
}

genfit::SharedPlanePtr CKFToSVDState::getPlane(const genfit::MeasuredStateOnPlane& state) const
{
  const SVDRecoHit& recoHit = getRecoHit();
  return recoHit.constructPlane(state);
}

const SVDRecoHit& CKFToSVDState::getRecoHit() const
{
  B2ASSERT("You are asking for the reco hit, although no hit is present.", not m_recoHits.empty());
  return m_recoHits.front();
}

const std::vector<SVDRecoHit>& CKFToSVDState::getRecoHits() const
{
  B2ASSERT("You are asking for reco hits, although no hit is present.", not m_recoHits.empty());
  return m_recoHits;
}

CKFToSVDState::CKFToSVDState(const SpacePoint* hit) : CKFState<RecoTrack, SpacePoint>(hit)
{
  m_recoHits.reserve(2);
  for (const SVDCluster& svdCluster : hit->getRelationsTo<SVDCluster>()) {
    m_recoHits.emplace_back(&svdCluster);
  }
}

const RecoTrack* CKFToSVDState::getRelatedSVDTrack() const
{
  return m_relatedSVDTrack;
}

void CKFToSVDState::setRelatedSVDTrack(const RecoTrack* relatedSVDTrack)
{
  if (m_relatedSVDTrack and m_relatedSVDTrack != relatedSVDTrack) {
    B2FATAL("You are resetting the related track to a different value!");
  }
  m_relatedSVDTrack = relatedSVDTrack;
}