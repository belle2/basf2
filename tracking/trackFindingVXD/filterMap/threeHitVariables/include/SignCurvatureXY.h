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

#define SIGNCURVATUREXY_NAME SignCurvatureXY

namespace Belle2 {

  /** calculates the sign of the curvature for three hits
   *
   * */
  template <typename PointType >
  class SIGNCURVATUREXY_NAME : public SelectionVariable< PointType , 3, int > {
  public:

    /** is replaced by "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(SIGNCURVATUREXY_NAME);

    /** calculates calculates the sign of the curvature of given 3-hit-tracklet.
    * a positive value represents a left-oriented curvature, a negative value means having a right-oriented curvature.
    * 0 means that it is exactly straight or that two hits are identical.
    * first vector should be outer hit, second = center hit, third is inner hit. */
    static int value(const PointType& a, const PointType& b, const PointType& c)
    {
      using boost::math::sign;
      B2Vector3<double> ba(a.X() - b.X(), a.Y() - b.Y(), 0.0);
      B2Vector3<double> bc(b.X() - c.X(), b.Y() - c.Y(), 0.0);
      return sign(bc.Orthogonal() * ba); //normal vector of m_vecBC times segment of ba
    }
  };

}
