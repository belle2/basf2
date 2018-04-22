/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
#include <boost/math/special_functions/sign.hpp>

#include <framework/geometry/B2Vector3.h>

#include <math.h>

#define SIGNCURVATUREXYERROR_NAME SignCurvatureXYError

namespace Belle2 {

  /** calculates the sign of the curvature for three hits
   *
   * */
  template <typename PointType >
  class SIGNCURVATUREXYERROR_NAME : public SelectionVariable< PointType , 3, int > {
  public:

    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(SIGNCURVATUREXYERROR_NAME);

    /** calculates calculates the sign of the curvature of given 3-hit-tracklet.
     * +1 represents a left-oriented curvature, -1 means having a right-oriented curvature.
     * 0 means it is approximately straight.
     * first vector should be outer hit, second = center hit, third is inner hit. */
    static int value(const PointType& a, const PointType& b, const PointType& c)
    {
      B2Vector3<double> sigma_a = a.getPositionError();
      B2Vector3<double> sigma_b = b.getPositionError();
      B2Vector3<double> sigma_c = c.getPositionError();

      B2Vector3<double> c2b(b.X() - c.X(), b.Y() - c.Y(), 0.0);
      B2Vector3<double> b2a(a.X() - b.X(), a.Y() - b.Y(), 0.0);
      //TODO: check if acos of the dot product is better (faster)
      double angle = atan2(b2a[0], b2a[1]) - atan2(c2b[0], c2b[1]);
      //TODO 1/3...mean of the sigmas. Possible improvement: Use a parameter instead, and determine with simulated events.
      double sigmaan = (sigma_a.Mag() + sigma_b.Mag() + sigma_c.Mag()) / (3.*(c2b.Mag() + b2a.Mag()));
      if (angle < (-sigmaan)) { return -1; }
      else if (angle > sigmaan)  {return 1; }
      else  { return 0; }
    }

  };

}
