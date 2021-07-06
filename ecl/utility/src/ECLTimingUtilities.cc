/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/utility/ECLTimingUtilities.h>
#include <math.h>

using namespace Belle2;
using namespace ECL;

ECLTimingUtilities::ECLTimingUtilities()
{ }

// Time offset as a function of the signal amplitude measured in units of ticks
double ECLTimingUtilities::energyDependentTimeOffsetElectronic(const double amp) const
{
  return  energyDependenceTimeOffsetFitParam_p1 + pow((energyDependenceTimeOffsetFitParam_p3 /
                                                       (amp + energyDependenceTimeOffsetFitParam_p2)),
                                                      energyDependenceTimeOffsetFitParam_p4) + energyDependenceTimeOffsetFitParam_p5 * exp(-amp /
                                                          energyDependenceTimeOffsetFitParam_p6) ;
}
