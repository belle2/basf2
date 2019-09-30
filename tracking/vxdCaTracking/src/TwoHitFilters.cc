/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/TwoHitFilters.h"

using namespace std;
using namespace Belle2;


double TwoHitFilters::filterNan(double value) const
{
  if (std::isnan(value) == true) { return 0; }
  if (std::isinf(value) == true) { return 0; }
  return value;
}
