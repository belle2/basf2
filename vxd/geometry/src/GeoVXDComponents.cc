/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald,           *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/geometry/GeoVXDComponents.h>
#include <framework/logging/Logger.h>
#include <locale>

namespace Belle2 {
  VXDGeoPlacement::VXDGeoPlacement(const std::string& name, double u, double v, std::string w, double woffset): m_name(name), m_u(u),
    m_v(v), m_woffset(woffset)
  {
    w = std::tolower(w, std::locale());
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
}  // namespace Belle2
