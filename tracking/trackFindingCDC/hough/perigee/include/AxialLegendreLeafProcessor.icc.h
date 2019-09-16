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

#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/hough/perigee/StereoHitContained.h>
#include <tracking/trackFindingCDC/hough/perigee/OffOrigin.h>
#include <tracking/trackFindingCDC/hough/algorithms/InPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/algorithms/InPhi0ImpactCurvBox.h>
#include <tracking/trackFindingCDC/hough/baseelements/WithSharedMark.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>

#include <tracking/trackFindingCDC/legendre/precisionFunctions/PrecisionUtil.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class ANode>
    void AxialLegendreLeafProcessor<ANode>::processLeaf(ANode* leaf)
    {
      // Special post processing looking for more hits
      // Start off by fitting the items of the leaf with a general trajectory
      // that may have a distinct impact != 0
      /////////////////////////////////////////////////////////////////////////
      auto fitPos = EFitPos::c_RecoPos;
      auto fitVariance = EFitVariance::c_DriftLength;
      // Other combinations of fit information
      // EFitPos::c_RLDriftCircle x EFitVariance::(c_Nominal, c_Pseudo, c_Proper)
      // have been tried, but found to be worse, which is
      // not intutive. Probably the perfect circle trajectory
      // is not as good of a model on the full CDC volume.
      CDCObservations2D observations2D(fitPos, fitVariance);

      CDCKarimakiFitter fitter;
      // Tested alternative CDCRiemannFitter with only marginal differences;

      std::vector<WithSharedMark<CDCRLWireHit>> hits(leaf->begin(), leaf->end());
      std::sort(hits.begin(), hits.end()); // Hits should be naturally sorted
      observations2D.appendRange(hits);
      CDCTrajectory2D trajectory2D = fitter.fit(observations2D);
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
      if (m_param_nRoadSearches > 0) {
        // Acquire all available items in some parent node for the road search
        // Somewhat bypasses the logic of the tree walk - Must handle with care!
        ANode* roadNode = leaf;
        while (roadNode->getParent() and roadNode->getLevel() > m_param_roadLevel) {
          roadNode = roadNode->getParent();
        }

        for (int iRoadSearch = 0; iRoadSearch < m_param_nRoadSearches; ++iRoadSearch) {
          // Use a road search to find new hits from the trajectory
          // hits = this->searchRoad(*roadNode, trajectory2D); // In case you only want the road hits
          // if (hits.size() < 5) return;

          // Second version always holding on to the originally found hits
          int nHitsBefore = hits.size();
          std::vector<WithSharedMark<CDCRLWireHit>> roadHits = this->searchRoad(*roadNode, trajectory2D);
          std::sort(roadHits.begin(), roadHits.end());
          hits.insert(hits.end(), roadHits.begin(), roadHits.end());
          std::inplace_merge(hits.begin(), hits.begin() + nHitsBefore, hits.end());
          hits.erase(std::unique(hits.begin(), hits.end()), hits.end());

          // Update the current fit
          observations2D.clear();
          observations2D.appendRange(hits);
          trajectory2D = fitter.fit(observations2D);
          trajectory2D.setLocalOrigin(Vector2D(0.0, 0.0));
        }
      }

      // Mark found hit as used and safe them with the trajectory
      /////////////////////////////////////////////////////////////////////////
      std::vector<const CDCWireHit*> foundWireHits;
      for (CDCRLWireHit& rlWireHit : hits) {
        foundWireHits.push_back(&rlWireHit.getWireHit());
      }

      AxialTrackUtil::addCandidateFromHits(foundWireHits, m_axialWireHits, m_tracks, true);

      // Sync up the marks with the used hits
      for (WithSharedMark<CDCRLWireHit>& markableRLWireHit : leaf->getTree()->getTopNode()) {
        const AutomatonCell& automatonCell = markableRLWireHit.getWireHit().getAutomatonCell();
        if (automatonCell.hasTakenFlag() or automatonCell.hasMaskedFlag()) {
          markableRLWireHit.mark();
        } else {
          markableRLWireHit.unmark();
        }
      }
    }

    template <class ANode>
    std::vector<WithSharedMark<CDCRLWireHit> >
    AxialLegendreLeafProcessor<ANode>::searchRoad(const ANode& node, const CDCTrajectory2D& trajectory2D)
    {
      PerigeeCircle circle = trajectory2D.getGlobalCircle();
      Vector2D support = trajectory2D.getGlobalPerigee();
      const float curv = circle.curvature();
      const float phi0 = circle.phi0();

      StereoHitContained<OffOrigin<InPhi0ImpactCurvBox> > hitInPhi0CurvBox(m_param_curlCurv);
      hitInPhi0CurvBox.setLocalOrigin(support);
      using RoadHoughBox = StereoHitContained<OffOrigin<InPhi0ImpactCurvBox> >::HoughBox;

      // Determine a precision that we expect to achieve at the fitted momentum
      // There certainly is some optimizsation potential here.
      // Spread in the impact parameter is made available here but is not activated yet.
      const float levelPrecision = 9.0;
      // Earlier version
      // const float levelPrecision = 10.5 - 0.24 * exp(-4.13118 * PrecisionUtil::convertRhoToPt(curv) + 2.74);
      const float phi0Precision = 3.1415 / std::pow(2.0, levelPrecision + 1.0);
      const float impactPrecision = 0.0 * std::sqrt(CDCWireHit::c_simpleDriftLengthVariance);
      const float curvPrecision = 0.15 / std::pow(2.0, levelPrecision);

      DiscreteCurv::Array curvBounds{{curv - curvPrecision, curv + curvPrecision}};
      ContinuousImpact::Array impactBounds{{ -impactPrecision, impactPrecision}};
      DiscretePhi0::Array phi0Bounds{{Vector2D::Phi(phi0 - phi0Precision), Vector2D::Phi(phi0 + phi0Precision)}};
      RoadHoughBox precisionPhi0CurvBox(DiscretePhi0::getRange(phi0Bounds),
                                        ContinuousImpact::getRange(impactBounds),
                                        DiscreteCurv::getRange(curvBounds));

      std::vector<WithSharedMark<CDCRLWireHit>> hitsInPrecisionBox;

      // Explicitly making a copy here to ensure that we do not change the node content
      for (WithSharedMark<CDCRLWireHit> markableRLWireHit : node) {
        // Skip marked hits
        if (markableRLWireHit.isMarked()) continue;
        Weight weight = hitInPhi0CurvBox(markableRLWireHit, &precisionPhi0CurvBox);
        if (not std::isnan(weight)) hitsInPrecisionBox.push_back(markableRLWireHit);
      }

      return hitsInPrecisionBox;
    }
  }
}

