/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>

/* Belle2 headers. */
#include <eklm/geometry/Arc2D.h>
#include <eklm/geometry/LineSegment2D.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * 2D polygon.
     */
    class Polygon2D {

    public:

      /**
       * Constructor.
       * @param[in] points Points.
       * @param[in] n      Number of points.
       */
      Polygon2D(const HepGeom::Point3D<double>* points, int n);

      /**
       * Copy constructor (disabled).
       */
      Polygon2D(const Polygon2D&) = delete;

      /**
       * Operator = (disabled).
       */
      Polygon2D& operator=(const Polygon2D&) = delete;

      /**
       * Destructor.
       */
      ~Polygon2D();

      /**
       * Check if point is inside the polygon.
       */
      bool pointInside(const HepGeom::Point3D<double>& point) const;

      /**
       * Check whether polygon has an intersection with a line segment or
       * this line segment is fully inside the polygon.
       * @param[in] lineSegment Line segment.
       */
      bool hasIntersection(const LineSegment2D& lineSegment) const;

      /**
       * Check whether polygon has an intersection with an arc or
       * this arc is fully inside the polygon.
       * @param[in] arc Arc.
       */
      bool hasIntersection(const Arc2D& arc) const;

      /**
       * Check whether polygon has an intersection with a polygon
       * or one of the polygons is fully inside another polygon.
       * @param[in] lineSegment Line segment.
       */
      bool hasIntersection(const Polygon2D& polygon) const;

      /**
       * Get line segments.
       */
      LineSegment2D** getLineSegments() const;

    private:

      /** Number of points. */
      int m_nPoints;

      /** Line segments. */
      LineSegment2D** m_LineSegments;

    };

  }

}
