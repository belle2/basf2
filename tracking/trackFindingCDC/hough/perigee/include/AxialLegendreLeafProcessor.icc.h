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
#include <tracking/trackFindingCDC/hough/algorithms/InPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/baseelements/WithSharedMark.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>
#include <tracking/trackFindingCDC/numerics/LookupTable.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/BasePrecisionFunction.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class ANode>
    void AxialLegendreLeafProcessor<ANode>::processLeaf(ANode* leaf)
    {
      // No special post processing version
      // std::array<DiscreteCurv, 2> curvs = leaf->template getBounds<DiscreteCurv>();
      // float lowerCurv = *(curvs[0]);
      // float upperCurv = *(curvs[1]);

      // std::array<DiscretePhi0, 2> phi0Vecs = leaf->template getBounds<DiscretePhi0>();
      // Vector2D lowerPhi0Vec = *(phi0Vecs[0]);
      // Vector2D upperPhi0Vec = *(phi0Vecs[1]);
      // for (WithSharedMark<CDCRLWireHit>& markableRLTaggedWireHit : *leaf) {
      //   markableRLTaggedWireHit.mark();
      // }
      // m_candidates.emplace_back(CDCTrajectory2D(PerigeeCircle(lowerCurv, lowerPhi0Vec, 0)),
      //                           std::vector<CDCRLWireHit>(leaf->begin(), leaf->end()));
      // return;

      // Special post processing looking for more hits
      // Start off by fitting the items of the leaf with a general trajectory
      // that may have a distinct impact != 0
      /////////////////////////////////////////////////////////////////////////
      const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();

      std::vector<WithSharedMark<CDCRLWireHit>> hits(leaf->begin(), leaf->end());
      CDCTrajectory2D trajectory2D = fitter.fit(hits);
      {
        const double curv = trajectory2D.getCurvature();
        std::array<DiscreteCurv, 2> curvs = leaf->template getBounds<DiscreteCurv>();

        float lowerCurv = *(curvs[0]);
        float upperCurv = *(curvs[1]);
        if (ESignUtil::common(lowerCurv, upperCurv) * curv < 0) {
          trajectory2D.reverse();
        }
      }
      trajectory2D.setLocalOrigin(Vector2D(0, 0));

      // Look for more hits near the found trajectory
      /////////////////////////////////////////////////////////////////////////
      if (m_nRoadSearches > 0) {
        // Acquire all available items in some parent node for the road search
        // Somewhat bypasses the logic of the tree walk - Must handle with care!
        ANode* roadNode = leaf;
        while (roadNode->getParent() and roadNode->getLevel() > m_roadLevel) {
          roadNode = roadNode->getParent();
        }

        // Make sure that all hits we have used somewhere else are removed from the available hits.
        auto isMarked = [](const WithSharedMark<CDCRLWireHit>& item) -> bool {
          return item.isMarked();
        };
        roadNode->eraseIf(isMarked);

        for (int iRoadSearch = 0; iRoadSearch < m_nRoadSearches; ++iRoadSearch) {
          hits = this->searchRoad(*roadNode, trajectory2D);
          trajectory2D = fitter.fit(hits);
          trajectory2D.setLocalOrigin(Vector2D(0.0, 0.0));
        }
      }

      // Mark found hit as used and safe them with the trajectory
      /////////////////////////////////////////////////////////////////////////
      for (WithSharedMark<CDCRLWireHit>& markableRLWireHit : hits) {
        markableRLWireHit.mark();
      }
      m_candidates.emplace_back(trajectory2D, std::vector<CDCRLWireHit>(hits.begin(), hits.end()));
    }

    template <class ANode>
    std::vector<WithSharedMark<CDCRLWireHit>>
                                           AxialLegendreLeafProcessor<ANode>::searchRoad(const ANode& node, const CDCTrajectory2D& trajectory2D)
    {
      PerigeeCircle circle = trajectory2D.getGlobalCircle();
      Vector2D support = trajectory2D.getGlobalPerigee();
      const float curv = circle.curvature();
      const float phi0 = circle.phi0();

      StereoHitContained<OffOrigin<InPhi0CurvBox> > hitInPhi0CurvBox(m_curlCurv);
      using HoughBox = StereoHitContained<OffOrigin<InPhi0CurvBox> >::HoughBox;
      hitInPhi0CurvBox.setLocalOrigin(support);

      // Determine a precision that we expect to achieve at the fitted momentum
      const float levelPrecision = 9.0;
      const float phi0Precision = 3.1415 / std::pow(2.0, levelPrecision + 1.0);
      const float curvPrecision = 0.15 / std::pow(2.0, levelPrecision);

      // Earlier version
      // BasePrecisionFunction funct;
      // float levelPrecision = 10.5 - 0.24 * exp(-4.13118 * funct.convertRhoToPt(curv) + 2.74);
      // float curvPrecision = 0.15 / (pow(2., levelPrecision));
      // float phi0Precision = M_PI / (pow(2., levelPrecision + 1));

      DiscreteCurv::Array curvBounds{{curv - curvPrecision, curv + curvPrecision}};
      DiscretePhi0::Array phi0Bounds{{Vector2D::Phi(phi0 - phi0Precision), Vector2D::Phi(phi0 + phi0Precision)}};
      HoughBox precisionPhi0CurvBox(DiscretePhi0::getRange(phi0Bounds), DiscreteCurv::getRange(curvBounds));

      // HoughBox precisionPhi0CurvBox = *leaf;
      std::vector<WithSharedMark<CDCRLWireHit>> hitsInPrecisionBox;

      // Explicitly making a copy here to ensure that we do not change the node content
      for (WithSharedMark<CDCRLWireHit> markableRLWireHit : node) {
        // Remove marked hits - redunadent since we removed them above
        if (markableRLWireHit.isMarked()) continue;
        Weight weight = hitInPhi0CurvBox(markableRLWireHit, &precisionPhi0CurvBox);
        if (not std::isnan(weight)) hitsInPrecisionBox.push_back(markableRLWireHit);
      }
      return hitsInPrecisionBox;
    }
  }
}
