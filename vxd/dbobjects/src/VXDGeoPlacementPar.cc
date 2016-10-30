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
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>

using namespace Belle2;
using namespace std;


VXDGeoPlacementPar::VXDGeoPlacementPar(const std::string& name, double u, double v, std::string w, double woffset): m_name(name),
  m_u(u),
  m_v(v), m_woffset(woffset)
{
  boost::to_lower(w);
  if (w == "below")        m_w = c_below;
  else if (w == "bottom")  m_w = c_bottom;
  else if (w == "center")  m_w = c_center;
  else if (w == "top")     m_w = c_top;
  else if (w == "above")   m_w = c_above;
  else  B2FATAL("Unknown z-placement for VXD Component " << name << ": " << w << ", check xml file");

  if (m_w != c_center && m_woffset < 0) {
    B2FATAL("VXD Component " << name << ": Offset has to be positive except for centered placement");
  }
}


void VXDGeoPlacementPar::setW(std::string  w)
{
  boost::to_lower(w);
  if (w == "below")        m_w = c_below;
  else if (w == "bottom")  m_w = c_bottom;
  else if (w == "center")  m_w = c_center;
  else if (w == "top")     m_w = c_top;
  else if (w == "above")   m_w = c_above;
  else  B2FATAL("Unknown z-placement for VXD Component " << m_name << ": " << w << ", check xml file");

  if (m_w != c_center && m_woffset < 0) {
    B2FATAL("VXD Component " << m_name << ": Offset has to be positive except for centered placement");
  }
}
