/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_LINESEGMENT2D_H
#define EKLM_LINESEGMENT2D_H

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>

/* Belle2 headers. */
#include <eklm/geometry/Line2D.h>

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
       * Find intersection(s) with a circle.
       * @param[in]  circle        Circle.
       * @param[out] intersections Intersections.
       * @return Number of intersections (0, 1 or 2).
       */
      int findIntersection(const Circle2D& circle,
                           HepGeom::Point3D<double> intersections[2]) const;

    private:

      /**
       * Check if t is within the line segment (0 <= t <= 1).
       */
      bool tWithinSegment(double t) const;

    };

  }

}

#endif

