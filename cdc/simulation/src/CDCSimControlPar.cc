/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/simulation/CDCSimControlPar.h>

using namespace Belle2;
using namespace CDC;

CDCSimControlPar* CDCSimControlPar::m_pntr = 0;

CDCSimControlPar& CDCSimControlPar::getInstance()
{
  if (!m_pntr) m_pntr = new CDCSimControlPar();
  return *m_pntr;
}

CDCSimControlPar::CDCSimControlPar()
{
  //  m_wireSag = false;
}

CDCSimControlPar::~CDCSimControlPar()
{
}
