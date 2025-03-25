/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackingUtilities/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/hough/phi0_curv/Phi0CurvBox.h>
#include <tracking/trackingUtilities/geometry/Vector2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Predicate class to check for the containment of segments in a phi0 curv hough space part.
     *  Unlike hits segments occupy only a point in the hough space.
     */
    class SegmentInPhi0CurvBox {

      /**
       *  Checks if the segment is contained in a phi0 curv hough space.
       *  Returns the size of the segment if it is contained, returns NAN if it is not contained.
       *  The method simply assumes that the segment has been fitted with origin constraint
       *  and simply uses the values from the contained trajectory
       *  The calling code has to ensure this situation
       */
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCSegment2D* segment,
                                           const Phi0CurvBox* phi0CurvBox)
      {
        if (segment->empty()) return NAN;
        const TrackingUtilities::CDCTrajectory2D& trajectory2D = segment->getTrajectory2D();
        TrackingUtilities::Weight weight = operator()(&trajectory2D, phi0CurvBox);
        return weight * segment->size();
      }

      /**
       *  Checks if the given trajectory is considered part of the hough box in phi0, curvature space.
       *  Returns a finit weight if it is contained, NAN if not contained.
       */
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCTrajectory2D* trajectory2D,
                                           const Phi0CurvBox* phi0CurvBox)
      {
        double curvature = trajectory2D->getCurvature();
        const TrackingUtilities::Vector2D& phi0Vec = trajectory2D->getStartUnitMom2D();

        bool in = isPhi0CurvPointIn(phi0Vec, curvature, phi0CurvBox);
        return in ? 1 : NAN;
      }


      /**
       *  Checks if the given hit triplet is considered part of the hough box in phi0, curvature space.
       *  Returns a finit weight if it is contained, NAN if not contained.
       */
      TrackingUtilities::Weight operator()(const TrackingUtilities::CDCFacet* facet,
                                           const Phi0CurvBox* phi0CurvBox)
      {
        const ParameterLine2D& line = facet->getStartToEndLine();

        const TrackingUtilities::Vector2D pos2D = facet->getMiddleRecoPos2D();
        const TrackingUtilities::Vector2D phiVec = line.tangential().unit();

        // Calculate the curvature and phi0 of the circle through the origin
        // that touches the position pos2D under the angle phiVec.
        double curvature = 2 * pos2D.cross(phiVec) / pos2D.normSquared();
        Vector2D phi0Vec = phiVec.flippedOver(pos2D);

        bool in = isPhi0CurvPointIn(phi0Vec, curvature, phi0CurvBox);
        return in ? 1 : NAN;
        // All facets are currently worth the same weight
        // Facets contain three hits but in general contribute only one
        // additional hit due to overlaps with neighboring facets.
        // Hence we stick with 1 as weight here.
      }

      /// Predicate checking if the phi0 vector and curvature are contained in the given box.
      bool isPhi0CurvPointIn(const TrackingUtilities::Vector2D& phi0Vec,
                             const double curvature,
                             const Phi0CurvBox* phi0CurvBox)
      {

        const TrackingUtilities::Vector2D& lowerPhi0Vec = phi0CurvBox->getLowerPhi0Vec();
        const TrackingUtilities::Vector2D& upperPhi0Vec = phi0CurvBox->getUpperPhi0Vec();

        // Allow containment to keep the reversal symmetry
        if (phi0CurvBox->isIn<1>(curvature)) {
          return phi0Vec.isBetween(lowerPhi0Vec, upperPhi0Vec);

        } else if (phi0CurvBox->isIn<1>(-curvature)) {
          return (-phi0Vec).isBetween(lowerPhi0Vec, upperPhi0Vec);

        } else {
          return false;

        }

      }
    };

  }
}
