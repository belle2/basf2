/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/geometry/UncertainSZLine.h>

#include <tracking/trackingUtilities/geometry/SZLine.h>

#include <ostream>

using namespace Belle2;
using namespace TrackingUtilities;

std::ostream& TrackingUtilities::operator<<(std::ostream& output, const UncertainSZLine& uncertainSZLine)
{
  return output << "UncertainSZLine("
         << "tanL=" << uncertainSZLine->slope() << ","
         << "z0=" << uncertainSZLine->intercept() << ")";
}
