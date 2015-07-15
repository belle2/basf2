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
#include <tracking/trackFindingCDC/numerics/numerics.h>

#include <boost/math/tools/precision.hpp>


using namespace boost::math;
using namespace Belle2;
using namespace TrackFindingCDC;


bool Belle2::TrackFindingCDC::isNAN(const double x)
{
  return std::isnan(x);
}



bool Belle2::TrackFindingCDC::isNAN(const float x)
{
  return std::isnan(x);
}



SignType Belle2::TrackFindingCDC::sign(double x)
{
  return std::isnan(x) ? INVALID_SIGN : (std::signbit(x) ? MINUS : PLUS);
}

std::vector<double>
Belle2::TrackFindingCDC::linspace(const double& start, const double& end, const int n)
{
  std::vector<double> result(n);
  result[0] = start;
  result[n - 1] = end;

  for (int i = 1; i < n - 1; ++i) {
    double start_weight = static_cast<double>(n - 1 - i) / (n - 1);
    double end_weight = static_cast<double>(i) / (n - 1);
    result[i] = start * start_weight + end * end_weight;
  }

  return result;
}
