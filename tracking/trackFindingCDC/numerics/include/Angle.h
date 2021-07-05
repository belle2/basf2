/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {


    /**
     *  Utility functions to be used with angular quantities.
     *  All angles are normalised to be in the range [-pi, pi]
     */
    struct AngleUtil {

      /// Combines two angluar values to the one that lies half way between them on the short arc
      static double average(const double angle1, double angle2)
      {
        return normalised(angle1 + normalised(angle2 - angle1) / 2);
      }

      /// Normalise an angle to lie in the range from [-pi, pi]
      static double normalised(const double angle)
      {
        if (angle > M_PI) return angle - 2 * M_PI;
        if (angle < -M_PI) return angle + 2 * M_PI;
        return angle;
      }

      /// Normalise an angle inplace to lie in the range from [-pi, pi]
      static void normalise(double& angle)
      {
        if (angle > M_PI) angle -= 2 * M_PI;
        if (angle < -M_PI) angle += 2 * M_PI;
      }

      /// Normalise an angle to lie in the range from [-pi, pi]
      static double fullNormalised(const double angle)
      {
        return std::remainder(angle, 2 * M_PI);
      }

      /// Get the angle that point in the opposite direction.
      static double reversed(const double angle)
      {
        return angle > 0 ? angle - M_PI : angle + M_PI;
      }

    };

  }
}
