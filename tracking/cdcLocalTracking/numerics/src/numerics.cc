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
