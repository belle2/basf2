/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/ThreeHitFilters.h"
// #include <boost/math/special_functions/fpclassify.hpp>
// #include <TMathBase.h>
// #include <math.h>
#include <boost/math/special_functions/sign.hpp>

using namespace std;
using namespace Belle2;
using boost::math::sign;

int ThreeHitFilters::calcSign(const TVector3& a, const TVector3& b, const TVector3& c)
{
  TVector3 ba = a - b; ba.SetZ(0.);
  TVector3 bc = b - c; bc.SetZ(0.);
  return sign(bc.Orthogonal() * ba); //normal vector of m_vecBC times segment of ba
}
