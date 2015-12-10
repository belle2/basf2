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

    /*
     * Basic function for checking quadtree's node "r" boundaries:
     * when used in quadtree processor marks nodes at lvl==13 as leafs
     */
    class OriginPrecisionFunction : public BasePrecisionFunction {

    public:

      OriginPrecisionFunction()
      {
        m_function = [&](double r_qt) -> double {
          double res;
          //TODO: bug is here!
          if ((convertRhoToPt(fabs(r_qt)) > 3.) && (r_qt != 0))
            r_qt = fabs(convertPtToRho(3.)) * r_qt / fabs(r_qt);

          if (r_qt != 0)
            res = exp(-16.1987 * convertRhoToPt(fabs(r_qt)) - 5.96206)
            + 0.000190872 - 0.0000739319 * convertRhoToPt(fabs(r_qt));

          else
            res = 0.00005;
          /*10.5 - 0.24 * exp(-4.13118 * convertRhoToPt(curv) + 2.74);*/
          B2DEBUG(100, "origin: res = " << res << "; r = " << r_qt);
          return res;
        };
      };



    };
  }
}
