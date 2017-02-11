/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <functional>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Basic function for checking quadtree's node "r" boundaries:
     * when used in quadtree processor marks nodes at lvl==13 as leafs
     */
    class PrecisionUtil {

    public: // Auxiliary stuff
      /// Returns desired deepness of the trigonometrical lookup table. Used as template parameter for the TrigonometricalLookupTable<> class.
      static constexpr int getLookupGridLevel() {return c_lookupGridLevel; };

      /// Deepness of the trigonometrical lookup table.
      static constexpr const int c_lookupGridLevel = 16;

      /// convert rho (one of the axis in legendre phase-space) to Pt (in GeV)
      static double convertRhoToPt(double rho) {return 1.5 * 0.00299792458 / fabs(rho); };

      /// Pt (in GeV)convert  to rho (one of the axis in legendre phase-space)
      static double convertPtToRho(double pt) {return 1.5 * 0.00299792458 / fabs(pt); };

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
      static double getBasicCurvPrecision(double __attribute__((unused)) r_qt)
      {
        // 0.3 is the width of the Legendre phase-space in curvatures direction.
        return 0.3 / pow(2, 16);
      }

      /**
       * Function which estimates desired rho (curvature) resolution of quadtree node in the given pt (1/rho) region
       * parameters of the function are taken from the fit:
       *
       * 10000 of pion tracks were generated with particle gun produced at (0,0,0) (IP) with pt=[0.05;2.0]GeV, phi=[-2pi;2pi]
       * by resolution we imply size of the quadtree node which can cover 80% of the hits in legendre phase-space
       * resolution was estimated in bins of pt with step of 200MeV
       * distribution of resolutions was fitted with function exp(a+b*pt)+c+d*pt (this function has been choosen as it can describe the shape of the distribution)
       * this function takes into account smearing of the track in legendre space due to energy losses
       */
      static double getOriginCurvPrecision(double r_qt)
      {
        double res;
        // TODO: bug is here!
        if ((convertRhoToPt(fabs(r_qt)) > 3.) && (r_qt != 0)) {
          r_qt = fabs(convertPtToRho(3.)) * r_qt / fabs(r_qt);
        }

        if (r_qt != 0) {
          res = exp(-16.1987 * convertRhoToPt(fabs(r_qt)) - 5.96206)
                + 0.000190872 - 0.0000739319 * convertRhoToPt(fabs(r_qt));

        } else {
          res = 0.00005;
        }

        /* 10.5 - 0.24 * exp(-4.13118 * convertRhoToPt(curv) + 2.74); */
        B2DEBUG(100, "origin: res = " << res << "; r = " << r_qt);
        return res;
      }

      /**
       * Function which estimates desired rho (curvature) resolution of quadtree node in the given pt (1/rho) region
       * parameters of the function are taken from the fit:
       *     10000 of pion tracks were generated with particle gun with impact parameter in XY plane of 3 cm (pt=[0.05;2.0]GeV, phi=[-2pi;2pi])
       *     by resolution we imply size of the quadtree node which can cover 80% of the hits in legendre phase-space
       *     resolution was estimated in bins of pt with step of 200MeV
       *     distribution of resolutions was fitted with function exp(a+b*pt)+c (this function has been choosen as it can describe the shape of the distribution)
       * this function takes into account smearing of the track in legendre space due to non-prompt production and energy losses
       */
      static double getNonOriginCurvPrecision(double r_qt)
      {

        double res;
        if ((convertRhoToPt(fabs(r_qt)) > 3.) && (r_qt != 0)) {
          r_qt = fabs(convertPtToRho(3.)) * r_qt / fabs(r_qt);
        }

        if (r_qt != 0) {
          if (convertRhoToPt(fabs(r_qt)) < 0.36) {
            res = exp(-0.356965 - 0.00186066 * convertRhoToPt(fabs(r_qt))) - 0.697526;
          } else {
            res = exp(-0.357335 + 0.000438872 * convertRhoToPt(fabs(r_qt))) - 0.697786;
          }
        } else {
          res = 0.00005;
        }
        B2DEBUG(100, "non origin: res = " << res << "; r = " << r_qt);
        return res;
      }
    };
  }
}
