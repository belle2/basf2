/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_LINE2D_H
#define EKLM_LINE2D_H

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Geometry/Vector3D.h>

namespace Belle2 {

  namespace EKLM {

    class Circle2D;

    /**
     * 2D Line. Equation: m_Point + m_Vector * t
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
       * Find intersection(s) with a circle.
       * @param[in]  circle        Circle.
       * @param[out] intersections Intersections.
       * @return Number of intersections (0, 1 or 2).
       */
      int findIntersection(const Circle2D& circle,
                           HepGeom::Point3D<double> intersections[2]) const;

    private:

      /** Initial point. */
      HepGeom::Point3D<double> m_Point;

      /** Vector. */
      HepGeom::Vector3D<double> m_Vector;

    };

  }

}

#endif