namespace Belle2 {
  namespace TrackFindingCDC {
    template <class ANode>
    std::vector<typename AxialLegendreLeafProcessor<ANode>::Candidate>
    AxialLegendreLeafProcessor<ANode>::getCandidates() const
    {
      std::vector<Candidate> result;
      for (const CDCTrack& track : m_tracks) {
        std::vector<CDCRLWireHit> rlWireHits;
        for (const CDCRecoHit3D& recoHit3D : track) {
          rlWireHits.push_back(recoHit3D.getRLWireHit());
        }
        CDCTrajectory2D trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
        result.emplace_back(std::move(trajectory2D), std::move(rlWireHits));
      }
      return result;
    }

    template <class ANode>
    void AxialLegendreLeafProcessor<ANode>::exposeParameters(ModuleParamList* moduleParamList,
                                                             const std::string& prefix)
    {
      moduleParamList->addParameter(prefixed(prefix, "maxLevel"),
                                    m_param_maxLevel,
                                    "Level of divisions in the hough space at which a leaf is reached",
                                    m_param_maxLevel);

      moduleParamList->addParameter(prefixed(prefix, "minWeight"),
                                    m_param_minWeight,
                                    "Minimal exceptable weight of a leaf to be considered",
                                    m_param_minWeight);

      moduleParamList->addParameter(prefixed(prefix, "maxCurv"),
                                    m_param_maxCurv,
                                    "Maximal curvature of a leaf to be considered",
                                    m_param_maxCurv);

      moduleParamList->addParameter(prefixed(prefix, "curlCurv"),
                                    m_param_curlCurv,
                                    "Curvature below which hits on both arms of the trajectory are allowed",
                                    m_param_curlCurv);

      moduleParamList->addParameter(prefixed(prefix, "nRoadSearches"),
                                    m_param_nRoadSearches,
                                    "How often the road search should be performed to find new hits",
                                    m_param_nRoadSearches);

      moduleParamList->addParameter(prefixed(prefix, "roadLevel"),
                                    m_param_roadLevel,
                                    "Level of the read from which additional hits in the road search can be taken",
                                    m_param_roadLevel);

      moduleParamList->addParameter(prefixed(prefix, "curvResolution"),
                                    m_param_curvResolution,
                                    "The name of the resolution function to be used. "
                                    "Valid values are 'none', 'const', 'basic', 'origin', 'nonOrigin'",
                                    m_param_curvResolution);
    }

    template <class ANode>
    void AxialLegendreLeafProcessor<ANode>::beginWalk()
    {
      // Setup the requested precision function
      if (m_param_curvResolution == "none") {
        m_curvResolution = [](double curv __attribute__((unused))) { return NAN; };
      } else if (m_param_curvResolution == "const") {
        m_curvResolution = [](double curv __attribute__((unused))) { return 0.0008; };
      } else if (m_param_curvResolution == "basic") {
        m_curvResolution = &PrecisionUtil::getBasicCurvPrecision;
      } else if (m_param_curvResolution == "origin") {
        m_curvResolution = &PrecisionUtil::getOriginCurvPrecision;
      } else if (m_param_curvResolution == "nonOrigin") {
        m_curvResolution = &PrecisionUtil::getNonOriginCurvPrecision;
      } else {
        B2WARNING("Unknown curvature resolution function " << m_param_curvResolution);
        m_curvResolution = [](double curv __attribute__((unused))) { return NAN; };
      }
    }
  }
}
