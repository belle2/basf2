/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/findlets/minimal/AxialStraightTrackCreator.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

AxialStraightTrackCreator::AxialStraightTrackCreator() = default;

std::string AxialStraightTrackCreator::getDescription()
{
  return "A findlet looking for straight tracks between IP and reconstructed ECL clusters.";
}

void AxialStraightTrackCreator::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "minEnergy"),
                                m_param_minEnergy,
                                "Parameter to define minimal threshold of ECL cluster energy.",
                                m_param_minEnergy);
  moduleParamList->addParameter(prefixed(prefix, "minNHits"),
                                m_param_minNHits,
                                "Parameter to define minimal threshold of track number of hits.",
                                m_param_minNHits);
  moduleParamList->addParameter(prefixed(prefix, "maxDistance"),
                                m_param_maxDistance,
                                "Parameter to define maximal threshold of hit distance to a line IP - ECL cluster.",
                                m_param_maxDistance);
}

void AxialStraightTrackCreator::initialize()
{
  Super::initialize();
}

void AxialStraightTrackCreator::apply(const std::vector<const ECLCluster*>& eclClusters,
                                      const std::vector<const CDCWireHit*>& axialWireHits,
                                      std::vector<CDCTrack>& tracks)
{
  for (const ECLCluster* cluster : eclClusters) {
    if (!cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;
    if (cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) < m_param_minEnergy) continue;
    float phi = cluster->getPhi();
    UncertainPerigeeCircle circle(0, Vector2D::Phi(phi), 0); //no covariance matrix (yet?)
    CDCTrajectory2D guidingTrajectory2D(circle);
    CDCTrack track;
    guidingTrajectory2D.setLocalOrigin(Vector2D(0, 0));
    std::vector<const CDCWireHit*> foundHits = search(axialWireHits, guidingTrajectory2D);
    if (foundHits.size() < m_param_minNHits) continue;
    // Fit trajectory
    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter(true, true);
    CDCTrajectory2D trajectory2D = fitter.fit(foundHits);
    track.setStartTrajectory3D(CDCTrajectory3D(trajectory2D, CDCTrajectorySZ::basicAssumption()));

    // Reconstruct and add hits
    for (const CDCWireHit* wireHit : foundHits) {
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstructNearest(wireHit, trajectory2D);
      track.push_back(std::move(recoHit3D));
    }
    track.sortByArcLength2D();
    tracks.emplace_back(std::move(track));
  }
}

std::vector<const CDCWireHit*> AxialStraightTrackCreator::search(const std::vector<const CDCWireHit*>& axialWireHits,
    const CDCTrajectory2D& guidingTrajectory2D)
{
  std::vector<const CDCWireHit*> foundHits;
  for (const CDCWireHit* hit : axialWireHits) {
    const Vector2D point = hit->reconstruct2D(guidingTrajectory2D);
    float arc = guidingTrajectory2D.calcArcLength2D(point);
    if (arc < 0) continue; // No b2b tracks
    float distance = guidingTrajectory2D.getDist2D(point);
    if (std::fabs(distance) < m_param_maxDistance) {
      foundHits.push_back(hit);
    }
  }
  return foundHits;
}
