/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/eklm/geometry/Circle2D.h>
#include <klm/eklm/geometry/Arc2D.h>

/* CLHEP headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Geometry/Vector3D.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * 2D line. Equation: m_Point + m_Vector * t
     */
    class Line2D {

    public:

      /**
       * Constructor.
       * @param[in] x    Initial point X coordinate.
       * @param[in] y    Initial point y coordinate.
       * @param[in] vecx Vector X component.
       * @param[in] vecy Vector Y component.
       */
      Line2D(double x, double y, double vecx, double vecy);

      /**
       * Destructor.
       */
      ~Line2D();

      /**
       * Get initial point.
       */
      const HepGeom::Point3D<double>& getInitialPoint() const
      {
        return m_Point;
      }

      /**
       * Get vector.
       */
      const HepGeom::Vector3D<double>& getVector() const
      {
        return m_Vector;
      }

      /**
       * Find intersection with a line.
       * @param[in]  line         Line.
       * @param[out] intersection Intersection.
       * @return Number of intersections (0 or 1).
       */
      int findIntersection(const Line2D& line,
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
       * Find intersection with a line.
       * @param[in]  line         Line.
       * @param[out] intersection Intersection.
       * @param[out] t            Values of t for intersection point
       *                          (t[0] - this line, t[1] - line from argument).
       * @return Number of intersections (0 or 1).
       *
       * If (*this) and line are the same line then 0 is returned.
       */
      int findIntersection(const Line2D& line,
                           HepGeom::Point3D<double>* intersection,
                           double t[2]) const;

    protected:

      /**
       * Find intersections with a circle.
       * @param[in]  circle        Circle.
       * @param[out] intersections Intersections.
       * @param[out] t             Values of t for intersection points.
       * @param[out] angles        Values of angles for intersection points.
       * @return Number of intersections (0, 1 or 2).
       */
      int findIntersection(const Circle2D& circle,
                           HepGeom::Point3D<double> intersections[2],
                           double t[2], double angles[2]) const;

      /**
       * Select intersections.
       * @param[in,out] intersections Intersections.
       * @param[in]     condition     Selection condition.
       * @param[in]     n             Number of intersections.
       * @return Number of selected intersections.
       */
      int selectIntersections(HepGeom::Point3D<double>* intersections,
                              bool* condition, int n) const;

      /** Initial point. */
      HepGeom::Point3D<double> m_Point;

      /** Vector. */
      HepGeom::Vector3D<double> m_Vector;

    };

  }

}
