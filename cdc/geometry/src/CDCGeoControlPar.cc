/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                                             *
 * Author: The Belle II Collaboration                                         *
 * Contributors: CDC group                                                     *
 *                                                                                             *
 * This software is provided "as is" without any warranty.        *
 **************************************************************************/

#include <cdc/geometry/CDCGeoControlPar.h>

using namespace Belle2;
using namespace CDC;

CDCGeoControlPar* CDCGeoControlPar::m_pntr = 0;

CDCGeoControlPar& CDCGeoControlPar::getInstance()
{
  if (!m_pntr) m_pntr = new CDCGeoControlPar();
  return *m_pntr;
}

CDCGeoControlPar::CDCGeoControlPar()
{
}

CDCGeoControlPar::~CDCGeoControlPar()
{
}
