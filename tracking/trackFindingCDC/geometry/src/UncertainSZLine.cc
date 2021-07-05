/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/UncertainSZLine.h>

#include <tracking/trackFindingCDC/geometry/SZLine.h>

#include <ostream>

using namespace Belle2;
using namespace TrackFindingCDC;

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const UncertainSZLine& uncertainSZLine)
{
  return output << "UncertainSZLine("
         << "tanL=" << uncertainSZLine->slope() << ","
         << "z0=" << uncertainSZLine->intercept() << ")";
}
