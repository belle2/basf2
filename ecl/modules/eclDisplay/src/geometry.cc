/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module
#include <ecl/modules/eclDisplay/geometry.h>

using namespace Belle2::ECLDisplayUtility;

int do_energy_count = 1;

void Belle2::ECLDisplayUtility::SetMode(int i)
{
  do_energy_count = i;
}
int Belle2::ECLDisplayUtility::GetMode()
{
  return do_energy_count;
}
