/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDGeoPlacementPar.h>




using namespace Belle2;
using namespace std;


VXDGeoPlacementPar::VXDGeoPlacementPar(const std::string& name, double u, double v, std::string w, double woffset): m_name(name),
  m_u(u),
  m_v(v), m_w(w), m_woffset(woffset)
{ }



