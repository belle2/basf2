/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
