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

namespace Belle2 {

  namespace EKLM {

    /**
     * 2D circle.
     */
    class Circle2D {

    public:

      /**
       * Constructor.
       * @param[in] x      Center X coordinate.
       * @param[in] y      Center Y coordinate.
       * @param[in] radius Radius.
       */
      Circle2D(double x, double y, double radius);

      /**
       * Destructor.
       */
      ~Circle2D();

      /**
       * Get center.
       */
      const HepGeom::Point3D<double>& getCenter() const;

      /**
       * Get radius.
       */
      double getRadius() const;

    protected:

      /** Center. */
      HepGeom::Point3D<double> m_Center;

      /** Radius. */
      double m_Radius;

    };

  }

}
