/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ewan Hill (ehill@mail.ubc.ca)                            *
 *               Alexander Kuzmin (A.S.Kuzmin@inp.nsk.su)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
