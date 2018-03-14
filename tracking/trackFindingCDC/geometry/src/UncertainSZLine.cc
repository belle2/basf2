/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
