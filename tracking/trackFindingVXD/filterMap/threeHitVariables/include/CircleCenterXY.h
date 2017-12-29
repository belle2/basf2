/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
*               Thomas Lueck (lueck@pi.infn.it)                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/core/FrameworkExceptions.h>
#include <math.h>

#define CIRCLECENTERXY_NAME CircleCenterXY

namespace Belle2 {

  /** calculates the center of the circle for 3 hits in the XY plane and returns a B2Vector3 with the result (z=0).
   * WARNING this method throws an exception!
   * */
  template <typename PointType >
  class CIRCLECENTERXY_NAME : public SelectionVariable< PointType, 3, B2Vector3<double> > {
  public:
    /** this exception is thrown by the CircleFit and occurs when the track is too straight. */
    BELLE2_DEFINE_EXCEPTION(Straight_Line, "The hits are on a straight Line (or indistinguishably near to being on it).");

    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(CIRCLECENTERXY_NAME);

    /** calculates an estimation of circleCenter position, result is returned as the x and y value of the B2Vector3. */
    static B2Vector3<double> value(const PointType& a, const PointType& b, const PointType& c) throw(Straight_Line)
    {
      // calculates the intersection point using Cramer's rule.
      // x_1+s*n_1==x_2+t*n_2 --> n_1 *s - n_2 *t == x_2 - x_1 --> http://en.wikipedia.org/wiki/Cramer%27s_rule
      double inX = b.X() - c.X(); // x value of the normal vector of the inner segment (b-c)
      double inY = b.Y() - c.Y(); // y value of the normal vector of the inner segment (b-c)
      double outX = a.X() - b.X(); // x value of the normal vector of the outer segment (a-b)
      double outY = a.Y() - b.Y(); // y value of the normal vector of the outer segment (a-b)

      //searching solution for Ax = b, aij are the matrix elements of A, bi are elements of b
      double a11 = inY;
      double a12 = -inX;
      double a21 = -outY;
      double a22 = outX;
      double b1 = b.X() + outX * 0.5 - (c.X() + inX * 0.5);
      double b2 = b.Y() + outY * 0.5 - (c.Y() + inY * 0.5);

      if (a11 * a22 == a12 * a21) { throw Straight_Line(); }

      double s = (b1 * a22 - b2 * a21) / (a11 * a22 - a12 * a21); //the determinant is zero if the three hits are on a line in (x,y).

      return B2Vector3<double>(c.X() + inX * 0.5 + s * inY, c.Y() + inY * 0.5 - s * inX, 0.);
    }



  };

}
