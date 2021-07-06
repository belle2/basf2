/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHMagnetTest.h>

using namespace Belle2;
using namespace std;

float ARICHMagnetTest::getDeadTime(unsigned int i) const
{
  if (i < m_deadTime.size()) return m_deadTime[i];
  else return -1.;
}
