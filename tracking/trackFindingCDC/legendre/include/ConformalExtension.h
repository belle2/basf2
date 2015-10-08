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
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>
#include <tracking/trackFindingCDC/legendre/TrackProcessor.h>
#include <tracking/trackFindingCDC/legendre/QuadTreeHitWrapper.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessorWithNewReferencePoint.h>

#include <list>
#include <vector>
#include <map>


namespace Belle2 {
  namespace TrackFindingCDC {

    class ConformalExtension {

    public:

      ConformalExtension(TrackProcessorNew* trackProcessor, double levelPrecision = 9): m_trackProcessor(trackProcessor),
        m_levelPrecision(levelPrecision)
      {
      };

      ~ConformalExtension() {};

      std::vector<const CDCWireHit*> newRefPoint(std::vector<const CDCWireHit*>& cdcWireHits, bool doMaskInitialHits = false)
      {
        CDCTrajectory2D trackTrajectory2D ;

        trackTrajectory2D = TrackFitter::fitWireHitsWhithoutRecoPos(cdcWireHits);

        double chi2 = trackTrajectory2D.getChi2();
        Vector2D refPos = trackTrajectory2D.getGlobalPerigee();
        double curv =
          -1.*trackTrajectory2D.getCurvature(); //change sign of the curvature; should be the same as the charge of the candidate
        double theta = trackTrajectory2D.getGlobalCircle().tangentialPhi() - boost::math::constants::pi<double>() /
                       2.; //theta is an angle between x-axis and vector to the center of the track

        for (const CDCWireHit* hit : cdcWireHits) {
          hit->getAutomatonCell().setTakenFlag(doMaskInitialHits);
          hit->getAutomatonCell().setMaskedFlag(doMaskInitialHits);

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

          CDCTrajectory2D trackTrajectory2Dtmp = TrackFitter::fitWireHitsWhithoutRecoPos(cdcWireHits);

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



      std::vector<const CDCWireHit*> newRefPoint(CDCTrack& track)
      {
        std::vector<const CDCWireHit*> cdcWireHits;

        for (const CDCRecoHit3D& hit : track) {
          cdcWireHits.push_back(&(hit.getWireHit()));
        }

        CDCTrajectory2D trackTrajectory2D ;

        trackTrajectory2D = TrackFitter::fitWireHitsWhithoutRecoPos(cdcWireHits);

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

          CDCTrajectory2D trackTrajectory2Dtmp = TrackFitter::fitWireHitsWhithoutRecoPos(cdcWireHits);

          double chi2New = trackTrajectory2Dtmp.getChi2();

          if (chi2New < chi2 * 2.) {
            for (const CDCWireHit* hit : newAssignedHits) {
              const CDCRecoHit3D& cdcRecoHit3D  = HitProcessor::createRecoHit3D(trackTrajectory2D, hit);
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


      std::vector<const CDCWireHit*> getHitsWRTtoRefPos(Vector2D refPos, double curv, double theta)
      {

        double precision_r, precision_theta;
        precision_theta = 3.1415 / (pow(2., m_levelPrecision + 1));
        precision_r = 0.15 / (pow(2., m_levelPrecision));

        AxialHitQuadTreeProcessorWithNewReferencePoint::ChildRanges
        ranges(AxialHitQuadTreeProcessorWithNewReferencePoint::rangeX(static_cast<float>(theta - precision_theta),
               static_cast<float>(theta + precision_theta)),
               AxialHitQuadTreeProcessorWithNewReferencePoint::rangeY(static_cast<float>(curv - precision_r),
                   static_cast<float>(curv + precision_r)));

        std::vector<QuadTreeHitWrapper*> hitsVector = m_trackProcessor->createQuadTreeHitWrappersForQT(false);
//        B2INFO("unuses hits: " << hitsVector.size() << "; candidateHits: " << cdcWireHits.size() << "; theta: " << theta << "; curv: " << curv );

        for (QuadTreeHitWrapper* hit : hitsVector) {
          hit->setUsedFlag(false);
          hit->setMaskedFlag(false);
        }

        AxialHitQuadTreeProcessorWithNewReferencePoint qtProcessor(ranges, std::make_pair(refPos.x(), refPos.y()));
        qtProcessor.provideItemsSet(hitsVector);

        std::vector<QuadTreeHitWrapper*> newAssignedHits = qtProcessor.getAssignedHits();

//          B2INFO("assigned hits: " << newAssignedHits.size());
        std::vector<const CDCWireHit*> newCdcWireHits;

        for (QuadTreeHitWrapper* hit : newAssignedHits) {
          newCdcWireHits.push_back(hit->getCDCWireHit());
        }


        return newCdcWireHits;
      }





    private:

      TrackProcessorNew* m_trackProcessor;
      double m_levelPrecision;//.5 - 0.24 * exp(-4.13118 * TrackCandidate::convertRhoToPt(fabs(track_par.second)) + 2.74);

    };

  }

}

