/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessorWithNewReferencePoint.h>
#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class performs extension (adding new hits) of given candidate using conformal transformation w.r.t point on the trajectory
     */
    class OffOriginExtension {

    public:
      /// Constructor
      OffOriginExtension(std::vector<const CDCWireHit*> allAxialWireHits, double levelPrecision = 9)
        : m_allAxialWireHits(std::move(allAxialWireHits))
        , m_levelPrecision(levelPrecision)
      {
      }

      /// Main entry point for the post processing call from the QuadTreeProcessor
      void operator()(const std::vector<const CDCWireHit*>& inputWireHits,
                      void* qt  __attribute__((unused)))
      {
        std::vector<const CDCWireHit*> candidateHits = inputWireHits;

        // Unset the taken flag and let the postprocessing decide
        for (const CDCWireHit* wireHit : candidateHits) {
          (*wireHit)->setTakenFlag(false);
        }

        roadSearch(candidateHits);
        AxialTrackUtil::addCandidateFromHitsWithPostprocessing(candidateHits, m_allAxialWireHits, m_tracks);
      }

      /// Perform transformation for set of given hits; reference position taken as POCA of the fitted trajectory
      std::vector<const CDCWireHit*> roadSearch(std::vector<const CDCWireHit*>& cdcWireHits)
      {
        const CDCKarimakiFitter& fitter = CDCKarimakiFitter::getNoDriftVarianceFitter();
        CDCTrajectory2D trackTrajectory2D = fitter.fit(cdcWireHits);

        double chi2 = trackTrajectory2D.getChi2();
        Vector2D refPos = trackTrajectory2D.getGlobalPerigee();

        // change sign of the curvature; should be the same as the charge of the candidate
        double curv = trackTrajectory2D.getCurvature();

        // theta is the vector normal to the trajectory at the perigee
        double theta = trackTrajectory2D.getGlobalCircle().phi0() + M_PI_2;

        for (const CDCWireHit* hit : cdcWireHits) {
          hit->getAutomatonCell().setTakenFlag(true);
          hit->getAutomatonCell().setMaskedFlag(true);

        }

        std::vector<const CDCWireHit*> newAssignedHits = getHitsWRTtoRefPos(refPos, curv, theta);

        if (newAssignedHits.size() > 0) {

          std::vector<const CDCWireHit*> cdcWireHitsNew;

          for (const CDCWireHit* hit : cdcWireHits) {
            cdcWireHitsNew.push_back(hit);
          }

          for (const CDCWireHit* hit : newAssignedHits) {
            cdcWireHitsNew.push_back(hit);
          }

          CDCTrajectory2D trackTrajectory2Dtmp = fitter.fit(cdcWireHits);

          double chi2New = trackTrajectory2Dtmp.getChi2();

          if (chi2New < chi2 * 2.) {
            for (const CDCWireHit* hit : newAssignedHits) {
              cdcWireHits.push_back(hit);
            }
          }
        }

        for (const CDCWireHit* hit : cdcWireHits) {
          hit->getAutomatonCell().setTakenFlag(false);
          hit->getAutomatonCell().setMaskedFlag(false);
        }

        return newAssignedHits;
      }

      /**
       * @brief Get hits which are compatible with given trajectory
       *
       * @param refPos defines 2D reference position with respect to which transformation will be performed
       * @param curv curvarute of the track trajectory
       * @param theta angle between x-axis and vector to the center of the circle which represents trajectory
       * @return vector of CDCWireHit objects which satisfy legendre transformation with respect to the given parameters
       */
      std::vector<const CDCWireHit*> getHitsWRTtoRefPos(Vector2D refPos, float curv, float theta)
      {
        float thetaPrecision = 3.1415 / (pow(2., m_levelPrecision + 1));
        float curvPrecision = 0.15 / (pow(2., m_levelPrecision));

        using XYSpans = AxialHitQuadTreeProcessorWithNewReferencePoint::XYSpans;
        XYSpans ranges({theta - thetaPrecision, theta + thetaPrecision},
        {curv - curvPrecision, curv + curvPrecision});

        AxialHitQuadTreeProcessorWithNewReferencePoint qtProcessor(ranges, std::make_pair(refPos.x(), refPos.y()));
        qtProcessor.seed(m_allAxialWireHits);

        std::vector<const CDCWireHit*> newWireHits = qtProcessor.getAssignedHits();
        return newWireHits;
      }

      /// Get the collected tracks
      const std::vector<CDCTrack>& getTracks() const
      {
        return m_tracks;
      }

    private:
      /// Pool of all axial hits from which the road search may select additional hits
      std::vector<const CDCWireHit*> m_allAxialWireHits;

      /// Collected tracks
      std::vector<CDCTrack> m_tracks;

      /// Precision level for the width of the off origin hough search
      double m_levelPrecision;
      //.5 - 0.24 * exp(-4.13118 * TrackCandidate::convertRhoToPt(fabs(track_par.second)) + 2.74);
    };
  }
}
