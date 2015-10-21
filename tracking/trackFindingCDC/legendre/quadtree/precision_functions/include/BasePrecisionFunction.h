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

namespace Belle2 {
  namespace TrackFindingCDC {

    /*
     * Basic function for checking quadtree's node "r" boundaries:
     * when used in quadtree processor marks nodes at lvl==13 as leafs
     */
    class BasePrecisionFunction {

    public:

      typedef std::function< double(double)> PrecisionFunction;

      BasePrecisionFunction()
      {
        m_function = [&](double __attribute__((unused)) r_qt) -> double {
          return 0.3 / pow(2, 16);
        };
      };

      virtual ~BasePrecisionFunction() {};

      PrecisionFunction& getFunction() { return m_function; };

      /** convert rho (one of the axis in legendre phase-space) to Pt (in GeV) */
      double convertRhoToPt(double rho) {return 1.5 * 0.00299792458 / fabs(rho); };

      /** Pt (in GeV)convert  to rho (one of the axis in legendre phase-space) */
      double convertPtToRho(double pt) {return 1.5 * 0.00299792458 / fabs(pt); };

    protected:

      PrecisionFunction m_function;


    };
  }
}
