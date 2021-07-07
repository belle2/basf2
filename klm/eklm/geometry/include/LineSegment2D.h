/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/eklm/geometry/Line2D.h>

/* CLHEP headers. */
#include <CLHEP/Geometry/Point3D.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * 2D line segment. Equation: m_Point + m_Vector * t, t = [0, 1].
     */
    class LineSegment2D : public Line2D {

    public:

      /**
       * Constructor.
       * @param[in] point1 First point.
       * @param[in] point2 Second point.
       */
      LineSegment2D(const HepGeom::Point3D<double>& point1,
                    const HepGeom::Point3D<double>& point2);

      /**
       * Destructor.
       */
      ~LineSegment2D();

      /**
       * Find intersection with a line.
       * @param[in]  line         Line.
       * @param[out] intersection Intersection.
       * @return Number of intersections (0 or 1).
       */
      int findIntersection(const Line2D& line,
                           HepGeom::Point3D<double>* intersection) const;

      /**
       * Find intersection with a line segment.
       * @param[in]  lineSegment  Line segment.
       * @param[out] intersection Intersection.
       * @return Number of intersections (0 or 1).
       */
      int findIntersection(const LineSegment2D& lineSegment,
                           HepGeom::Point3D<double>* intersection) const;

      /**
       * Find intersections with a circle.
       * @param[in]  circle        Circle.
       * @param[out] intersections Intersections.
       * @return Number of intersections (0, 1 or 2).
       */
      int findIntersection(const Circle2D& circle,
                           HepGeom::Point3D<double> intersections[2]) const;

      /**
       * Find intersections with an arc.
       * @param[in]  arc           Arc.
       * @param[out] intersections Intersections.
       * @return Number of intersections (0, 1 or 2).
       */
      int findIntersection(const Arc2D& arc,
                           HepGeom::Point3D<double> intersections[2]) const;

      /**
       * Check if t is within the line segment (0 <= t <= 1).
       * @param[in] t Line parameter.
       */
      bool tWithinRange(double t) const;

    };

  }

}
