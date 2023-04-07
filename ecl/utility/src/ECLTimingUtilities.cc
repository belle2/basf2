/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/utility/ECLTimingUtilities.h>

#include <framework/database/DBObjPtr.h>
#include <ecl/dbobjects/ECLTimeWalkCorrection.h>

#include <math.h>
#include <vector>

using namespace Belle2;
using namespace ECL;

ECLTimingUtilities::ECLTimingUtilities()
{ }

// Time offset as a function of the signal amplitude measured in units of ticks
double ECLTimingUtilities::energyDependentTimeOffsetElectronic(const double amp) const
{
  std::vector<double> params;
  if (m_loadFromDB) {
    DBObjPtr<ECLTimeWalkCorrection> correction_data;
    // Get energy dependence time offset fit parameters
    params = correction_data->getParams();
  } else {
    params = {
      energyDependenceTimeOffsetFitParam_p0,
      energyDependenceTimeOffsetFitParam_p1,
      energyDependenceTimeOffsetFitParam_p2,
      energyDependenceTimeOffsetFitParam_p3,
      energyDependenceTimeOffsetFitParam_p4,
      energyDependenceTimeOffsetFitParam_p5
    };
  }
  return params[0] + pow((params[2] / (amp + params[1])), params[3]) +
         params[4] * exp(-amp / params[5]);
}

