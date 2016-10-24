/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                                             *
 * Author: The Belle II Collaboration                                         *
 * Contributors: CDC group                                                     *
 *                                                                                             *
 * This software is provided "as is" without any warranty.        *
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
