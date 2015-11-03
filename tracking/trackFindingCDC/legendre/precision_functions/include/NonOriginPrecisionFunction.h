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

#include <tracking/trackFindingCDC/legendre/precision_functions/BasePrecisionFunction.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /*
     * Basic function for checking quadtree's node "r" boundaries:
     * when used in quadtree processor marks nodes at lvl==13 as leafs
     */
    class NonOriginPrecisionFunction : public BasePrecisionFunction {

    public:

      NonOriginPrecisionFunction()
      {
        m_function = [&](double r_qt) -> double {
          double res;
          if ((convertRhoToPt(fabs(r_qt)) > 3.) && (r_qt != 0))
            r_qt = fabs(convertPtToRho(3.)) * r_qt / fabs(r_qt);

          if (r_qt != 0)
            if (convertRhoToPt(fabs(r_qt)) < 0.36)
              res = exp(-0.356965 - 0.00186066 * convertRhoToPt(fabs(r_qt))) - 0.697526;
            else
              res = exp(-0.357335 + 0.000438872 * convertRhoToPt(fabs(r_qt))) - 0.697786;
          else
            res = 0.00005;
          B2DEBUG(100, "non origin: res = " << res << "; r = " << r_qt);
          return res;
        };
      };

      virtual ~NonOriginPrecisionFunction() {};


    };
  }
}
