/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
