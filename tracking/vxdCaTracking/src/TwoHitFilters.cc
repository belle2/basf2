/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/TwoHitFilters.h"
#include <boost/math/special_functions/fpclassify.hpp>


using namespace std;
using namespace Belle2;
using boost::math::isnan;


double TwoHitFilters::calcNormedDist3D()
{
  double normedVal = (m_x2 + m_y2) / (m_x2 + m_y2 + m_z2);
  return filterNan(normedVal);
//   if ((boost::math::isnan)(normedVal) == true) {
//     return 0;
//   } else {
//     return normedVal;
//   }
} // return unit: none


double TwoHitFilters::calcSlopeRZ()
{
  double slope = (m_x2 + m_y2) / m_dz;
  return filterNan(slope);
//   if ((boost::math::isnan)(slope) == true) {
//     return 0;
//   } else {
//     return slope;
//   }
} // return unit: cm  (cm^2/cm = cm)


double TwoHitFilters::filterNan(double value)
{
  if ((boost::math::isnan)(value) == true) return 0;
  return value;

}
