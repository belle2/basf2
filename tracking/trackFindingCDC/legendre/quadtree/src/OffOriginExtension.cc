/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/legendre/quadtree/OffOriginExtension.h>

#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/numerics/LookupTable.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

OffOriginExtension::OffOriginExtension(std::vector<const CDCWireHit*> allAxialWireHits,
                                       double levelPrecision)
  : BaseCandidateReceiver(std::move(allAxialWireHits))
  , m_levelPrecision(levelPrecision)
{
}

void OffOriginExtension::operator()(const std::vector<const CDCWireHit*>& inputWireHits,
                                    void* qt __attribute__((unused)))
{
  // Unset the taken flag and let the postprocessing decide
  for (const CDCWireHit* wireHit : inputWireHits) {
    (*wireHit)->setTakenFlag(false);
  }

  std::vector<const CDCWireHit*> candidateHits = roadSearch(inputWireHits);
  AxialTrackUtil::addCandidateFromHits(candidateHits,
                                       m_allAxialWireHits,
                                       m_tracks,
                                       true);
}

std::vector<const CDCWireHit*>
OffOriginExtension::roadSearch(const std::vector<const CDCWireHit*>& wireHits)
{
  const CDCKarimakiFitter& fitter = CDCKarimakiFitter::getNoDriftVarianceFitter();
  CDCTrajectory2D trackTrajectory2D = fitter.fit(wireHits);

  double chi2 = trackTrajectory2D.getChi2();
  Vector2D refPos = trackTrajectory2D.getGlobalPerigee();

  // change sign of the curvature; should be the same as the charge of the candidate
  double curv = trackTrajectory2D.getCurvature();

  // theta is the vector normal to the trajectory at the perigee
  double theta = trackTrajectory2D.getGlobalCircle().phi0() + M_PI_2;

  // Hide the current hits from the road search
  for (const CDCWireHit* hit : wireHits) {
    hit->getAutomatonCell().setTakenFlag(true);
  }

  std::vector<const CDCWireHit*> newWireHits = getHitsWRTtoRefPos(refPos, curv, theta);

  // Unhide the current hits from the road search
  for (const CDCWireHit* hit : wireHits) {
    hit->getAutomatonCell().setTakenFlag(false);
  }

  if (newWireHits.size() == 0) return wireHits;

  std::vector<const CDCWireHit*> combinedWireHits;

  for (const CDCWireHit* hit : wireHits) {
    combinedWireHits.push_back(hit);
  }

  for (const CDCWireHit* hit : newWireHits) {
    combinedWireHits.push_back(hit);
  }

  CDCTrajectory2D combinedTrajectory2D = fitter.fit(wireHits);
  double combinedChi2 = combinedTrajectory2D.getChi2();

  if (combinedChi2 < chi2 * 2.) {
    return combinedWireHits;
  }

  return wireHits;
}

std::vector<const CDCWireHit*>
OffOriginExtension::getHitsWRTtoRefPos(const Vector2D& refPos, float curv, float theta)
{
  float thetaPrecision = 3.1415 / (pow(2., m_levelPrecision + 1));
  float curvPrecision = 0.15 / (pow(2., m_levelPrecision));

  using YSpan = AxialHitQuadTreeProcessor::YSpan;
  YSpan curvSpan{curv - curvPrecision, curv + curvPrecision};
  LookupTable<Vector2D> thetaSpan(&Vector2D::Phi, 1, theta - thetaPrecision, theta + thetaPrecision);

  AxialHitQuadTreeProcessor qtProcessor(refPos, curvSpan, &thetaSpan);
  qtProcessor.seed(m_allAxialWireHits);

  std::vector<const CDCWireHit*> newWireHits = qtProcessor.getAssignedItems();
  return newWireHits;
}
