/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vxd/dbobjects/VXDGeoPlacementPar.h>




using namespace Belle2;
using namespace std;


VXDGeoPlacementPar::VXDGeoPlacementPar(const std::string& name, double u, double v, const std::string& w,
                                       double woffset): m_name(name),
  m_u(u),
  m_v(v), m_w(w), m_woffset(woffset)
{ }



