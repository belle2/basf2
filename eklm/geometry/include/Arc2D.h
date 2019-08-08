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

/* Belle2 headers. */
#include <eklm/geometry/Circle2D.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * 2D arc.
     */
    class Arc2D : public Circle2D {

    public:

      /**
       * Constructor.
       * @param[in] x      Center X coordinate.
       * @param[in] y      Center Y coordinate.
       * @param[in] radius Radius.
       * @param[in] angle1 Angle 1.
       * @param[in] angle2 Angle 2.
       *
       * Values of angles must be from -M_PI to M_PI (as returned by atan2()).
       */
      Arc2D(double x, double y, double radius, double angle1, double angle2);

      /**
       * Destructor.
       */
      ~Arc2D();

      /**
       * Check if angle is within the arc.
       * @param[in] angle Angle from -M_PI to M_PI.
       */
      bool angleWithinRange(double angle) const;

      /**
       * Get initial point.
       */
      HepGeom::Point3D<double> getInitialPoint() const;

    private:

      /** Angle 1. */
      double m_Angle1;

      /** Angle 2. */
      double m_Angle2;

    };

  }

}
