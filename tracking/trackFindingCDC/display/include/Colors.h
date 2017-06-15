/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <array>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Utility functions related to colors
    class Colors {
    public:
      /// Get a list of useful colors
      static std::vector<std::string> getList();

    public:
      /// Transforms a Color given in the HLS System to RGB.
      static double hueToRgb(double p, double q, double t);

      /**
       *  Transforms a Color given in the HLS System to RGB.
       *
       *  @param h Hue ranging from 0 to 1.
       *  @param l Lighness.
       *  @param s Saturation.
       *  @return Array containing rgb-values in the order red, green, blue.
       */
      static std::array<double, 3> hlsToRgb(double h, double l, double s);

      /**
       *  Get a color from the wheel of colors
       *
       *  @param degree the degree viewed form the center of the color wheel (greater 0)
       */
      static std::string getWheelColor(int degree);
    };
  }
}
