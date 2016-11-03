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

#include <tracking/trackFindingCDC/legendre/precisionFunctions/BasePrecisionFunction.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Function for checking quadtree's node "r" boundaries;
     * used upon making decision whether quadtree node is a leaf or not
     */
    class OriginPrecisionFunction : public BasePrecisionFunction {

    public:

      /// Constructor
      OriginPrecisionFunction()
      {
        // function which estimates desired rho (curvature) resolution of quadtree node in the given pt (1/rho) region
        // parameters of the function are taken from the fit:
        //     10000 of pion tracks were generated with particle gun produced at (0,0,0) (IP) with pt=[0.05;2.0]GeV, phi=[-2pi;2pi]
        //     by resolution we imply size of the quadtree node which can cover 80% of the hits in legendre phase-space
        //     resolution was estimated in bins of pt with step of 200MeV
        //     distribution of resolutions was fitted with function exp(a+b*pt)+c+d*pt (this function has been choosen as it can describe the shape of the distribution)
        // this function takes into account smearing of the track in legendre space due to energy losses
        m_function = [&](double r_qt) -> double {
          double res;
          //TODO: bug is here!
          if ((convertRhoToPt(fabs(r_qt)) > 3.) && (r_qt != 0))
          {
            r_qt = fabs(convertPtToRho(3.)) * r_qt / fabs(r_qt);
          }

          if (r_qt != 0)
          {
            res = exp(-16.1987 * convertRhoToPt(fabs(r_qt)) - 5.96206)
            + 0.000190872 - 0.0000739319 * convertRhoToPt(fabs(r_qt));

          } else {
            res = 0.00005;
          }
          /*10.5 - 0.24 * exp(-4.13118 * convertRhoToPt(curv) + 2.74);*/
          B2DEBUG(100, "origin: res = " << res << "; r = " << r_qt);
          return res;
        };
      };

    };
  }
}
