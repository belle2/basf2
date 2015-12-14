/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_POLYGON2D_H
#define EKLM_POLYGON2D_H

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
       * Destructor.
       */
      ~Polygon2D();

      /**
       * Check whether polygon has an intersection with a line segment.
       * @param[in] lineSegment Line segment.
       */
      bool hasIntersection(const LineSegment2D& lineSegment) const;

      /**
       * Check whether polygon has an intersection with an arc.
       * @param[in] arc Arc.
       */
      bool hasIntersection(const Arc2D& arc) const;

      /**
       * Check whether polygon has an intersection with a polygon.
       * @param[in] lineSegment Line segment.
       */
      bool hasIntersection(const Polygon2D& polygon) const;

    private:

      /** Number of points. */
      int m_nPoints;

      /** Line segments. */
      LineSegment2D** m_LineSegments;

    };

  }

}

#endif

