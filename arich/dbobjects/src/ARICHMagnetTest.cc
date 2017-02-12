/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHMagnetTest.h>

using namespace Belle2;
using namespace std;

float ARICHMagnetTest::getDeadTime(unsigned int i) const
{
  if (i < m_deadTime.size()) return m_deadTime[i];
  else return -1.;
}
