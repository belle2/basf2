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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessorWithNewReferencePoint.h>

#include <vector>
#include <map>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class performs extension (adding new hits) of given candidate using conformal transformation w.r.t point on the trajectory
     */
    class ConformalExtension {

    public:

      /// Constructor
      ConformalExtension(double levelPrecision = 9) : m_levelPrecision(levelPrecision)
      {
      };

      /// Perform transformation for set of given hits; reference position taken as POCA of the fitted trajectory
      std::vector<const CDCWireHit*> newRefPoint(std::vector<const CDCWireHit*>& cdcWireHits,
                                                 const std::vector<const CDCWireHit*>& allAxialWireHits,
                                                 bool doMaskInitialHits = false)
      {
        const CDCKarimakiFitter& fitter = CDCKarimakiFitter::getNoDriftVarianceFitter();
        CDCTrajectory2D trackTrajectory2D = fitter.fit(cdcWireHits);

        double chi2 = trackTrajectory2D.getChi2();
        Vector2D refPos = trackTrajectory2D.getGlobalPerigee();
        double curv =
          -1.*trackTrajectory2D.getCurvature(); //change sign of the curvature; should be the same as the charge of the candidate
        double theta = trackTrajectory2D.getGlobalCircle().phi0() - boost::math::constants::pi<double>() /
                       2.; //theta is an angle between x-axis and vector to the center of the track

        for (const CDCWireHit* hit : cdcWireHits) {
          hit->getAutomatonCell().setTakenFlag(doMaskInitialHits);
          hit->getAutomatonCell().setMaskedFlag(doMaskInitialHits);

        }

        std::vector<const CDCWireHit*> newAssignedHits = getHitsWRTtoRefPos(refPos, curv, theta, allAxialWireHits);

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


      /// perform transformation for the given track; reference position taken as POCA of the fitted trajectory
      std::vector<const CDCWireHit*> newRefPoint(CDCTrack& track,
                                                 const std::vector<const CDCWireHit*>& allAxialWireHits)
      {
        std::vector<const CDCWireHit*> cdcWireHits;

        for (const CDCRecoHit3D& hit : track) {
          cdcWireHits.push_back(&(hit.getWireHit()));
        }

        const CDCKarimakiFitter& fitter = CDCKarimakiFitter::getNoDriftVarianceFitter();
        const CDCTrajectory2D& trackTrajectory2D = fitter.fit(cdcWireHits);

        std::nth_element(track.begin(), track.begin() + track.size() / 2, track.end());

        const CDCRecoHit3D& medianHit = track[track.size() / 2];

        double chi2 = trackTrajectory2D.getChi2();
        Vector2D refPos = medianHit.getRecoPos2D();
        double curv =
          -1.*trackTrajectory2D.getCurvature(); //change sign of the curvature; should be the same as the charge of the candidate
        double theta = trackTrajectory2D.getGlobalCircle().tangential(refPos).phi() - boost::math::constants::pi<double>() /
                       2.; //theta is an angle between x-axis and vector to the center of the track

        for (const CDCWireHit* hit : cdcWireHits) {
          hit->getAutomatonCell().setTakenFlag(true);
          hit->getAutomatonCell().setMaskedFlag(false);

        }

        std::vector<const CDCWireHit*> newAssignedHits = getHitsWRTtoRefPos(refPos, curv, theta, allAxialWireHits);

        if (newAssignedHits.size() > 0) {

          std::vector<const CDCWireHit*> cdcWireHitsNew;

          for (const CDCWireHit* hit : cdcWireHits) {
            cdcWireHitsNew.push_back(hit);
          }

          for (const CDCWireHit* hit : newAssignedHits) {
            cdcWireHitsNew.push_back(hit);
          }

          const CDCTrajectory2D& trackTrajectory2Dtmp = fitter.fit(cdcWireHits);
          double chi2New = trackTrajectory2Dtmp.getChi2();

          if (chi2New < chi2 * 2.) {
            for (const CDCWireHit* hit : newAssignedHits) {
              const CDCRecoHit3D& cdcRecoHit3D  = CDCRecoHit3D::reconstructNearest(hit, trackTrajectory2D);
              track.push_back(std::move(cdcRecoHit3D));
              cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
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
      std::vector<const CDCWireHit*> getHitsWRTtoRefPos(Vector2D refPos, double curv, double theta,
                                                        const std::vector<const CDCWireHit*>& allAxialWireHits)
      {

        double precision_r, precision_theta;
        precision_theta = 3.1415 / (pow(2., m_levelPrecision + 1));
        precision_r = 0.15 / (pow(2., m_levelPrecision));

        AxialHitQuadTreeProcessorWithNewReferencePoint::ChildRanges
        ranges(AxialHitQuadTreeProcessorWithNewReferencePoint::rangeX(static_cast<float>(theta - precision_theta),
               static_cast<float>(theta + precision_theta)),
               AxialHitQuadTreeProcessorWithNewReferencePoint::rangeY(static_cast<float>(curv - precision_r),
                   static_cast<float>(curv + precision_r)));

        std::vector<const CDCWireHit*> hitsVector;
        for (const CDCWireHit* wireHit : allAxialWireHits) {
          if (wireHit->getAutomatonCell().hasTakenFlag() or wireHit->getAutomatonCell().hasMaskedFlag()) continue;
          hitsVector.push_back(wireHit);
        }

        AxialHitQuadTreeProcessorWithNewReferencePoint qtProcessor(ranges, std::make_pair(refPos.x(), refPos.y()));
        qtProcessor.provideItemsSet(hitsVector);

        std::vector<const CDCWireHit*> newCdcWireHits = qtProcessor.getAssignedHits();

        return newCdcWireHits;
      }


    private:
      double m_levelPrecision;   /**< Function which defines precision of conformal transformation */
      //.5 - 0.24 * exp(-4.13118 * TrackCandidate::convertRhoToPt(fabs(track_par.second)) + 2.74);

    };

  }

}

