/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <utility>
#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Computes the square of an input value, and allows for operations like
    /// TVector3 * TVector3, which results in a double return type
    template<class ANumber>
    inline auto square(ANumber t) -> decltype(t* t) {return t * t;}

    /**
     *  Faster version of the hypot2 funcion.
     *
     *  hypot2 turned out to be a major bottleneck for many calculations
     *  Keep the name though because the expressions can be spelled more clearly
     *  in comparision to std::sqrt(someStuff() * someStuff() + otherStuff() * otherStuff());
     */
    inline double hypot2(double x, double y) { return sqrt(x * x + y * y); }

    /// Calculates the diagonal distance of three coordinate point
    inline double hypot3(double x, double y, double z) { return sqrt(x * x + y * y + z * z); }

    /**
     *  Returns the two roots of abc formula.
     *  Calculates the two roots of the parabola a*x*x + b*x + c = 0 in stable manner. \n
     *  The first value of the returned pair is the solution with the bigger absolut value. \n
     *  The formula also handels the case a == 0 correctly. Here to only root of the linear \n
     *  form b*x + c = 0 is returned in the .second  of the pair, while the bigger 'solution' \n
     *  .first is infinity.
     */
    inline std::pair<double, double> solveQuadraticABC(const double a, const double b, const double c)
    {
      /*if ( a == 0 ){
      // this distinction must not be made since the else block gives the same result
      // we may save some computation steps here but maybe that is not worth while
      // since we most often ask for solutions with a != 0
        // essentially this is the zero position of a line
        smallSolution = - c / b
        bigSolution = std::numeric_limits<double>::infinity() ;
      return result;
      }*/

      const double discriminant = b * b - 4 * a * c;
      const double root = sqrt(discriminant);
      const double bigSum = (b > 0) ? -b - root : -b + root;

      const double bigSolution = bigSum / 2 / a;
      const double smallSolution = 2 * c / bigSum;

      return {bigSolution, smallSolution};
    }
  }
}
