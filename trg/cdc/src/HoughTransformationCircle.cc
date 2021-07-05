/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughTransformationCircle.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent circle Hough transformation.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <math.h>
#include "trg/trg/Point2D.h"
#include "trg/cdc/HoughTransformationCircle.h"



namespace Belle2 {

  TRGCDCHoughTransformationCircle::TRGCDCHoughTransformationCircle(
    const std::string& name)
    : TRGCDCHoughTransformation(name)
  {
  }

  TRGCDCHoughTransformationCircle::~TRGCDCHoughTransformationCircle()
  {
  }

  float
  TRGCDCHoughTransformationCircle::y(float xReal, float yReal, float x) const
  {
//    return log10((xReal * xReal + yReal * yReal) /
//        (2 * xReal * cos(x) + 2 * yReal * sin(x)));
    const float r = (xReal * xReal + yReal * yReal) /
                    (2 * xReal * cos(x) + 2 * yReal * sin(x));
    if (r > 0)
      return log10(r);
    else
      return 0;
  }

  TRGPoint2D
  TRGCDCHoughTransformationCircle::circleCenter(const TRGPoint2D& p) const
  {
//    return TRGPoint2D(pow(10, p.y()), p.x());
    return TRGPoint2D(p.x(), pow(10, p.y()));
  }

  TRGPoint2D
  TRGCDCHoughTransformationCircle::convert(const TRGPoint2D& p) const
  {
    return TRGPoint2D(p.x(), log10(p.y()));
  }

  bool
  TRGCDCHoughTransformationCircle::diverge(float xReal,
                                           float yReal,
                                           float x0,
                                           float x1) const
  {
    static const float PI2 = 2 * M_PI;

    //...Divergence points...
    float d0 = atan(- xReal / yReal);
    if (d0 < 0) d0 += PI2;
    float d1 = d0 + M_PI;
    if (d1 > PI2) d1 -= PI2;

    //...In region ?...
    if ((x0 < d0) && (d0 < x1)) return true;
    if ((x0 < d1) && (d1 < x1)) return true;

    return false;
  }

  bool
  TRGCDCHoughTransformationCircle::positiveDiverge(float xReal,
                                                   float yReal,
                                                   float x0,
                                                   float x1) const
  {
    return diverge(xReal, yReal, x0, x1);
  }

  bool
  TRGCDCHoughTransformationCircle::negativeDiverge(float xReal,
                                                   float yReal,
                                                   float x0,
                                                   float x1) const
  {
    return diverge(xReal, yReal, x0, x1);
  }

} // namespace Belle2
