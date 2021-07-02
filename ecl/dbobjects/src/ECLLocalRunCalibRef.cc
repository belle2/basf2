/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
