/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/
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
