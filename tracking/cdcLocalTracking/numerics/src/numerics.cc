/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <cmath>
#include "../include/numerics.h"

#include <boost/math/tools/precision.hpp>


using namespace boost::math;
using namespace Belle2;
using namespace CDCLocalTracking;


bool Belle2::CDCLocalTracking::isNAN(const double x)
{
  return std::isnan(x);
}



bool Belle2::CDCLocalTracking::isNAN(const float x)
{
  return std::isnan(x);
}



SignType Belle2::CDCLocalTracking::sign(double x)
{
  return std::isnan(x) ? INVALID_SIGN : (std::signbit(x) ? MINUS : PLUS);
}



FloatType Belle2::CDCLocalTracking::cosc(double x)
{
  // Implementation of  (1 - cos(x)) / x
  // Inspired by BOOST's sinc
  BOOST_MATH_STD_USING;

  FloatType const taylor_n_bound = tools::forth_root_epsilon<FloatType>();

  if (abs(x) >= taylor_n_bound) {
    return (1 - cos(x)) / x;

  } else {
    // approximation by taylor series in x at 0 up to order 0
    FloatType result = 0.0;

    FloatType const taylor_0_bound = tools::epsilon<FloatType>();
    if (abs(x) >= taylor_0_bound) {
      // approximation by taylor series in x at 0 up to order 1
      result += x / 2.0;

      FloatType const taylor_2_bound = tools::root_epsilon<FloatType>();
      if (abs(x) >= taylor_2_bound) {
        // approximation by taylor series in x at 0 up to order 3
        result -= x * x * x / 24.0;

      }
    }
    return result;
  }




}
