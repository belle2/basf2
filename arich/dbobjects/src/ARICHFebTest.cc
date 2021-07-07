/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHFebTest.h>

using namespace Belle2;
using namespace std;

int ARICHFebTest::getDeadChannel(unsigned int i) const
{
  if (i < m_deadChannel.size()) return m_deadChannel[i];
  else return -1;
}

float ARICHFebTest::getSlopeRough(unsigned int i) const
{
  if (i < m_slopesRough.size()) return m_slopesRough[i];
  else return -1;
}

float ARICHFebTest::getSlopeFine(unsigned int i) const
{
  if (i < m_slopesFine.size()) return m_slopesFine[i];
  else return -1;
}

