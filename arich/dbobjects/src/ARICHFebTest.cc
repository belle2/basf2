/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHFebTest.h>

using namespace Belle2;
using namespace std;

ClassImp(ARICHFebTest);

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

