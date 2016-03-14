/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.h>

#include <tracking/trackFindingCDC/hough/perigee/StereoHitContained.h>
#include <tracking/trackFindingCDC/hough/perigee/OffOrigin.h>
#include <tracking/trackFindingCDC/hough/perigee/InPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/WithSharedMark.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>

template<class Node>
void Belle2::TrackFindingCDC::AxialLegendreLeafProcessor<Node>::processLeaf(Node* leaf)
{
  // No special post processing version
  // const Phi0CurvBox& phi0CurvBox = *leaf;
  // for (WithSharedMark<CDCRLTaggedWireHit>& markableRLTaggedWireHit : *leaf) {
  //   markableRLTaggedWireHit.mark();
  // }
  // m_candidates.emplace_back(CDCTrajectory2D(PerigeeCircle(phi0CurvBox.getLowerCurv(),
  //                phi0CurvBox.getLowerPhi0Vec(),
  //                0)),
  //          std::vector<CDCRLTaggedWireHit>(leaf->begin(),
  //                  leaf->end()));
  // return;



  // start off by fitting the items of the leaf with a general trajectory
  // that may have a distinct d0 != 0
  /////////////////////////////////////////////////////////////////////////
  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
  CDCTrajectory2D trajectory2D = fitter.fit(*leaf);
  GeneralizedCircle circle = trajectory2D.getGlobalCircle();
  {
    const double curv = circle.curvature();
    std::array<DiscreteCurv, 2> curvs = leaf->template getBounds<DiscreteCurv>();

    const float& lowerCurv = *(curvs[0]);
    const float& upperCurv = *(curvs[1]);
    if (SameSignChecker::commonSign(lowerCurv, upperCurv) * curv < 0) {
      circle.reverse();
    }
  }

  const double curv = circle.curvature();
  // const Vector2D& phi0Vec = circle.tangential();
  const double phi0 = circle.tangentialPhi();
  // const double impact = circle.impact();
  const Vector2D& perigee = circle.perigee();

  // Next we reject high momentum tracks
  // TODO: To be cross checked
  // if (not increaseThreshold) {
  //   if (TrackCandidate::convertRhoToPt(fabs(qt->getYMean())) > 0.7 && (D > 0.4)) {
  //     for (TrackHit* hit : qt->getItemsVector()) {
  //  hit->setHitUsage(TrackHit::c_bad);
  //     }
  //     return;
  //   }
  // }


  // Look for more hits near the found trajectory in all available hits
  /////////////////////////////////////////////////////////////////////////

  const float curlCurv = 0.018;
  StereoHitContained<OffOrigin<InPhi0CurvBox> > hitInPhi0CurvBox(curlCurv);
  using HoughBox = StereoHitContained<OffOrigin<InPhi0CurvBox> >::HoughBox;
  hitInPhi0CurvBox.setLocalOrigin(perigee);

  // Determine a precision that we expect to achieve at the fitted momentum
  double levelPrecision = 10.5 - 0.24 * exp(-4.13118 * TrackCandidate::convertRhoToPt(curv) + 2.74);
  double curvPrecision = 0.15 / (pow(2., levelPrecision));
  double phi0Precision = PI / (pow(2., levelPrecision + 1));

  // Make a hough space box with the determined precision
  DiscreteCurv::Array curvBounds = linspace<float>(curv - curvPrecision, curv + curvPrecision, 2);
  DiscretePhi0::Array phi0Bounds = linspace<Vector2D>(phi0 - phi0Precision, phi0 + phi0Precision, 2,
                                                      &(Vector2D::Phi));

  HoughBox precisionPhi0CurvBox(DiscretePhi0::getRange(phi0Bounds),
                                DiscreteCurv::getRange(curvBounds));

  // Acquire all available items
  Node& topNode = leaf->getTree()->getTopNode();
  // Make sure that all items we have used somewhere else are removed from the available items.
  auto isMarked = [](const WithSharedMark<CDCRLTaggedWireHit>& item) -> bool {
    return item.isMarked();
  };
  topNode.eraseIf(isMarked);

  // Collect all hits that are in the precision box
  std::vector<WithSharedMark<CDCRLTaggedWireHit> > hitsInPrecisionBox;

  for (const WithSharedMark<CDCRLTaggedWireHit>& markableRLTaggedWireHit : topNode) {
    // Explicitly making a copy here to emphasise that we do not change the top node.
    WithSharedMark<CDCRLTaggedWireHit> copiedMarkableRLTaggedWireHit = markableRLTaggedWireHit;

    Weight weight = hitInPhi0CurvBox(copiedMarkableRLTaggedWireHit, &precisionPhi0CurvBox);
    if (not std::isnan(weight)) {
      hitsInPrecisionBox.push_back(copiedMarkableRLTaggedWireHit);
    }
  }

  // Fit again and see if the fit improved with the new added hits.
  ///////////////////////////////////////////////////////////////////
  CDCTrajectory2D precisionTrajectory2D = fitter.fit(hitsInPrecisionBox);

  for (WithSharedMark<CDCRLTaggedWireHit>& markableRLTaggedWireHit : hitsInPrecisionBox) {
    markableRLTaggedWireHit.mark();
  }
  m_candidates.emplace_back(precisionTrajectory2D,
                            std::vector<CDCRLTaggedWireHit>(hitsInPrecisionBox.begin(),
                                                            hitsInPrecisionBox.end()));
}
