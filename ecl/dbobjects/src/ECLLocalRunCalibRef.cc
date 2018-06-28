/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * ECLLocalRunCalibRef                                                    *
 *                                                                        *
 * This class is designed to store reference marks to database for        *
 * ECL local run calibration.                                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// ECL
#include <ecl/dbobjects/ECLLocalRunCalibRef.h>
using namespace Belle2;
// Constructor.
ECLLocalRunCalibRef::ECLLocalRunCalibRef():
  c_exp(0), c_run(0)
{
}
// Constructor.
ECLLocalRunCalibRef::ECLLocalRunCalibRef(const int& curExp,
                                         const int& curRun):
  c_exp(curExp), c_run(curRun)
{
}
// Destructor.
ECLLocalRunCalibRef::~ECLLocalRunCalibRef()
{
}
// Get experiment number.
int ECLLocalRunCalibRef::getExp() const
{
  return c_exp;
}
// Get run number.
int ECLLocalRunCalibRef::getRun() const
{
  return c_run;
}
