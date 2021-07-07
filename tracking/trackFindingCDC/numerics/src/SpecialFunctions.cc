/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/numerics/SpecialFunctions.h>

#include <boost/math/tools/precision.hpp>
#include <boost/math/special_functions/sinc.hpp>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

double TrackFindingCDC::sinc(double x)
{
  return boost::math::sinc_pi(x);
}

double TrackFindingCDC::asinc(double x)
{
  // Implementation inspired by BOOST's sinc
  BOOST_MATH_STD_USING;

  using namespace boost::math;
  double const taylor_n_bound = tools::forth_root_epsilon<double>();

  if (abs(x) >= taylor_n_bound) {
    return asin(x) / x;
  } else {
    // approximation by taylor series in x at 0 up to order 0
    double result = 1.0;

    double const taylor_0_bound = tools::epsilon<double>();
    if (abs(x) >= taylor_0_bound) {
      double x2 = x * x;
      // approximation by taylor series in x at 0 up to order 2
      result += x2 / 6.0;

      double const taylor_2_bound = tools::root_epsilon<double>();
      if (abs(x) >= taylor_2_bound) {
        // approximation by taylor series in x at 0 up to order 4
        result += x2 * x2 * (3.0 / 40.0);
      }
    }
    return result;
  }
}
