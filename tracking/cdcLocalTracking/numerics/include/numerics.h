/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef NUMERICS_H
#define NUMERICS_H

#include <math.h>
#include <utility>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/SignType.h>

namespace Belle2 {

  namespace CDCLocalTracking {
    /// The famous number
    const FloatType PI = 3.141592653589793238462;

    ///Checks if an integer is even
    template<class IntNumber>
    inline bool isEven(const IntNumber& x)
    { return (x % 2) == 0; }

    ///Checks if an integer is odd
    template<class IntNumber>
    inline bool isOdd(const IntNumber& x)
    { return (x % 2) != 0; }


    ///Checks if a float is not a number
    bool isNAN(const double x);

    ///Checks if a float is not a number
    bool isNAN(const float x);

    ///Returns the sign of an integer number
    inline SignType sign(int x)
    { return x > 0 ? PLUS : (x < 0 ? MINUS : ZERO); }

    ///Returns the sign of a floating point number.
    /** Essentially return the signbit of the float.
     *  This means 0.0 has sign PLUS while -0.0 has sign MINUS
     *  NAN is treat specially and returns an INVALID_SIGN
     */
    SignType sign(double x);

    /// Returns the two roots in pq formula
    /** Calculates the two roots of the parabola x*x + p*x + q = 0 in a rather stable manner \n
     *  The first value of the returned pair is the solution with the bigger absolut value. \n
     *  If you ask for the roots of a*x*x + b*x + c = 0, do not your formula to the pq from but use \n
     *  the better behaved solveQuadraticABC */
    inline std::pair<FloatType, FloatType> solveQuadraticPQ(FloatType p, FloatType q)
    {
      //solves the quadratic equation x*x + p*x + q == 0 in a rather stable manner (see book numerical recipes in c++)
      //the result has only float precision though, if the discriminant phalf*phalf - q is small
      //use always double precision in intermediate steps
      const double phalf = p / 2;
      const double discriminant = phalf * phalf - q;

      // calculate bigger solution first !!!
      const double solutionOne = (p > 0) ?  -phalf  - sqrt(discriminant)  : -phalf  + sqrt(discriminant) ;

      //second solution by vieta identity
      return std::pair<FloatType, FloatType>(solutionOne, q / solutionOne);
    }

    /// Returns the two roots of abc formula
    /** Calculates the two roots of the parabola a*x*x + b*x + c = 0 in stable manner. \n
     *  The first value of the returned pair is the solution with the bigger absolut value. \n
     *  The formula also handels the case a == 0 correctly. Here to only root of the linear \n
     *  form b*x + c = 0 is returned in the .second  of the pair, while the bigger 'solution' \n
     *  .first is infinity */
    inline std::pair<FloatType, FloatType> solveQuadraticABC(const FloatType& a, const FloatType& b, const FloatType& c)
    {

      std::pair<FloatType, FloatType> result;

      FloatType& bigSolution = result.first;
      FloatType& smallSolution = result.second;

      /*if ( a == 0 ){
        //this distinction must not be made since the else block gives the same result
        //we may save some computation steps here but maybe that is not worth while
        //since we most often ask for solutions with a != 0

        //essentially this is the zero position of a line
        smallSolution = - c / b
        bigSolution = std::numeric_limits<FloatType>::infinity() ;
      return result;
      }
      */

      const double discriminant = ((double)b) * b - 4 * a * c;
      const double root = sqrt(discriminant);

      const double bigSum = (b > 0) ? -b - root : -b + root;

      bigSolution = bigSum / 2 / a;
      smallSolution = 2 * c / bigSum;

      return result;
    }


  } // end namespace CDCLocalTracking
} // end namespace Belle2

#endif // NUMERICS_H
