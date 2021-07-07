/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/vtx/entities/CKFToVTXState.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vtx/dataobjects/VTXCluster.h>

#include <genfit/MeasuredStateOnPlane.h>

using namespace Belle2;

CKFToVTXState::CKFToVTXState(const RecoTrack* seed, bool reversed) : CKFState(seed)
{
  if (reversed) {
    setMeasuredStateOnPlane(seed->getMeasuredStateOnPlaneFromLastHit());
  } else {
    setMeasuredStateOnPlane(seed->getMeasuredStateOnPlaneFromFirstHit());
  }
  m_stateCache.isHitState = false;
  m_stateCache.phi = this->getMeasuredStateOnPlane().getPos().Phi();
  m_stateCache.theta = this->getMeasuredStateOnPlane().getPos().Theta();
  // get geometrical layer as virtual layer since we are coming from a CDC track from the outside
  m_stateCache.geoLayer = this->getGeometricalLayer();
}

unsigned int CKFToVTXState::getGeometricalLayer() const
{
  const SpacePoint* spacePoint = getHit();
  if (not spacePoint) {
    // If there is no SpacePoint, we are coming from a CDC track, so return a virtual outermost layer as nLayers + 1
    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
    return static_cast<int>(geoCache.getLayers(VXD::SensorInfoBase::SensorType::VTX).size()) + 1;
  }

  return spacePoint->getVxdID().getLayerNumber();
}

genfit::SharedPlanePtr CKFToVTXState::getPlane(const genfit::MeasuredStateOnPlane& state) const
{
  const VTXRecoHit& recoHit = getRecoHit();
  return recoHit.constructPlane(state);
}

const VTXRecoHit& CKFToVTXState::getRecoHit() const
{
  B2ASSERT("You are asking for the reco hit, although no hit is present.", not m_recoHits.empty());
  return m_recoHits.front();
}

const std::vector<VTXRecoHit>& CKFToVTXState::getRecoHits() const
{
  B2ASSERT("You are asking for the reco hit, although no hit is present.", not m_recoHits.empty());
  return m_recoHits;
}

CKFToVTXState::CKFToVTXState(const SpacePoint* hit) : CKFState<RecoTrack, SpacePoint>(hit)
{
  for (const VTXCluster& vtxCluster : hit->getRelationsTo<VTXCluster>()) {
    m_recoHits.emplace_back(&vtxCluster);
  }
  m_stateCache.isHitState = true;
  m_stateCache.sensorID = hit->getVxdID();
  m_stateCache.geoLayer = m_stateCache.sensorID.getLayerNumber();
  m_stateCache.ladder = m_stateCache.sensorID.getLadderNumber();
  const VXD::SensorInfoBase& sensorInfo = VXD::GeoCache::getInstance().getSensorInfo(hit->getVxdID());
  m_stateCache.sensorCenterPhi = sensorInfo.pointToGlobal(TVector3(0., 0., 0.), true).Phi();
  m_stateCache.phi = hit->getPosition().Phi();
  m_stateCache.theta = hit->getPosition().Theta();
  m_stateCache.localNormalizedu = hit->getNormalizedLocalU();
  m_stateCache.localNormalizedv = hit->getNormalizedLocalV();
}

const RecoTrack* CKFToVTXState::getRelatedVTXTrack() const
{
  return m_relatedVTXTrack;
}

void CKFToVTXState::setRelatedVTXTrack(const RecoTrack* relatedVTXTrack)
{
  if (m_relatedVTXTrack and m_relatedVTXTrack != relatedVTXTrack) {
    B2FATAL("You are resetting the related track to a different value!");
  }
  m_relatedVTXTrack = relatedVTXTrack;
}
