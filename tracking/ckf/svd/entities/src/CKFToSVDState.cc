/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  m_stateCache.isHitState = false;
  m_stateCache.phi = this->getMeasuredStateOnPlane().getPos().Phi();
  m_stateCache.theta = this->getMeasuredStateOnPlane().getPos().Theta();
  m_stateCache.geoLayer = this->getGeometricalLayer();
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
  m_stateCache.isHitState = true;
  m_stateCache.sensorID = hit->getVxdID();
  m_stateCache.geoLayer = this->getGeometricalLayer();
  m_stateCache.ladder = m_stateCache.sensorID.getLadderNumber();
  const VXD::SensorInfoBase& sensorInfo = VXD::GeoCache::getInstance().getSensorInfo(hit->getVxdID());
  m_stateCache.sensorCenterPhi = sensorInfo.pointToGlobal(ROOT::Math::XYZVector(0., 0., 0.), true).Phi();
  m_stateCache.phi = hit->getPosition().Phi();
  m_stateCache.theta = hit->getPosition().Theta();
  m_stateCache.localNormalizedu = hit->getNormalizedLocalU();
  m_stateCache.localNormalizedv = hit->getNormalizedLocalV();
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
