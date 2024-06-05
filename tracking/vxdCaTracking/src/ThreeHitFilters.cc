/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/vxdCaTracking/ThreeHitFilters.h"
// #include <boost/math/special_functions/fpclassify.hpp>
// #include <TMathBase.h>
// #include <math.h>
#include <boost/math/special_functions/sign.hpp>

using namespace Belle2;
using boost::math::sign;

int ThreeHitFilters::calcSign(const B2Vector3D& a, const B2Vector3D& b, const B2Vector3D& c)
{
  B2Vector3D ba = a - b; ba.SetZ(0.);
  B2Vector3D bc = b - c; bc.SetZ(0.);
  return sign(bc.Orthogonal() * ba); //normal vector of m_vecBC times segment of ba
}
