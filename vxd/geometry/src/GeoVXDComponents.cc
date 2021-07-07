/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vxd/geometry/GeoVXDComponents.h>
#include <framework/logging/Logger.h>
#include <boost/algorithm/string.hpp>

namespace Belle2 {
  VXDGeoPlacement::VXDGeoPlacement(const std::string& name, double u, double v, std::string w, double woffset): m_name(name), m_u(u),
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
}  // namespace Belle2
