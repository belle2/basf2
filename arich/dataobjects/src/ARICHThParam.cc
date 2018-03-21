/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dataobjects/ARICHThParam.h>
#include <arich/dataobjects/sa02board.h>

using namespace std;
using namespace Belle2;

double ARICHThParam::getVth() const
{
  return m_th0 + m_index * m_dth;
}

int ARICHThParam::getVal() const
{
  double Vth = getVth();
  return int((Vth - VTH_MIN) / VTH_STEP);
}

