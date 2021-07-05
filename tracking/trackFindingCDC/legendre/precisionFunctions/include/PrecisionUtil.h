/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>
#include <functional>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Utility collection for functions to determine a curvature precision such at a hough box
     * covers a certain percentage of hits in the legendre algorithms.
     */
    class PrecisionUtil {

    public: // Auxiliary stuff
      /// Returns desired deepness of the trigonometrical lookup table. Used as template parameter for the TrigonometricalLookupTable<> class.
      static constexpr int getLookupGridLevel() {return c_lookupGridLevel; };

      /// Deepness of the trigonometrical lookup table.
      static constexpr const int c_lookupGridLevel = 16;

      /// convert curvature (one of the axis in legendre phase-space) to Pt (in GeV)
      static double convertRhoToPt(double curv) {return 1.5 * 0.00299792458 / fabs(curv); };

    public: // Variations of precision functions
      /**
       * Function type which is used for resolution calculations (resolution=f(curvature))
       * Takes a curvature value and returns a width in the curvature direction such that
       * a certain amout of hits is covered by the box in the hough space.
       */
      using PrecisionFunction = std::function<double(double)>;

      /**
       * Basic function to estimate the curvature precision
       * Takes a curvature value and returns a width that
       */
      static double getBasicCurvPrecision(double __attribute__((unused)) curv)
      {
        // 0.3 is the width of the Legendre phase-space in curvatures direction.
        return 0.3 / pow(2, 16);
      }

      /**
       * Function which estimates desired curvature resolution of quadtree node in the given pt region
       * parameters of the function are taken from the fit:
       *
       * 10000 of pion tracks were generated with particle gun produced at (0,0,0) (IP) with pt=[0.05;2.0]GeV, phi=[-2pi;2pi]
       * by resolution we imply size of the quadtree node which can cover 80% of the hits in legendre phase-space
       * resolution was estimated in bins of pt with step of 200MeV
       * distribution of resolutions was fitted with function exp(a+b*pt)+c+d*pt (this function has been choosen as it can describe the shape of the distribution)
       * this function takes into account smearing of the track in legendre space due to energy losses
       */
      static double getOriginCurvPrecision(double curv)
      {
        // Convert to pt making the cut-off of that has been here before
        double pt = convertRhoToPt(curv);
        if (not(pt <= 3.0)) {
          pt = 3.0;
        }

        double precision = exp(-16.1987 * pt - 5.96206) + 0.000190872 - 0.0000739319 * pt;

        /* 10.5 - 0.24 * exp(-4.13118 * convertRhoToPt(curv) + 2.74); */
        B2DEBUG(100, "origin: precision = " << precision << "; curv = " << curv);
        return precision;
      }

      /**
       * Function which estimates desired curvature resolution of quadtree node in the given pt region
       * parameters of the function are taken from the fit:
       *
       * 10000 of pion tracks were generated with particle gun with impact parameter in XY plane of 3 cm (pt=[0.05;2.0]GeV, phi=[-2pi;2pi])
       * by resolution we imply size of the quadtree node which can cover 80% of the hits in legendre phase-space
       * resolution was estimated in bins of pt with step of 200MeV
       * distribution of resolutions was fitted with function exp(a+b*pt)+c (this function has been choosen as it can describe the shape of the distribution)
       * this function takes into account smearing of the track in legendre space due to non-prompt production and energy losses
       */
      static double getNonOriginCurvPrecision(double curv)
      {
        // Convert to pt making the cut-off of that has been here before
        double pt = convertRhoToPt(curv);
        if (not(pt <= 3.0)) {
          pt = 3.0;
        }

        double precision{};
        if (pt < 0.36) {
          precision = exp(-0.356965 - 0.00186066 * pt) - 0.697526;
        } else {
          precision = exp(-0.357335 + 0.000438872 * pt) - 0.697786;
        }

        B2DEBUG(100, "non origin: precision = " << precision << "; curv = " << curv);
        return precision;
      }
    };
  }
}
