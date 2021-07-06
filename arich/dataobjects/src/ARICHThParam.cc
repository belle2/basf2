/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

