/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* CLHEP headers. */
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
      const HepGeom::Point3D<double>& getCenter() const
      {
        return m_Center;
      }

      /**
       * Get radius.
       */
      double getRadius() const
      {
        return m_Radius;
      }

    protected:

      /** Center. */
      HepGeom::Point3D<double> m_Center;

      /** Radius. */
      double m_Radius;

    };

  }

}
